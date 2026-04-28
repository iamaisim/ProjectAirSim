# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
#
# Module Name:
#
#   CMakeLists.txt
#
# Abstract:
#
#   CMake build file.
#
# ---------------------------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15.0)

# Before making projects, set CMake policy to use MSVC_RUNTIME_LIBRARY property
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")  # Have to match UE (uses /MD non-debug CRT for all configs)

include(FetchContent)
include(ExternalProject)

set(PROJECTAIRSIM_IS_MACOS FALSE)
set(PROJECTAIRSIM_IS_LINUX FALSE)
if(APPLE)
    set(PROJECTAIRSIM_IS_MACOS TRUE)
elseif(UNIX)
    set(PROJECTAIRSIM_IS_LINUX TRUE)
endif()

set(PROJECTAIRSIM_ENABLE_LVMON TRUE)
if(PROJECTAIRSIM_IS_MACOS)
    set(PROJECTAIRSIM_ENABLE_LVMON FALSE)
endif()

if (PROJECTAIRSIM_IS_LINUX)
    if(DEFINED ENV{UE_ROOT})
        # Build/link against UE's packaged Linux toolchain instead of system-installed toolchain
        # Note: This must be set BEFORE `project()` is called
        message("Building/linking against Unreal packaged toolchain from UE_ROOT=$ENV{UE_ROOT}")
        set(CMAKE_TOOLCHAIN_FILE "unreal-linux-toolchain.cmake")
        link_directories("$ENV{UE_ROOT}/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/x86_64-unknown-linux-gnu")
    else()
        message("Building/linking against system-installed toolchain")
        set(CMAKE_C_COMPILER clang-15)
        set(CMAKE_CXX_COMPILER clang++-15)
    endif()
elseif(PROJECTAIRSIM_IS_MACOS)
    message("Building/linking against macOS system toolchain")
endif()

# Set up project
project("ProjectAirSimLibs")

# Capture build commit hash once during configure. Fallback to "unknown"
# when git is unavailable (or source is not a git checkout).
set(PROJECTAIRSIM_BUILD_COMMIT_HASH "unknown")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROJECTAIRSIM_BUILD_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE PROJECTAIRSIM_GIT_RESULT
    )

    if(NOT PROJECTAIRSIM_GIT_RESULT EQUAL 0 OR PROJECTAIRSIM_BUILD_COMMIT_HASH STREQUAL "")
        set(PROJECTAIRSIM_BUILD_COMMIT_HASH "unknown")
    endif()
endif()
message("PROJECTAIRSIM_BUILD_COMMIT_HASH set to ${PROJECTAIRSIM_BUILD_COMMIT_HASH}")

file(WRITE ${CMAKE_BINARY_DIR}/commit_hash.txt "${PROJECTAIRSIM_BUILD_COMMIT_HASH}\n")

# Set up paths for packaging the sim libs build outputs as custom post-command steps
set(SIMLIBS_PACKAGE_DIR ${CMAKE_SOURCE_DIR}/packages/projectairsim_simlibs)
set(SIMLIBS_SHARED_LIB_DIR ${SIMLIBS_PACKAGE_DIR}/shared_libs)
set(SIMLIBS_TEST_DIR ${CMAKE_BINARY_DIR}/unit_tests)
set(UE_PLUGIN_SIMLIBS_DIR ${CMAKE_SOURCE_DIR}/unreal/Blocks/Plugins/ProjectAirSim/SimLibs)
set(UNITY_WRAPPER_DLL_DIR ${CMAKE_SOURCE_DIR}/unity/BlocksUnity/Assets/Plugins)
set(MATLAB_PHYSICS_DIR ${CMAKE_SOURCE_DIR}/physics/matlab_sfunc)
set(JSBSIM_CORESIM_DIR ${CMAKE_SOURCE_DIR}/core_sim/jsbsim)
set(MATLAB_CONTROL_DIR ${CMAKE_SOURCE_DIR}/vehicle_apis/multirotor_api/matlab_sfunc)
# set(UE_PLUGIN_CESIUM_NATIVE_DIR ${CMAKE_SOURCE_DIR}/unreal/Blocks/Plugins/ProjectAirSim/SimLibs)
# set(UE_CESIUM_PLUGIN_DIR ${CMAKE_SOURCE_DIR}/unreal/Blocks/Plugins/CesiumForUnreal/)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    # Note: /EHsc is for MSVC to enable unwinding stack during C++ exception handling
    add_compile_options(/EHsc)
    # set(CESIUM_CXX_FLAGS /EHsc)

    # Disable manifest generation for executables to avoid build failures due to Windows write
    # permission issues (possibly from file locks by antivirus or other OS file protection)
    add_link_options(/MANIFEST:NO)
