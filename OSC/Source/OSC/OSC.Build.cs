namespace UnrealBuildTool.Rules
{
	public class OSC : ModuleRules
	{
		public OSC(ReadOnlyTargetRules Target) : base(Target)
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

            PublicIncludePaths.AddRange(
				new string[] {
					"OSC/Public",
					"OSC/Public/Common",
					"OSC/Public/Receive",
					"OSC/Public/Send",
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

            if (Target.Type == TargetRules.TargetType.Editor)
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
