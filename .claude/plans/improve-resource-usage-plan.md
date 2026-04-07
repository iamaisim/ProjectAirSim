# Fix Excessive CPU Usage — Step-API-Only Mode

## Context

When a Python client connects, CPU usage spikes to 103%+ even when idle. The root cause is a single Python thread spinning `recv(block=False)` in a zero-sleep loop. Server-side system CPU is only 3-6% — not a real problem.

## Plan

### PR1: Kill client-side recv spin
**File:** `client/python/projectairsim/src/projectairsim/client.py`

1. Remove auto-start of `recv_topic_thread` from `connect()` (line 81-82)
2. Start it lazily on first `subscribe()` call
3. Switch recv loop from `recv(block=False)` to blocking recv with 100ms timeout
4. Keep thread non-daemon — the 100ms recv timeout guarantees bounded join on `disconnect()`

**Note:** `get_topic_info()` calls `subscribe()` internally, so lazy start still works for topic discovery.

**Acceptance:** Idle Python CPU < 5%. Step throughput within 5% of baseline.

### PR2: Server topic manager — blocking recv
**File:** `core_sim/src/topic_manager.cpp`

1. Set `NNG_OPT_RECVTIMEO` (100ms) on topic socket at startup
2. Remove `NNG_FLAG_NONBLOCK` from `nng_recv()` (line 422)
3. Remove `sleep_for(milliseconds(1))` (line 427)
4. Handle `NNG_ETIMEDOUT` same as `NNG_EAGAIN` (continue)

**Acceptance:** Server stops waking 1000/sec while idle.

### After PR1+PR2: Re-profile
Run full test matrix. If system CPU and step throughput look fine, stop here.

## Files to Modify
- `client/python/projectairsim/src/projectairsim/client.py`
- `core_sim/src/topic_manager.cpp`

## Verification
```
uv run pytest tests/test_cpu_usage.py -v -s
```
**Pass criteria:**
1. Idle Python CPU < 5% (from 103%)
2. No thread pinned near 100% when idle
3. Step throughput regression < 5% (baseline: 342.5 steps/sec at 3ms dt)

---

## Future (only if post-fix profiling shows need)

### Scene step yield loops → condition variable
`core_sim/src/scene.cpp` lines 695, 722, 746, 769, 789 — `while (!IsSimPaused()) { yield(); }`. Replace with CV wait + stop predicate. Requires lock-order validation against `ScheduledExecutor::mutex_` (clock.cpp:305).

### ScheduledExecutor paused spin → 1ms sleep
`core_sim/src/clock.cpp:287-326` — loops at 333Hz when paused. Back off to 1ms sleep. Must verify < 5% throughput regression.

### UnrealScene sleep(0) → sleep(100µs)
`UnrealScene.cpp:441-443` — replace zero-duration sleep with bounded microsecond sleep.

---

## Measurements

### Baseline (2026-04-05, pre-fix)

#### Idle / Connected Cases

| Case | Scenario                   | Py CPU     | Sys CPU | Hottest Thread |
|------|----------------------------|------------|---------|----------------|
| 2    | Connected only             | **103.5%** | 5.9%    | 104.0%         |
| 3    | Scene loaded, idle         | **103.3%** | 3.7%    | 103.6%         |
| 9    | Idle after stepping        | **103.6%** | 3.4%    | 103.6%         |

#### Stepping Cases

| Case | Scenario                        | Py CPU     | Steps/sec | Avg Latency |
|------|---------------------------------|------------|-----------|-------------|
| 4    | Step 3ms, no images             | **105.9%** | 342.5     | 2.92ms      |
| 5    | Step 3ms + GetImages every step | **104.2%** | 3.2       | 314.57ms    |
| 6    | Step 3ms + GetImages every 10   | **104.3%** | 31.2      | 32.06ms     |
| 7    | Step 10ms, no images            | **104.2%** | 85.8      | 11.66ms     |
| 8    | Step 20ms, no images            | **104.0%** | 48.9      | 20.44ms     |

#### Key Findings

1. **One Python thread (recv_topic) burns an entire core** even when idle — the only real problem.
2. **System CPU is 3-6%** — server-side spins exist but aren't impactful.
3. **GetImages is 314ms/call** — separate issue, not a spin problem.
4. **Step latency scales linearly with dt** — RPC overhead is negligible.
