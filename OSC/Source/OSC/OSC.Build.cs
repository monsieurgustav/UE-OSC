namespace UnrealBuildTool.Rules
{
	public class OSC : ModuleRules
	{
		public OSC(ReadOnlyTargetRules Target) : base(Target)
        {
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

			PrivateIncludePaths.AddRange(
				new string[] {
					"OSC/Private",
					"OSC/Private/Common",
					"OSC/Private/Receive",
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
