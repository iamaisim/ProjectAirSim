// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BlocksEditorTarget : TargetRules
{
	public BlocksEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;  // from UE5.1
		ExtraModuleNames.AddRange(new string[] { "Blocks" });

		// Uncomment the below options to disable Unity file merging or PCHs to
		// prevent sharing include dependencies with UE
		// bUseUnityBuild = false;
		// bUsePCHFiles = false;
		// bUseSharedPCHs = false;
	}
}
