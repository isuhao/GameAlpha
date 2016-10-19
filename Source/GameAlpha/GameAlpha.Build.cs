// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class GameAlpha : ModuleRules
{
    private string ModulePath
    {
        get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
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
        PublicIncludePaths.Add("ThirdParty\\LibNoise\\src");
        PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "..\\ThirdParty\\LibNoise\\lib\\libnoise.lib"));
    }
}