elseif(PROJECTAIRSIM_IS_LINUX)
    add_compile_definitions(__CLANG__)
    add_compile_options(-stdlib=libc++ -ferror-limit=10)
    # Note: Linking -lanl (for getaddrinfo) and -pthread are required because of nng
    add_link_options(-stdlib=libc++ -lanl -pthread)

    # set(CESIUM_CXX_FLAGS "-std=c++17 -stdlib=libc++")
    # set(CESIUM_LINKER_FLAGS "-stdlib=libc++ -lc++abi")
    # set(CESIUM_LINUX_TOOLCHAIN ${UE_CESIUM_PLUGIN_DIR}/extern/unreal-linux-toolchain.cmake)
elseif(PROJECTAIRSIM_IS_MACOS)
    add_compile_definitions(__CLANG__)
    add_compile_options(-stdlib=libc++ -ferror-limit=10)
    add_link_options(-stdlib=libc++)

    # set(CESIUM_CXX_FLAGS "-std=c++17 -stdlib=libc++")
    # set(CESIUM_LINKER_FLAGS "-stdlib=libc++ -lc++abi")
    # set(CESIUM_LINUX_TOOLCHAIN ${UE_CESIUM_PLUGIN_DIR}/extern/unreal-linux-toolchain.cmake)
endif()

# Set up dependency: jsbsim
message("Setting up [jsbsim] dependency as an external project...")
# CMake external projects don't adopt the parent's CMAKE_MSVC_RUNTIME_LIBRARY setting,
# so to force /MD non-debug CRT to match UE, build Debug config as Relwithdebinfo and
# build Release as Release.
set(JSBSIM_BUILD_TYPE $<IF:$<CONFIG:Debug>,Relwithdebinfo,Release>)
set(JSBSIM_SRC_DIR ${CMAKE_BINARY_DIR}/_deps/jsbsim/src/jsbsim-repo)
set(JSBSIM_LIB_DIR ${CMAKE_BINARY_DIR}/_deps/jsbsim-install/lib)
set(JSBSIM_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/jsbsim-install/include/JSBSim)
set(JSBSIM_LIBRARY_TYPE SHARED)
set(JSBSIM_BUILD_SHARED_LIBS ON)

# Platform-specific settings for JSBSim
if(WIN32)
    set(JSBSIM_C_COMPILER ${CMAKE_C_COMPILER})
    set(JSBSIM_CXX_COMPILER ${CMAKE_CXX_COMPILER})
    set(JSBSIM_CXX_FLAGS "")
    set(JSBSIM_BIN_DIR ${CMAKE_BINARY_DIR}/_deps/jsbsim-install/bin)
    set(JSBSIM_SHARED_LIB ${JSBSIM_BIN_DIR}/JSBSim.dll)
    set(JSBSIM_IMPORT_LIB ${JSBSIM_LIB_DIR}/JSBSim.lib)
    set(JSBSIM_BYPRODUCTS ${JSBSIM_SHARED_LIB} ${JSBSIM_IMPORT_LIB})
elseif(PROJECTAIRSIM_IS_MACOS)
    set(JSBSIM_C_COMPILER ${CMAKE_C_COMPILER})
    set(JSBSIM_CXX_COMPILER ${CMAKE_CXX_COMPILER})
    set(JSBSIM_CXX_FLAGS "-stdlib=libc++")
    set(JSBSIM_LIBRARY_TYPE STATIC)
    set(JSBSIM_BUILD_SHARED_LIBS OFF)
    set(JSBSIM_SHARED_LIB ${JSBSIM_LIB_DIR}/libJSBSim.a)
    set(JSBSIM_IMPORT_LIB "")
    set(JSBSIM_BYPRODUCTS ${JSBSIM_SHARED_LIB})
else()
    set(JSBSIM_C_COMPILER clang)
    set(JSBSIM_CXX_COMPILER clang++)
    set(JSBSIM_CXX_FLAGS "-stdlib=libc++")
    set(JSBSIM_SHARED_LIB ${JSBSIM_LIB_DIR}/libJSBSim.so)
    set(JSBSIM_IMPORT_LIB "")
    set(JSBSIM_BYPRODUCTS ${JSBSIM_SHARED_LIB})
endif()

