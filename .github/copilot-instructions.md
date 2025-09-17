# Project AirSim Development Instructions

**Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.**

Project AirSim is a simulation platform for drones, robots, and other autonomous systems built on Unreal Engine 5. It consists of C++ simulation libraries, an Unreal Engine plugin, and Python client libraries for API interactions.

## Working Effectively

### Bootstrap Development Environment

**NEVER CANCEL these setup commands - they require network access and may take extended time:**

1. **Update system packages** (takes ~12 seconds):
   ```bash
   sudo apt-get update
   ```

2. **Install prerequisites** (takes ~60 seconds, NEVER CANCEL):
   ```bash
   sudo apt-get -y install --no-install-recommends \
       build-essential rsync make cmake \
       clang-18 libc++-18-dev libc++abi-18-dev \
       ninja-build libvulkan1 vulkan-tools \
       libeigen3-dev
   ```

3. **Fix clang version compatibility** (required for Ubuntu 24.04):
   ```bash
   sudo ln -sf /usr/bin/clang-18 /usr/bin/clang-13
   sudo ln -sf /usr/bin/clang++-18 /usr/bin/clang++-13
   ```

### Build Simulation Libraries

**CRITICAL: Build commands require external network access and may take 45+ minutes. NEVER CANCEL. Set timeout to 90+ minutes.**

1. **Configure and build debug libraries** (may take 45+ minutes, NEVER CANCEL):
   ```bash
   ./build.sh simlibs_debug
   ```

2. **Configure and build release libraries** (may take 45+ minutes, NEVER CANCEL):
   ```bash
   ./build.sh simlibs_release
   ```

3. **Test simulation libraries** (takes 15+ minutes, NEVER CANCEL):
   ```bash
   ./build.sh test_simlibs_debug
   ./build.sh test_simlibs_release
   ```

4. **Full build including Unreal Engine integration** (requires UE_ROOT environment variable):
   ```bash
   # Only if Unreal Engine is available
   export UE_ROOT=/path/to/UnrealEngine
   ./build.sh all
   ```

### Python Client Development

**Network connectivity required for pip installations. May fail in restricted environments.**

1. **Install Python client in development mode** (may fail due to network restrictions):
   ```bash
   cd client/python/projectairsim
   python3 -m pip install -e .
   ```

2. **Test Python client compilation** (always works, no network required):
   ```bash
   cd client/python/projectairsim
   find . -name "*.py" -exec python3 -m py_compile {} \;
   ```

3. **Run Python tests** (requires running simulation server):
   ```bash
   cd client/python/projectairsim
   python3 -m pytest tests/
   ```

## Validation

**CRITICAL: Always validate changes through complete scenarios, not just compilation.**

### Manual Validation Requirements

1. **Always compile Python client** to verify syntax:
   ```bash
   cd client/python/projectairsim
   find . -name "*.py" -exec python3 -m py_compile {} \;
   ```

2. **Test basic import functionality** (if dependencies are available):
   ```bash
   cd client/python/projectairsim/src
   python3 -c "import projectairsim; print('Import successful')"
   ```

3. **Build verification after C++ changes**:
   ```bash
   ./build.sh clean
   ./build.sh simlibs_debug
   ```

### Known Limitations

- **Network Dependencies**: Build requires external access to download Eigen3, NNG, assimp, and other dependencies from GitLab/GitHub
- **Clang Version**: Hardcoded to clang-13 but can use clang-18 with symbolic links
- **Python Dependencies**: open3d version constraints may conflict with newer Python versions
- **Unreal Engine**: Full functionality requires UE5 installation and UE_ROOT environment variable

## Build Targets and Timing

All timing estimates include 50% buffer. **NEVER CANCEL these operations:**

- `simlibs_debug`: Build debug simulation libraries - **45+ minutes, NEVER CANCEL**
- `simlibs_release`: Build release simulation libraries - **45+ minutes, NEVER CANCEL**  
- `test_simlibs_debug`: Run debug tests - **15+ minutes, NEVER CANCEL**
- `test_simlibs_release`: Run release tests - **15+ minutes, NEVER CANCEL**
- `all`: Complete build including Unreal integration - **90+ minutes, NEVER CANCEL**
- `clean`: Clean all build artifacts - **2 minutes**

## Repository Structure

```
projectairsim/
├── client/python/projectairsim/    # Python client library
├── core_sim/                       # Core simulation components
├── physics/                        # Physics integrations
├── vehicle_apis/                   # Vehicle control APIs
├── unreal/Blocks/                  # Unreal Engine environment
├── unity/                          # Unity integration
├── mavlinkcom/                     # MAVLink communication
├── rendering/                      # Rendering components
├── ros/                            # ROS integration
├── samples/                        # Example projects
├── docs/                           # Documentation
├── build.sh                        # Main build script
├── build_linux.mk                  # Make-based build system
└── setup_linux_dev_tools.sh       # Prerequisites installer
```

## Common Tasks

### Documentation Build
```bash
cd docs
pip install sphinx
make html
```

### Python Client Tests
```bash
cd client/python/projectairsim
pytest tests/test_*.py
```

### Check Available Build Targets
```bash
./build.sh
# Shows all available targets with descriptions
```

## Development Workflow

1. **Always start with prerequisites installation**
2. **Build debug libraries first** for development
3. **Test Python client compilation** after any Python changes
4. **Run appropriate test suites** based on changed components
5. **Never cancel long-running builds** - they require substantial network downloads
6. **Use clean builds** when encountering unexplained build failures

## Network Dependency Workarounds

If external network access is limited:

1. **Use system packages** when available (like libeigen3-dev)
2. **Pre-download dependencies** in environments with network access
3. **Modify CMakeLists.txt** to use local dependency sources
4. **Document network requirements** clearly for deployment environments

## CI/CD Integration

The repository includes `.github/workflows/` for CI automation. Key considerations:

- **Windows builds**: Use `build.cmd` instead of `build.sh`
- **Long timeouts required**: Set CI timeouts to 90+ minutes for full builds
- **Network access required**: CI environments must have external connectivity
- **Multi-stage builds recommended**: Cache intermediate artifacts to reduce build times