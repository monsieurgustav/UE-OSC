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

            bEnableExceptions = true;
        }
	}
}