ExternalProject_Add(jsbsim-repo
    GIT_REPOSITORY  https://github.com/JSBSim-Team/jsbsim
    GIT_TAG         "v1.1.12"
    GIT_CONFIG      "advice.detachedHead=false"
    PREFIX          ${CMAKE_BINARY_DIR}/_deps/jsbsim
    BUILD_COMMAND   ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${JSBSIM_BUILD_TYPE}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${JSBSIM_BUILD_TYPE} --target install
    UPDATE_COMMAND  ""  # disable update step
    CMAKE_ARGS
        -DCMAKE_C_COMPILER=${JSBSIM_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${JSBSIM_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${JSBSIM_CXX_FLAGS}
        -DCMAKE_POSITION_INDEPENDENT_CODE=True
        -DCMAKE_BUILD_TYPE:STRING=${JSBSIM_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/_deps/jsbsim-install
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DBUILD_SHARED_LIBS=${JSBSIM_BUILD_SHARED_LIBS}
        -DBUILD_PYTHON_MODULE=OFF
        -DBUILD_DOCS=OFF
        # Disable SONAME versioning so libJSBSim.so doesn't require libJSBSim.so.1 symlink at runtime
        -DCMAKE_PLATFORM_NO_VERSIONED_SONAME=TRUE
        # Fix RPATH issue with Ninja generator
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE
    TEST_COMMAND    ""  # disable test step
    BUILD_BYPRODUCTS ${JSBSIM_BYPRODUCTS}
    BUILD_ALWAYS 1
)

set(JSBSIM_POST_INSTALL_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E echo "Copying [jsbsim] library to ${JSBSIM_CORESIM_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${JSBSIM_CORESIM_DIR}/lib/$<IF:$<CONFIG:Release>,Release,Debug>/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JSBSIM_INCLUDE_DIR}" "${JSBSIM_CORESIM_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JSBSIM_SRC_DIR}/aircraft" "${JSBSIM_CORESIM_DIR}/models/aircraft"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JSBSIM_SRC_DIR}/engine" "${JSBSIM_CORESIM_DIR}/models/engine"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JSBSIM_SRC_DIR}/systems" "${JSBSIM_CORESIM_DIR}/models/systems"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${JSBSIM_SRC_DIR}/scripts" "${JSBSIM_CORESIM_DIR}/models/scripts"
    COMMAND ${CMAKE_COMMAND} -E copy "${JSBSIM_SHARED_LIB}" "${JSBSIM_CORESIM_DIR}/lib/$<IF:$<CONFIG:Release>,Release,Debug>/"
)
if(WIN32)
    list(APPEND JSBSIM_POST_INSTALL_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy "${JSBSIM_IMPORT_LIB}" "${JSBSIM_CORESIM_DIR}/lib/$<IF:$<CONFIG:Release>,Release,Debug>/")
endif()

ExternalProject_Add_Step(jsbsim-repo post-install
    ${JSBSIM_POST_INSTALL_COMMANDS}
    DEPENDEES install
)

# Add jsbsim as imported library
add_library(jsbsim ${JSBSIM_LIBRARY_TYPE} IMPORTED)
if(WIN32)
    set_target_properties(jsbsim PROPERTIES
        IMPORTED_LOCATION ${JSBSIM_SHARED_LIB}
        IMPORTED_IMPLIB ${JSBSIM_IMPORT_LIB}
    )
else()
    set_target_properties(jsbsim PROPERTIES
        IMPORTED_LOCATION ${JSBSIM_SHARED_LIB}
    )
endif()



# Set up dependency: nlohmann JSON
# Directly download single include file json.hpp
FetchContent_Declare(
    nlohmann-json
    URL                  https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp
    URL_HASH             SHA256=9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6
    DOWNLOAD_NO_EXTRACT  True
    DOWNLOAD_NO_PROGRESS True
    DOWNLOAD_DIR         ${CMAKE_BINARY_DIR}/_deps/nlohmann-install
)
FetchContent_GetProperties(nlohmann-json)
if(NOT nlohmann-json_POPULATED)
    message("Checking if [nlohmann-json] dependency needs to be fetched...")
    FetchContent_Populate(nlohmann-json)
    set(JSON_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/nlohmann-install)
    message("JSON_INCLUDE_DIR set to ${JSON_INCLUDE_DIR}")

    message("Packaging [JSON (nlohmann)] header to ${UE_PLUGIN_SIMLIBS_DIR}/json/include")
    file(COPY ${JSON_INCLUDE_DIR}/ DESTINATION ${UE_PLUGIN_SIMLIBS_DIR}/json/include)
endif()

# Set up dependency: NNG
message("Setting up [NNG] dependency as an external project...")
# CMake external projects don't adopt the parent's CMAKE_MSVC_RUNTIME_LIBRARY setting,
# so to force /MD non-debug CRT to match UE, build Debug config as Relwithdebinfo and
# build Release as Release.
set(NNG_BUILD_TYPE $<IF:$<CONFIG:Debug>,Relwithdebinfo,Release>)
if(WIN32)
    set(NNG_CMAKE_LINKER_MANIFEST_FLAG "/MANIFEST:NO")
else()
    set(NNG_CMAKE_LINKER_MANIFEST_FLAG "")
endif()
ExternalProject_Add(nng-external
    GIT_REPOSITORY          https://github.com/nanomsg/nng.git
    GIT_TAG                 v1.11
    GIT_CONFIG              "advice.detachedHead=false"
    PREFIX                  ${CMAKE_BINARY_DIR}/_deps/nng
    # Use Update step override to delete unused /etc/ folder of NNG development support files that list a lot of Go reference packages
    UPDATE_COMMAND          ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/_deps/nng/src/nng-external/etc"
    BUILD_COMMAND           ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${NNG_BUILD_TYPE}
    INSTALL_COMMAND         ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${NNG_BUILD_TYPE} --target install
    TEST_COMMAND            ""  # disable test step
    CMAKE_ARGS
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_EXE_LINKER_FLAGS=${NNG_CMAKE_LINKER_MANIFEST_FLAG}
        -DCMAKE_POSITION_INDEPENDENT_CODE=True
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/_deps/nng-install
        -DCMAKE_INSTALL_MESSAGE=LAZY  # don't print out all files copied
        -DCMAKE_BUILD_TYPE:STRING=${NNG_BUILD_TYPE}  # for single-config generators (Ninja)
        -DNNG_HAVE_GETRANDOM=False  # llvm-ar fails to find symbol `getrandom` (a syscall on Linux); Requires glibc >=2.25
        -DNNG_TESTS=False  # don't build NNG's unit tests
    EXCLUDE_FROM_ALL        True
)
set(NNG_LIB_DIR ${CMAKE_BINARY_DIR}/_deps/nng-install/lib)
set(NNG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/nng-install/include)
message("NNG_LIB_DIR set to ${NNG_LIB_DIR}")
message("NNG_INCLUDE_DIR set to ${NNG_INCLUDE_DIR}")
ExternalProject_Add_Step(nng-external post-install
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [nng] installed outputs to ${UE_PLUGIN_SIMLIBS_DIR}/nng/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_LIB_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/nng/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_INCLUDE_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/nng/include"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/nng/$<IF:$<CONFIG:Release>,Release,Debug>/cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/nng/$<IF:$<CONFIG:Release>,Release,Debug>/pkgconfig"

    # Also copy NNG install lib to Matlab physics and control S-functions folder for building in Matlab:
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_LIB_DIR}" "${MATLAB_PHYSICS_DIR}/_deps/nng-install/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_INCLUDE_DIR}" "${MATLAB_PHYSICS_DIR}/_deps/nng-install/include"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_LIB_DIR}" "${MATLAB_CONTROL_DIR}/_deps/nng-install/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${NNG_INCLUDE_DIR}" "${MATLAB_CONTROL_DIR}/_deps/nng-install/include"
    DEPENDEES install
)

