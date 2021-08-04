using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UE4_OSC : ModuleRules
	{
		public UE4_OSC(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivatePCHHeaderFile = "Private/OscPrivatePCH.h";

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Networking",
				}
			); 

			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
                    "Engine",
					"Sockets",
                    "InputCore",
                    "Slate",
                    "SlateCore",
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"Settings",
				}
			);

			if (Target.Type == TargetRules.TargetType.Editor)
            {
                PublicDefinitions.Add("OSC_EDITOR_BUILD=1");

                PrivateDependencyModuleNames.Add("UnrealEd");
            }
            else
            {
                PublicDefinitions.Add("OSC_EDITOR_BUILD=0");
            }
        }
	}
}
