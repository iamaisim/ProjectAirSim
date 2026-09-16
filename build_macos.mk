# ---------------------------------------------------------------------------------------------------------------------
#
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.
#
# Module Name:
#
#   build_macos.mk
#
# Abstract:
#
#   Main makefile for macOS builds to drive CMake commands.
#
# ---------------------------------------------------------------------------------------------------------------------

REDIRECT_OUTPUT = > /dev/null 2>&1

default:
	@echo "======================================================================="
	@echo "No target specified. Please run './build.sh [target]' using the following targets:"
	@echo
	@echo " all = Build and test Debug and Release SimLibs, then package them"
	@echo " rebuild_all = Clean, build, test, and package Debug and Release SimLibs"
	@echo " clean = Clean macOS SimLibs build files and staged outputs"
	@echo
	@echo " simlibs_debug = Build SimLibs for Debug"
	@echo " simlibs_release = Build SimLibs for Release"
	@echo " test_simlibs_debug = Test SimLibs for Debug"
	@echo " test_simlibs_release = Test SimLibs for Release"
	@echo " package_simlibs = Package Debug and Release SimLibs"
	@echo

.PHONY: all
all: simlibs_debug test_simlibs_debug simlibs_release test_simlibs_release package_simlibs

.PHONY: rebuild_all
rebuild_all: clean all

CMAKE_BUILD_DIR = build/macos
OPENSSL_ROOT_DIR ?= $(shell brew --prefix openssl@3)
ZLIB_HOME ?= $(shell brew --prefix zlib)
CMAKE_EXTRA_ARGS ?=
CMAKE_CMD = cmake $(CMAKE_EXTRA_ARGS) -G "Ninja" \
	-DOPENSSL_ROOT_DIR="$(OPENSSL_ROOT_DIR)" \
	-DZLIB_HOME="$(ZLIB_HOME)"
CMAKE_DBG_BUILD_CMD = cmake --build $(CMAKE_BUILD_DIR)/Debug
CMAKE_REL_BUILD_CMD = cmake --build $(CMAKE_BUILD_DIR)/Release

.PHONY: config_simlibs_debug
config_simlibs_debug:
	@echo "======================================================================="
	@echo "Configuring the ProjectAirSimLibs project for macOS-Debug..."
	mkdir -p $(CMAKE_BUILD_DIR)/Debug $(REDIRECT_OUTPUT)
	cd $(CMAKE_BUILD_DIR)/Debug && $(CMAKE_CMD) -DCMAKE_BUILD_TYPE=Debug ../../..

.PHONY: simlibs_debug
simlibs_debug: config_simlibs_debug
	@echo "======================================================================="
	@echo "Building the ProjectAirSimLibs project for macOS-Debug..."
	$(CMAKE_DBG_BUILD_CMD)

.PHONY: config_simlibs_release
config_simlibs_release:
	@echo "======================================================================="
	@echo "Configuring the ProjectAirSimLibs project for macOS-Release..."
	mkdir -p $(CMAKE_BUILD_DIR)/Release $(REDIRECT_OUTPUT)
	cd $(CMAKE_BUILD_DIR)/Release && $(CMAKE_CMD) -DCMAKE_BUILD_TYPE=Release ../../..

.PHONY: simlibs_release
simlibs_release: config_simlibs_release
	@echo "======================================================================="
	@echo "Building the ProjectAirSimLibs project for macOS-Release..."
	$(CMAKE_REL_BUILD_CMD)

.PHONY: test_simlibs_debug
test_simlibs_debug: simlibs_debug
	@echo "======================================================================="
	@echo "Testing the ProjectAirSimLibs project for macOS-Debug..."
	cmake --build $(CMAKE_BUILD_DIR)/Debug --target simlibs_unit_tests
	ctest --test-dir $(CMAKE_BUILD_DIR)/Debug -C Debug --output-on-failure

.PHONY: test_simlibs_release
test_simlibs_release: simlibs_release
	@echo "======================================================================="
	@echo "Testing the ProjectAirSimLibs project for macOS-Release..."
	cmake --build $(CMAKE_BUILD_DIR)/Release --target simlibs_unit_tests
	ctest --test-dir $(CMAKE_BUILD_DIR)/Release -C Release --output-on-failure

.PHONY: package_simlibs
package_simlibs: simlibs_debug simlibs_release
	@echo "======================================================================="
	@echo "Packaging SimLibs for use in custom projects..."
	mkdir -p $(CURDIR)/packages/projectairsim_simlibs/
	rsync -a $(CURDIR)/unreal/Blocks/Plugins/ProjectAirSim/SimLibs/ \
		$(CURDIR)/packages/projectairsim_simlibs/
	@echo "Packaging completed to: $(CURDIR)/packages/projectairsim_simlibs"

.PHONY: clean
clean:
	@echo "======================================================================="
	@echo "Cleaning macOS build files..."
	rm -fr $(CMAKE_BUILD_DIR) $(REDIRECT_OUTPUT)
	rm -fr physics/matlab_sfunc/_deps $(REDIRECT_OUTPUT)
	rm -fr physics/matlab_sfunc/message $(REDIRECT_OUTPUT)
	rm -fr packages/projectairsim_simlibs $(REDIRECT_OUTPUT)
	rm -fr unreal/Blocks/Plugins/ProjectAirSim/SimLibs $(REDIRECT_OUTPUT)
	rm -fr unity/BlocksUnity/Assets/Plugins/* $(REDIRECT_OUTPUT)
