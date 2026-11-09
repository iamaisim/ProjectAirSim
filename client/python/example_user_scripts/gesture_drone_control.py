"""
Gesture-Based Drone Control System using Project AirSim

Dependencies:
pip install mediapipe
pip install opencv-python
pip install projectairsim

Gesture Mapping:
- 1 Finger Up         -> Ascend / Takeoff
- 1 Finger Down       -> Descend
- 1 Finger Left       -> Move Left
- 2 Fingers Up        -> Move Forward
- 2 Fingers Down      -> Move Backward
- Fist (Closed Hand)  -> Hover (Stop motion)
- Thumb Right         -> Move Right
- Thumb Down          -> Move Down
- 3 Fingers Up        -> Instant Kill (Disarm + Disconnect)

"""


import asyncio
import cv2
import mediapipe as mp
import math
from collections import deque, Counter

from projectairsim import ProjectAirSimClient, Drone, World

# =========================
# SETUP (UNCHANGED LOGIC)
# =========================

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

hands = mp_hands.Hands(
    max_num_hands=1,
    model_complexity=1,
    min_detection_confidence=0.6,
    min_tracking_confidence=0.6
)

gesture_history = deque(maxlen=5)

# =========================
# HELPERS (UNCHANGED)
# =========================

def distance(a, b):
    return math.hypot(a.x - b.x, a.y - b.y)

def is_extended(tip, base):
    return distance(tip, base) > 0.11

def get_direction(tip, base):
    dx = tip.x - base.x
    dy = tip.y - base.y

    if abs(dx) > abs(dy):
        return "RIGHT" if dx > 0 else "LEFT"
    else:
        return "UP" if dy < 0 else "DOWN"

def stable(gesture):
    if gesture:
        gesture_history.append(gesture)
    if not gesture_history:
        return ""
    return Counter(gesture_history).most_common(1)[0][0]

# =========================
# GESTURE LOGIC (UNCHANGED)
# =========================

def detect_gesture(frame):
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    res = hands.process(rgb)

    gesture = ""

    if res.multi_hand_landmarks:
        for lm in res.multi_hand_landmarks:
            mp_draw.draw_landmarks(frame, lm, mp_hands.HAND_CONNECTIONS)

            l = lm.landmark
            wrist = l[0]

            thumb_tip = l[4]

            index_tip = l[8]
            middle_tip = l[12]
            ring_tip = l[16]
            pinky_tip = l[20]

            index_base = l[5]
            middle_base = l[9]
            ring_base = l[13]
            pinky_base = l[17]

            index_up = is_extended(index_tip, index_base)
            middle_up = is_extended(middle_tip, middle_base)
            ring_up = is_extended(ring_tip, ring_base)
            pinky_up = is_extended(pinky_tip, pinky_base)

            index_dir = get_direction(index_tip, index_base)
            middle_dir = get_direction(middle_tip, middle_base)
            ring_dir = get_direction(ring_tip, ring_base)

            fingers_folded = not (index_up or middle_up or ring_up or pinky_up)

            thumb_dx = thumb_tip.x - wrist.x
            thumb_dy = thumb_tip.y - wrist.y

            thumb_right = thumb_dx > 0.15 and abs(thumb_dx) > abs(thumb_dy)
            thumb_down = thumb_dy > 0.15 and abs(thumb_dy) > abs(thumb_dx)

            if fingers_folded and thumb_right:
                gesture = "RIGHT"

            elif fingers_folded and thumb_down:
                gesture = "DOWN"

            elif fingers_folded:
                gesture = "HOLD"

            elif index_up and not middle_up and not ring_up and not pinky_up:
                if index_dir == "UP":
                    gesture = "UP"
                elif index_dir == "LEFT":
                    gesture = "LEFT"

            elif index_up and middle_up and not ring_up and not pinky_up:
                if index_dir == "UP" and middle_dir == "UP":
                    gesture = "FRONT"
                elif index_dir == "DOWN" and middle_dir == "DOWN":
                    gesture = "BACK"

            elif index_up and middle_up and ring_up and not pinky_up:
                if index_dir == "UP":
                    gesture = "LAND"

    return stable(gesture)