# Assimp compiler-specific flags
set(ASSIMP_EXTRA_CXX_FLAGS "")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(ASSIMP_EXTRA_CXX_FLAGS "-Wno-error=nontrivial-memcall")
endif()

# Set up dependency: assimp
message("Setting up [assimp] dependency as an external project...")
# CMake external projects don't adopt the parent's CMAKE_MSVC_RUNTIME_LIBRARY setting,
# so to force /MD non-debug CRT to match UE, build Debug config as Relwithdebinfo and
# build Release as Release.
set(ASSIMP_BUILD_TYPE $<IF:$<CONFIG:Debug>,Relwithdebinfo,Release>)
if(WIN32)
    # Drop VC toolset version from library name so ProjectAirSim.Build.cs works regardless of the version of Visual Studio
    set(ASSIMP_LIBRARY_SUFFIX_OVERRIDE "-DLIBRARY_SUFFIX:STRING=-mt")
else()
    set(ASSIMP_LIBRARY_SUFFIX_OVERRIDE "")
endif()
ExternalProject_Add(assimp-external
    GIT_REPOSITORY          https://github.com/assimp/assimp.git
    GIT_TAG                 v5.2.5
    GIT_CONFIG              "advice.detachedHead=false"
    PREFIX                  ${CMAKE_BINARY_DIR}/_deps/assimp
    UPDATE_COMMAND          ""  # disable update step
    BUILD_COMMAND           ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${ASSIMP_BUILD_TYPE}
    INSTALL_COMMAND         ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${ASSIMP_BUILD_TYPE} --target install
    TEST_COMMAND            ""  # disable test step
    CMAKE_ARGS
        -DCMAKE_SYSROOT=${CMAKE_SYSROOT}
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}\ ${ASSIMP_EXTRA_CXX_FLAGS}
	    -DASSIMP_WARNINGS_AS_ERRORS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=True
        -DBUILD_SHARED_LIBS=OFF
        -DASSIMP_BUILD_ZLIB=ON  # force build of assimp's packaged zlib in case it detects it existing in a python environment
        -DASSIMP_BUILD_TESTS=OFF
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
        ${ASSIMP_LIBRARY_SUFFIX_OVERRIDE}
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/_deps/assimp-install
        -DCMAKE_INSTALL_MESSAGE=LAZY  # don't print out all files copied
        -DCMAKE_BUILD_TYPE:STRING=${ASSIMP_BUILD_TYPE}  # for single-config generators (Ninja)
)
set(ASSIMP_LIB_DIR ${CMAKE_BINARY_DIR}/_deps/assimp-install/lib)
set(ASSIMP_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/assimp-install/include)
message("ASSIMP_LIB_DIR set to ${ASSIMP_LIB_DIR}")
message("ASSIMP_INCLUDE_DIR set to ${ASSIMP_INCLUDE_DIR}")
ExternalProject_Add_Step(assimp-external post-install
    COMMAND ${CMAKE_COMMAND} -E echo "Packaging [assimp] installed outputs to ${UE_PLUGIN_SIMLIBS_DIR}/assimp/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSIMP_LIB_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/assimp/$<IF:$<CONFIG:Release>,Release,Debug>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSIMP_INCLUDE_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/assimp/include"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/assimp/$<IF:$<CONFIG:Release>,Release,Debug>/cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/assimp/$<IF:$<CONFIG:Release>,Release,Debug>/pkgconfig"
    DEPENDEES install
)

