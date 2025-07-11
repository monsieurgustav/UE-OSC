using System;
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UE_OSC : ModuleRules
	{
		public UE_OSC(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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

            PrivateDefinitions.Add("_CRT_SECURE_NO_WARNINGS=1");
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
