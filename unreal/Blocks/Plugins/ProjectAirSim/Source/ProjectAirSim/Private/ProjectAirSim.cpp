// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "ProjectAirSim.h"

#include "GlobalShader.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FProjectAirSimModule"

void FProjectAirSimModule::StartupModule() {
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module
  FString ShaderDirectory = FPaths::Combine(
      FPaths::ProjectDir(),
      TEXT("Plugins/ProjectAirSim/Source/ProjectAirSim/Private/Shaders"));
  AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FProjectAirSimModule::ShutdownModule() {
  // This function may be called during shutdown to clean up your module.  For
  // modules that support dynamic reloading, we call this function before
  // unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FProjectAirSimModule, ProjectAirSim)