# Set up dependency: msgpack
# "When you use msgpack on C++, you can just add msgpack-c/include to your include path"
FetchContent_Declare(
    msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG        cpp-6.1.0
)
FetchContent_GetProperties(msgpack)
if(NOT msgpack_POPULATED)
    message("Checking if [msgpack] dependency needs to be fetched...")
    FetchContent_Populate(msgpack)
    add_definitions(-DMSGPACK_NO_BOOST)
    set(MSGPACK_INCLUDE_DIR ${msgpack_SOURCE_DIR}/include)
    message("MSGPACK_INCLUDE_DIR set to ${MSGPACK_INCLUDE_DIR}")
    # Note: msgpack is only used internally by sim libs, so no need to package it with other sim libs

    # Copy msgpack headers to Matlab physics and control S-function folders for building in Matlab:
    message("Packaging [msgpack] headers to ${MATLAB_PHYSICS_DIR}/msgpack")
    file(COPY ${msgpack_SOURCE_DIR}/include/ DESTINATION ${MATLAB_PHYSICS_DIR}/_deps/msgpack/include)

    message("Packaging [msgpack] headers to ${MATLAB_CONTROL_DIR}/msgpack")
    file(COPY ${msgpack_SOURCE_DIR}/include/ DESTINATION ${MATLAB_CONTROL_DIR}/_deps/msgpack/include)
endif()

# Set up dependency: Eigen3
# Using latest stable release as of from 08/18/21
FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        3.4.0
)
FetchContent_GetProperties(Eigen3)
string(TOLOWER "Eigen3" lc_eigen)
if(NOT ${lc_eigen}_POPULATED)
    message("Checking if [Eigen3] dependency needs to be fetched...")
    FetchContent_Populate(Eigen3)
    set(EIGEN_INCLUDE_DIR ${${lc_eigen}_SOURCE_DIR})
    message("EIGEN_INCLUDE_DIR set to ${EIGEN_INCLUDE_DIR}")

    message("Packaging [Eigen3] headers to ${UE_PLUGIN_SIMLIBS_DIR}/eigen")
    file(COPY ${EIGEN_INCLUDE_DIR}/ DESTINATION ${UE_PLUGIN_SIMLIBS_DIR}/eigen/include)
endif()

# Set up dependency: Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    message("Checking if [GTest] dependency needs to be fetched...")
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include(CTest) # this automatically also sets enable_testing()

