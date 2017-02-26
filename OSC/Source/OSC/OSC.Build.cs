namespace UnrealBuildTool.Rules
{
	public class OSC : ModuleRules
	{
		public OSC(TargetInfo Target)
		{
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
					"OSC/Private/Send",
				}
			);

            if (Target.Type.HasValue && Target.Type.Value == TargetRules.TargetType.Editor)
            {
                Definitions.Add("OSC_EDITOR_BUILD=1");

                PrivateDependencyModuleNames.Add("UnrealEd");
            }
            else
            {
                Definitions.Add("OSC_EDITOR_BUILD=0");
            }
        }
	}
}
