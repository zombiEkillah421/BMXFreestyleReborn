#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMXTrainingPark.generated.h"

class UStaticMesh;
class UMaterialInterface;
class USceneComponent;

// A self-contained training park. All solid obstacles use their visible mesh
// as their collision surface: there are no invisible ramp collider substitutes.
UCLASS()
class BMXFREESTYLEREBORN_API ABMXTrainingPark : public AActor
{
    GENERATED_BODY()
public:
    ABMXTrainingPark();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> ParkRoot;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CubeMesh;
    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CylinderMesh;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> ConcreteMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> RampMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> LedgeMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> RailMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> WallMaterial;

    void AddBox(const FVector& Center, const FVector& Dimensions,
                const FRotator& Rotation = FRotator::ZeroRotator,
                UMaterialInterface* Material = nullptr);
    void AddRail(const FVector& Center, float Length, float Height);
    void AddRamp(const FVector& Origin, const FRotator& Rotation,
                 float Length, float Width, float Height, bool bCurved);
};