# Set up dependency: tinygltf
# "When you use tinygltf on C++, you can just add to your include path since its header only library"
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG        v2.6.3
)
FetchContent_GetProperties(tinygltf)
if(NOT tinygltf_POPULATED)
    message("Checking if [tinygltf] dependency needs to be fetched...")
    FetchContent_Populate(tinygltf)
    set(TINYGLTF_INCLUDE_DIR ${tinygltf_SOURCE_DIR})
    file(REMOVE_RECURSE ${TINYGLTF_INCLUDE_DIR}/examples)
    message("TINYGLTF_INCLUDE_DIR set to ${TINYGLTF_INCLUDE_DIR}")
    # Note: tinygltf is only used internally by sim libs, so no need to package it with other sim libs
endif()

if(PROJECTAIRSIM_IS_LINUX)
    # Set up dependency: openssl
    message("Setting up [openssl] dependency as an external project...")
    # CMake external projects don't adopt the parent's CMAKE_MSVC_RUNTIME_LIBRARY setting,
    # so to force /MD non-debug CRT to match UE, build Debug config as Relwithdebinfo and
    # build Release as Release.
    set(OPENSSL_BUILD_TYPE $<IF:$<CONFIG:Debug>,debug,release>)
    ExternalProject_Add(openssl-external
        GIT_REPOSITORY          https://github.com/openssl/openssl.git
        GIT_TAG                 openssl-3.1.1
        GIT_CONFIG              "advice.detachedHead=false"
        PREFIX                  ${CMAKE_BINARY_DIR}/_deps/openssl
        UPDATE_COMMAND          ""  # disable update step
        CONFIGURE_COMMAND       <SOURCE_DIR>/Configure --${OPENSSL_BUILD_TYPE} --prefix=${CMAKE_BINARY_DIR}/_deps/openssl-install no-tests
        BUILD_COMMAND           make
        INSTALL_COMMAND         make install
        TEST_COMMAND            ""  # disable test step
    )
    set(OPENSSL_LIB_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-install/lib64)
    set(OPENSSL_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/openssl-install/include)
    message("OPENSSL_LIB_DIR set to ${OPENSSL_LIB_DIR}")
    message("OPENSSL_INCLUDE_DIR set to ${OPENSSL_INCLUDE_DIR}")
    ExternalProject_Add_Step(openssl-external post-install
        COMMAND ${CMAKE_COMMAND} -E echo "Packaging [openssl] installed outputs to $${UE_PLUGIN_SIMLIBS_DIR}/openssl/$<IF:$<CONFIG:Release>,Release,Debug>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${OPENSSL_LIB_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/openssl/$<IF:$<CONFIG:Release>,Release,Debug>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${OPENSSL_INCLUDE_DIR}" "${UE_PLUGIN_SIMLIBS_DIR}/openssl/include"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/openssl/$<IF:$<CONFIG:Release>,Release,Debug>/cmake"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${UE_PLUGIN_SIMLIBS_DIR}/openssl/$<IF:$<CONFIG:Release>,Release,Debug>/pkgconfig"
        DEPENDEES install
    )
elseif(PROJECTAIRSIM_IS_MACOS)
    find_package(OpenSSL REQUIRED)
    get_filename_component(OPENSSL_LIB_DIR "${OPENSSL_CRYPTO_LIBRARY}" DIRECTORY)
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR})
    message("Using macOS OpenSSL from ${OPENSSL_LIB_DIR}")
endif()