# =========================
# MOVEMENT SETTINGS
# =========================

MOVE_SPEED = 6.5
Z_SPEED = 6.0
COMMAND_DURATION = 0.05

# =========================
# CONTROL FIXES (IMPORTANT PART)
# =========================

async def move_xy(drone, vx, vy):
    # 🔥 FIX: NO ALTITUDE CHANGE EVER
    await drone.move_by_velocity_async(vx, vy, 0, COMMAND_DURATION)

async def move_vertical(drone, vz):
    await drone.move_by_velocity_async(0, 0, vz, COMMAND_DURATION)

# =========================
# 🔥 INSTANT KILL LAND (NEW REQUIREMENT)
# =========================

async def instant_shutdown(drone, client):
    # 1. STOP ALL MOTION IMMEDIATELY
    await drone.move_by_velocity_async(0, 0, 0, 0.01)

    # 2. DISARM IMMEDIATELY (cuts motors logically)
    drone.disarm()

    # 3. REMOVE CONTROL (prevents further commands)
    drone.disable_api_control()

    # 4. HARD DISCONNECT
    client.disconnect()

# =========================
# VELOCITY MAP
# =========================

def gesture_to_velocity(g):
    vx, vy, vz = 0, 0, 0

    if g == "UP":
        vz = -Z_SPEED
    elif g == "DOWN":
        vz = Z_SPEED
    elif g == "FRONT":
        vx = MOVE_SPEED
    elif g == "BACK":
        vx = -MOVE_SPEED
    elif g == "RIGHT":
        vy = MOVE_SPEED
    elif g == "LEFT":
        vy = -MOVE_SPEED

    return vx, vy, vz

# =========================
# MAIN
# =========================

async def main():

    client = ProjectAirSimClient()
    client.connect()

    world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=1)
    drone = Drone(client, world, "Drone1")

    drone.enable_api_control()
    drone.arm()

    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 360)


    flying = False

    try:
        while True:

            ret, frame = cap.read()
            if not ret:
                break

            frame = cv2.flip(frame, 1)
            gesture = detect_gesture(frame)

            # =========================
            # TAKEOFF
            # =========================
            if gesture == "UP":
                if not flying:
                    await drone.takeoff_async()
                    flying = True
                await move_vertical(drone, -Z_SPEED)

            # =========================
            # 🔥 LAND = INSTANT KILL
            # =========================
            elif gesture == "LAND":
                await drone.move_by_velocity_async(0, 0, 0, 0.05)

                drone.disarm()
                drone.disable_api_control()
                await asyncio.sleep(0.2)
                client.disconnect()

                break
            # =========================
            # HOLD
            # =========================
            elif gesture == "HOLD":
                await drone.move_by_velocity_async(0, 0, 0, COMMAND_DURATION)

            # =========================
            # HORIZONTAL ONLY (NO ALTITUDE DRIFT)
            # =========================
            elif gesture in ["LEFT", "RIGHT", "FRONT", "BACK"]:
                vx, vy, _ = gesture_to_velocity(gesture)
                await move_xy(drone, vx, vy)

            # =========================
            # VERTICAL ONLY
            # =========================
            elif gesture in ["UP", "DOWN"]:
                _, _, vz = gesture_to_velocity(gesture)
                await move_vertical(drone, vz)

            cv2.putText(frame, f"GESTURE: {gesture}", (30, 60),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)

            cv2.imshow("Drone Control", frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

            await asyncio.sleep(0.005)

    finally:
        cap.release()
        cv2.destroyAllWindows()

        try:
            drone.disable_api_control()
        except:
            pass

        try:
            drone.disarm()
        except:
            pass

        try:
            client.disconnect()
        except:
            pass

        await asyncio.sleep(0.2)

        print("Shutdown complete")

if __name__ == "__main__":
    asyncio.run(main())
