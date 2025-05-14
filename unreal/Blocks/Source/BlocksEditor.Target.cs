// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BlocksEditorTarget : TargetRules
{
	public BlocksEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;  // from UE5.1
		ExtraModuleNames.AddRange(new string[] { "Blocks" });

		// Uncomment the below options to disable Unity file merging or PCHs to
		// prevent sharing include dependencies with UE
		// bUseUnityBuild = false;
		// bUsePCHFiles = false;
		// bUseSharedPCHs = false;
	}
}
