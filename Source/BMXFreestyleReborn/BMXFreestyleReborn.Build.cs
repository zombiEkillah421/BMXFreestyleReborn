using UnrealBuildTool;

public class BMXFreestyleReborn : ModuleRules
{
    public BMXFreestyleReborn(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent" });
    }
}
