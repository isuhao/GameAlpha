// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class GameAlpha : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "F:\\Epic Games\\MyProject\\GameAlpha\\Source\\ThirdParty\\LibNoise\\")); }
    }

	public GameAlpha(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                            "Core",
                            "CoreUObject",
                            "Engine",
                            "RenderCore",
                            "ShaderCore",
                            "RHI"
            }
        );
        PrivateIncludePaths.Add("GameAlpha/Grid");

        PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "src"));
        PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "lib", "libnoise.lib"));
    }
}
