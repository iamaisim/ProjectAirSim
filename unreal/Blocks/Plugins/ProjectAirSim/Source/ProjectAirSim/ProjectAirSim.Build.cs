// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class ProjectAirSim : ModuleRules
{
    public ProjectAirSim(ReadOnlyTargetRules Target) : base(Target)
    {
        CppStandard = CppStandardVersion.Cpp17;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = "Public/ProjectAirSim.h";

        // Allow Unreal's default setting for IWYU instead of setting explicitly
        // bEnforceIWYU = true;  // UE <= 5.1
        // IWYUSupport = IWYUSupport.None;  // UE 5.2+

        bEnableExceptions = true;

        // Silence MSVC 14.25.28610 deprecation warning from Eigen
        PublicDefinitions.Add("_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING");

        string buildType = (Target.Configuration == UnrealTargetConfiguration.Debug ||
                            Target.Configuration == UnrealTargetConfiguration.DebugGame)
                            ? "Debug"
                            : "Release";

        PublicIncludePaths.AddRange(
            new string[] {
                ModuleDirectory + "/Public"
                // ... add public include paths required here ...
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
                EngineDirectory + "/Source/Runtime/Renderer/Private"
            }
        );

        // TODO: Can we do something to add includes and libraries for features
        // in a less manual fashion?
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            List<string> liststrIncludes = new List<string> {
                    ModuleDirectory + "/Private",
                    PluginDirectory + "/SimLibs/core_sim/include",
                    PluginDirectory + "/SimLibs/simserver/include",
                    PluginDirectory + "/SimLibs/physics/include",
                    PluginDirectory + "/SimLibs/rendering_scene/include",
                    PluginDirectory + "/SimLibs/multirotor_api/include",
                    PluginDirectory + "/SimLibs/rover_api/include",
                    PluginDirectory + "/SimLibs/mavlinkcom/include",
                    PluginDirectory + "/SimLibs/eigen/include",
                    PluginDirectory + "/SimLibs/assimp/include",
                    PluginDirectory + "/SimLibs/json/include",
                    PluginDirectory + "/SimLibs/nng/include",
                    PluginDirectory + "/SimLibs/shared_libs/onnxruntime/include"
                    // ... add other private include paths required here ...
                };

            if (buildType == "Debug")
                liststrIncludes.Add(PluginDirectory + "/SimLibs/lvmon/include");

            PrivateIncludePaths.AddRange(liststrIncludes);
        }
        else
        {
            List<string> liststrIncludes = new List<string> {
                    ModuleDirectory + "/Private",
                    PluginDirectory + "/SimLibs/core_sim/include",
                    PluginDirectory + "/SimLibs/simserver/include",
                    PluginDirectory + "/SimLibs/physics/include",
                    PluginDirectory + "/SimLibs/multirotor_api/include",
                    PluginDirectory + "/SimLibs/rover_api/include",
                    PluginDirectory + "/SimLibs/rendering_scene/include",
                    PluginDirectory + "/SimLibs/mavlinkcom/include",
                    PluginDirectory + "/SimLibs/eigen/include",
                    PluginDirectory + "/SimLibs/assimp/include",
                    PluginDirectory + "/SimLibs/json/include",
                    PluginDirectory + "/SimLibs/nng/include",
                    PluginDirectory + "/SimLibs/shared_libs/onnxruntime/include",
                    // PluginDirectory + "/SimLibs/cesium-native/include"
                    // ... add other private include paths required here ...
                };

            if (buildType == "Debug")
                liststrIncludes.Add(PluginDirectory + "/SimLibs/lvmon/include");

            PrivateIncludePaths.AddRange(liststrIncludes);
        }

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",  // for base C++ project
                "CoreUObject",  // for base C++ project
                "Engine",  // for base C++ project
                "InputCore",  // for base C++ project
                "ImageWrapper",
                "RenderCore",
                "Renderer",
                "RHI",
                "Landscape",
                "UMG",		// For weather features (Widgets),
                "PhysicsCore",  // For UPhysicalMaterial in UE 4.26
                "CinematicCamera",
                "Niagara",
                // ... add other public dependencies that you statically link with here ...
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "UMG",
                "Core",  // default
                "CoreUObject",
                "Engine",
                "MovieSceneCapture",
                "RenderCore",
                "Renderer",
                "RHI",
                "Slate",
                "SlateCore",
                "Projects",  // default
                "ProceduralMeshComponent",
                "PixelStreaming",
                "SunPosition"
                // ... add private dependencies that you statically link with here ...
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                // ... add any modules that your module loads dynamically here ...
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            List<string> liststrLibraries = new List<string> {
                    PluginDirectory + "/SimLibs/core_sim/" + buildType + "/core_sim.lib",
                    PluginDirectory + "/SimLibs/simserver/" + buildType + "/simserver.lib",
                    PluginDirectory + "/SimLibs/physics/" + buildType + "/physics.lib",
                    PluginDirectory + "/SimLibs/multirotor_api/" + buildType + "/multirotor_api.lib",
                    PluginDirectory + "/SimLibs/rover_api/" + buildType + "/rover_api.lib",
                    PluginDirectory + "/SimLibs/rendering_scene/" + buildType + "/rendering_scene.lib",
                    PluginDirectory + "/SimLibs/mavlinkcom/" + buildType + "/mavlinkcom.lib",
                    PluginDirectory + "/SimLibs/nng/" + buildType + "/nng.lib",
                    PluginDirectory + "/SimLibs/assimp/" + buildType + "/assimp-mt.lib",
                    PluginDirectory + "/SimLibs/assimp/" + buildType + "/zlibstatic.lib",
                    PluginDirectory + "/SimLibs/shared_libs/onnxruntime.lib",
                };

            if (buildType == "Debug")
                liststrLibraries.Add(PluginDirectory + "/SimLibs/lvmon/" + buildType + "/lvmon.lib");

            PublicAdditionalLibraries.AddRange(liststrLibraries);
            PublicSystemLibraries.AddRange(
                new string[] {
                    "ws2_32.lib",  // req by nng for Win
                    "mswsock.lib",  // req by nng for Win
                    "advapi32.lib",  // req by nng for Win
                    "Bcrypt.lib",  // Client authorization for Win
                    "Crypt32.lib",  // Client authorization for Win
                    "Ncrypt.lib",  // Client authorization for Win
                }
            );

            var onnx_files = Directory.GetFiles(PluginDirectory + "/SimLibs/shared_libs/", "*.dll");
            foreach (var file in onnx_files)
            {
                var fileName = Path.GetFileName(file);
                RuntimeDependencies.Add("$(BinaryOutputDir)/" + fileName, PluginDirectory + "/SimLibs/shared_libs/" + fileName);
            }
        }
        else
        {
            // PublicDefinitions.Add("ENABLE_CESIUM");
             
            List<string> liststrLibraries = new List<string> {
                    PluginDirectory + "/SimLibs/core_sim/" + buildType + "/libcore_sim.a",
                    PluginDirectory + "/SimLibs/simserver/" + buildType + "/libsimserver.a",
                    PluginDirectory + "/SimLibs/physics/" + buildType + "/libphysics.a",
                    PluginDirectory + "/SimLibs/multirotor_api/" + buildType + "/libmultirotor_api.a",
                    PluginDirectory + "/SimLibs/rover_api/" + buildType + "/librover_api.a",
                    PluginDirectory + "/SimLibs/rendering_scene/" + buildType + "/librendering_scene.a",
                    PluginDirectory + "/SimLibs/mavlinkcom/" + buildType + "/libmavlinkcom.a",
                    PluginDirectory + "/SimLibs/nng/" + buildType + "/libnng.a",
                    PluginDirectory + "/SimLibs/assimp/" + buildType + "/libassimp.a",
                    PluginDirectory + "/SimLibs/assimp/" + buildType + "/libzlibstatic.a",
                    PluginDirectory + "/SimLibs/shared_libs/libonnxruntime.so",
                };

            if (buildType == "Debug")
                liststrLibraries.Add(PluginDirectory + "/SimLibs/lvmon/" + buildType + "/liblvmon.a");

            var onnx_files = Directory.GetFiles(PluginDirectory + "/SimLibs/shared_libs");
            foreach (var file in onnx_files)
            {
                var fileName = Path.GetFileName(file);
                RuntimeDependencies.Add("$(BinaryOutputDir)/" + fileName, PluginDirectory + "/SimLibs/shared_libs/" + fileName);
            }

            PublicAdditionalLibraries.AddRange(liststrLibraries);
            PublicSystemLibraries.AddRange(
                new string[] {
                    "stdc++",
                    "supc++",
                    "pthread",
                    "anl"
                }
            );
        }
    }
}
