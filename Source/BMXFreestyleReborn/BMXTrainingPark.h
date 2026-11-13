#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMXTrainingPark.generated.h"

class UStaticMesh;
class UMaterialInterface;
class USceneComponent;

// v0.4.0: a straight grindable edge in world space (rail top, ledge or
// deck edge). The rider snaps to these.
struct FBMXGrindSegment
{
    FVector Start = FVector::ZeroVector;
    FVector End = FVector::ZeroVector;
    FString Label;
};

// A self-contained training park. All solid obstacles use their visible mesh
// as their collision surface: there are no invisible ramp collider substitutes.
UCLASS()
class BMXFREESTYLEREBORN_API ABMXTrainingPark : public AActor
{
    GENERATED_BODY()
public:
    ABMXTrainingPark();
    const TArray<FBMXGrindSegment>& GetGrindSegments() const { return GrindSegments; }

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
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInterface> DirtMaterial;

    void AddBox(const FVector& Center, const FVector& Dimensions,
                const FRotator& Rotation = FRotator::ZeroRotator,
                UMaterialInterface* Material = nullptr);
    void AddRail(const FVector& Center, float Length, float Height);
    void AddRamp(const FVector& Origin, const FRotator& Rotation,
                 float Length, float Width, float Height, bool bCurved);
    // v0.3.0: a closed, rideable mound from a side profile (X along, Z up),
    // used for dirt lips and landings. Profile must start and end at Z >= 0.
    void AddProfileMound(const FVector& Origin, float Yaw, float Width,
                         const TArray<FVector2D>& Profile, UMaterialInterface* Material);
    void AddDirtJump(float LipStartX, float Y);
    // Registers a grindable edge given in park-local space.
    void AddGrindEdge(const FVector& LocalStart, const FVector& LocalEnd, const TCHAR* Label);
    // Both long top edges of an axis-aligned box (ledges, tabletop deck).
    void AddBoxGrindEdges(const FVector& Center, const FVector& Dimensions, const TCHAR* Label);
    TArray<FBMXGrindSegment> GrindSegments;
};
