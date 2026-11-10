#include "BMXGameMode.h"
#include "BMXRiderPawn.h"
#include "BMXTrainingPark.h"
#include "BMXHUD.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"

ABMXGameMode::ABMXGameMode()
{
    DefaultPawnClass = ABMXRiderPawn::StaticClass();
    HUDClass = ABMXHUD::StaticClass();
}

void ABMXGameMode::StartPlay()
{
    // No .umap, Blueprint, or imported artwork is required: the practice park is created at play time.
    if (UWorld* World = GetWorld())
    {
        World->SpawnActor<ABMXTrainingPark>(FVector::ZeroVector, FRotator::ZeroRotator);
        // The empty Entry map otherwise renders a black horizon.
        World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator);
        ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0, 0, 1500), FRotator(-55, -35, 0));
        if (Sun && Sun->GetLightComponent())
        {
            Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
            Sun->GetLightComponent()->SetIntensity(7.0f);
            if (UDirectionalLightComponent* SunComponent = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
            {
                SunComponent->SetAtmosphereSunLight(true);
            }
        }
        ASkyLight* Sky = World->SpawnActor<ASkyLight>();
        if (Sky && Sky->GetLightComponent())
        {
            Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
            Sky->GetLightComponent()->SetIntensity(1.2f);
            Sky->GetLightComponent()->RecaptureSky();
        }
    }
    Super::StartPlay();
}

void ABMXGameMode::RestartPlayer(AController* NewPlayer)
{
    // The shipped source runs on an empty Engine Entry map with no PlayerStart.
    // Spawn from a known safe point instead of relying on level authoring.
    if (!NewPlayer || !GetWorld()) return;
    if (NewPlayer->GetPawn()) return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ABMXRiderPawn* Rider = GetWorld()->SpawnActor<ABMXRiderPawn>(
        FVector(-150.0f,-1150.0f,200.0f), FRotator(0.0f, 180.0f, 0.0f), Params);
    if (Rider) NewPlayer->Possess(Rider);
}