# 1. 针对不同平台设置自动下载的 URL
if(PROJECTAIRSIM_IS_LINUX)
    set(onnx_url https://github.com/microsoft/onnxruntime/releases/download/v1.12.1/onnxruntime-linux-x64-gpu-1.12.1.tgz)
    set(onnx_hash MD5=27adfa51648d2713608f35c3c6957a4f)
elseif(WIN32)
    set(onnx_url https://github.com/microsoft/onnxruntime/releases/download/v1.12.1/onnxruntime-win-x64-gpu-1.12.1.zip)
    set(onnx_hash MD5=dd6d6ba7aaaa58dd9ef8528dfcb822d2)
elseif(PROJECTAIRSIM_IS_MACOS)
    # 动态判断 Mac 是 Apple Silicon (arm64) 还是 Intel 芯片
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm64")
        set(onnx_url https://github.com/microsoft/onnxruntime/releases/download/v1.12.1/onnxruntime-osx-arm64-1.12.1.tgz)
    else()
        set(onnx_url https://github.com/microsoft/onnxruntime/releases/download/v1.12.1/onnxruntime-osx-x86_64-1.12.1.tgz)
    endif()
endif()

# 2. macOS 优先使用本地预编译包；没有提供时再自动下载
set(ONNXRUNTIME_ROOTDIR "" CACHE PATH "Path to a prebuilt ONNX Runtime package")
if(PROJECTAIRSIM_IS_MACOS AND NOT ONNXRUNTIME_ROOTDIR AND DEFINED ENV{ONNXRUNTIME_ROOT})
    set(ONNXRUNTIME_ROOTDIR "$ENV{ONNXRUNTIME_ROOT}" CACHE PATH "Path to a prebuilt ONNX Runtime package" FORCE)
endif()

if(NOT (PROJECTAIRSIM_IS_MACOS AND ONNXRUNTIME_ROOTDIR))
    if(PROJECTAIRSIM_IS_MACOS)
        FetchContent_Declare(
            onnxruntime
            URL                  ${onnx_url}
            DOWNLOAD_NO_PROGRESS True
            DOWNLOAD_DIR         ${CMAKE_BINARY_DIR}/_deps/onnxruntime-download
        )
    else()
        FetchContent_Declare(
            onnxruntime
            URL                  ${onnx_url}
            URL_HASH             ${onnx_hash}
            DOWNLOAD_NO_PROGRESS True
            DOWNLOAD_DIR         ${CMAKE_BINARY_DIR}/_deps/onnxruntime-download
        )
    endif()
endif()

# 3. 先确定 ONNX Runtime 的根目录，再统一进行变量设置和文件分发
if(PROJECTAIRSIM_IS_MACOS AND ONNXRUNTIME_ROOTDIR)
    if(NOT EXISTS "${ONNXRUNTIME_ROOTDIR}/include" OR NOT EXISTS "${ONNXRUNTIME_ROOTDIR}/lib")
        message(FATAL_ERROR "ONNXRUNTIME_ROOTDIR is set, but ${ONNXRUNTIME_ROOTDIR} does not contain include/ and lib/ directories")
    endif()
    set(ONNX_INCLUDE_DIR ${ONNXRUNTIME_ROOTDIR}/include)
    set(ONNX_LIB_DIR ${ONNXRUNTIME_ROOTDIR}/lib)
    message("Using macOS ONNX Runtime from ${ONNXRUNTIME_ROOTDIR}")
else()
    FetchContent_GetProperties(onnxruntime)
    if(NOT onnxruntime_POPULATED)
        message("Checking if [onnxruntime] dependency needs to be fetched...")
        FetchContent_Populate(onnxruntime)
    endif()

    set(ONNXRUNTIME_ROOTDIR ${onnxruntime_SOURCE_DIR})
    set(ONNX_INCLUDE_DIR ${onnxruntime_SOURCE_DIR}/include)
    set(ONNX_LIB_DIR ${onnxruntime_SOURCE_DIR}/lib/)
endif()

message("ONNX_INCLUDE_DIR set to ${ONNX_INCLUDE_DIR}")
message("Packaging [onnxruntime] to ${UE_PLUGIN_SIMLIBS_DIR}/shared_libs")
file(COPY "${ONNX_INCLUDE_DIR}" DESTINATION "${UE_PLUGIN_SIMLIBS_DIR}/shared_libs/onnxruntime")

# 根据平台拷贝不同后缀的动态库文件，并隔离 NVIDIA 专属宏
if(PROJECTAIRSIM_IS_MACOS)
    file(COPY "${ONNX_LIB_DIR}" DESTINATION "${UE_PLUGIN_SIMLIBS_DIR}/shared_libs" FILES_MATCHING PATTERN "*.dylib" PATTERN "*.dylib.*")
    message("Packaging [onnxruntime] to ${UNITY_WRAPPER_DLL_DIR}")
    file(COPY "${ONNX_LIB_DIR}/" DESTINATION "${UNITY_WRAPPER_DLL_DIR}/" FILES_MATCHING PATTERN "*.dylib" PATTERN "*.dylib.*")
else()
    file(COPY "${ONNX_LIB_DIR}" DESTINATION "${UE_PLUGIN_SIMLIBS_DIR}/shared_libs" FILES_MATCHING PATTERN "*onn*.dll" PATTERN "*onn*.lib" PATTERN "*.so*")
    message("Packaging [onnxruntime] to ${UNITY_WRAPPER_DLL_DIR}")
    file(COPY "${ONNX_LIB_DIR}/" DESTINATION "${UNITY_WRAPPER_DLL_DIR}/" FILES_MATCHING PATTERN "*onn*.dll" PATTERN "*onn*.lib" PATTERN "*.so*")

    # 仅在非 Mac 平台启用 CUDA 和 TensorRT
    add_definitions(-DUSE_CUDA)
    add_definitions(-DUSE_TENSORRT)
endif()

# if(WIN32)
#     # Set up dependency: cesium-native
#     message("Setting up [cesium-native] dependency as an external project...")
#     set (CESIUM_BUILD_TYPE $<IF:$<CONFIG:Debug>,RelWithDebInfo,Release>)
#     ExternalProject_Add(cesium-native
#         GIT_REPOSITORY          https://github.com/CesiumGS/cesium-native.git
#         GIT_TAG                 v0.3.0
#         GIT_CONFIG              "advice.detachedHead=false"
#         GIT_SUBMODULES_RECURSE  True
#         PREFIX                  ${CMAKE_BINARY_DIR}/_deps/cesium-native
#         UPDATE_COMMAND          ""  # disable update step
#         BUILD_COMMAND           ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CESIUM_BUILD_TYPE}
#         INSTALL_COMMAND         ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CESIUM_BUILD_TYPE} --target install
#         TEST_COMMAND            ""  # disable test step
#         TEST_EXCLUDE_FROM_MAIN  True
#         CMAKE_ARGS
#             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#             -DCMAKE_POSITION_INDEPENDENT_CODE=True
#             -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/_deps/cesium-native-install
#             -DCMAKE_INSTALL_MESSAGE=LAZY  # don't print out all files copied
#             -DCMAKE_BUILD_TYPE:STRING=${CESIUM_BUILD_TYPE}  # for single-config generators (Ninja)
#             -DCMAKE_CXX_FLAGS=${CESIUM_CXX_FLAGS}
#             -DCMAKE_EXE_LINKER_FLAGS=${CESIUM_LINKER_FLAGS}
#     )
#     set(CESIUM_LIB_DIR ${CMAKE_BINARY_DIR}/_deps/cesium-native-install/lib)
#     set(CESIUM_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/cesium-native-install/include)
#     set(CESIUM_PLUGIN_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/unreal/Blocks/Plugins/CesiumForUnreal/Source/ThirdParty)
#     message("CESIUM_LIB_DIR set to ${CESIUM_LIB_DIR}")
#     message("CESIUM_INCLUDE_DIR set to ${CESIUM_INCLUDE_DIR}")

#     ExternalProject_Add_Step(cesium-native post-install
#         COMMAND ${CMAKE_COMMAND} -E echo "Packaging [cesium-native] installed outputs to ${SIMLIBS_PACKAGE_DIR}/cesium-native/$<IF:$<CONFIG:Release>,Release,Debug>"
#         COMMAND ${CMAKE_COMMAND} -E copy_directory "${CESIUM_LIB_DIR}" "${SIMLIBS_PACKAGE_DIR}/cesium-native/$<IF:$<CONFIG:Release>,Release,Debug>"
#         COMMAND ${CMAKE_COMMAND} -E copy_directory "${CESIUM_INCLUDE_DIR}" "${SIMLIBS_PACKAGE_DIR}/cesium-native/include"
#         COMMAND ${CMAKE_COMMAND} -E remove_directory "${SIMLIBS_PACKAGE_DIR}/cesium-native/$<IF:$<CONFIG:Release>,Release,Debug>/cmake"
#         COMMAND ${CMAKE_COMMAND} -E remove_directory "${SIMLIBS_PACKAGE_DIR}/cesium-native/$<IF:$<CONFIG:Release>,Release,Debug>/pkgconfig"
#         COMMAND ${CMAKE_COMMAND} -E echo "Packaging [cesium-native] installed outputs to ${CESIUM_PLUGIN_THIRDPARTY_DIR}"
#         COMMAND ${CMAKE_COMMAND} -E copy_directory "${SIMLIBS_PACKAGE_DIR}/cesium-native/$<IF:$<CONFIG:Release>,Release,Debug>" "${CESIUM_PLUGIN_THIRDPARTY_DIR}/lib/$<IF:$<CONFIG:Release>,Release,Debug>"
#         COMMAND ${CMAKE_COMMAND} -E copy_directory "${SIMLIBS_PACKAGE_DIR}/cesium-native/include" "${CESIUM_PLUGIN_THIRDPARTY_DIR}/include"
#         COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/_deps/cesium-native/src/cesium-native/extern/stb/stb_image_resize.h" "${CESIUM_PLUGIN_THIRDPARTY_DIR}/include"
#         DEPENDEES install
#     )
# endif()

# Add sub-projects
add_subdirectory(core_sim)
add_subdirectory(simserver)
add_subdirectory(samples)
add_subdirectory(physics)
add_subdirectory(mavlinkcom)
add_subdirectory(rendering)
if(PROJECTAIRSIM_ENABLE_LVMON)
    add_subdirectory(tools)
endif()
add_subdirectory(unity)
add_subdirectory(vehicle_apis)
