#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BMXRiderPawn.generated.h"

class UCapsuleComponent;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;

enum class EBMXTrickType : uint8 { None, Barspin, Tailwhip, YawSpin, Flip };

struct FBMXTrick
{
    FString Name;
    EBMXTrickType Type = EBMXTrickType::None;
    int32 Points = 0;
    float Duration = 0.0f;
    float Elapsed = 0.0f;
    float Rotation = 0.0f;
};

// Non-reflected runtime contact data, intentionally outside the reflected UCLASS.
struct FBMXWheelContact
{
    bool bSupported = false;
    float FloorZ = 0.0f;
    FVector Normal = FVector::UpVector;
};
struct FBMXRidingSupport
{
    FBMXWheelContact Front;
    FBMXWheelContact Rear;
    bool HasContact() const { return Front.bSupported || Rear.bSupported; }
    float AverageFloorZ() const
    {
        if (Front.bSupported && Rear.bSupported)
            return (Front.FloorZ + Rear.FloorZ) * 0.5f;
        return Front.bSupported ? Front.FloorZ : Rear.FloorZ;
    }
    float SurfacePitchDegrees() const
    {
        if (!Front.bSupported || !Rear.bSupported) return 0.0f;
        return FMath::RadiansToDegrees(FMath::Atan2(Front.FloorZ - Rear.FloorZ, 126.0f));
    }
    bool HasRampContact() const
    {
        return (Front.bSupported && Front.Normal.Z < 0.96f) ||
               (Rear.bSupported && Rear.Normal.Z < 0.96f);
    }
};

UCLASS()
class BMXFREESTYLEREBORN_API ABMXRiderPawn : public APawn
{
    GENERATED_BODY()
public:
    ABMXRiderPawn();
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    int32 GetTotalScore() const { return TotalScore; }
    float GetSpeedKmh() const { return ActualSpeedCmPerSec * 0.036f; }
    float GetPedalInput() const { return ThrottleInput; }
    bool IsGrounded() const { return bGrounded; }
    bool IsMovementBlocked() const { return bMovementBlocked; }
    bool HasFrontWheelContact() const { return bFrontWheelContact; }
    bool HasRearWheelContact() const { return bRearWheelContact; }
    float GetPitchDegrees() const { return FloorPitchDeg; }
    FString GetBlockerName() const { return LastBlockerName; }
    bool HasCombo() const { return CompletedComboTricks.Num() > 0; }
    int32 GetComboPoints() const;
    float GetComboMultiplier() const;
    FString GetComboNames() const;
    FString GetStatusMessage() const { return StatusMessage; }

private:
    UPROPERTY(VisibleAnywhere, Category="BMX|Collision")
    TObjectPtr<UCapsuleComponent> Collider;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> TrickPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> BikeBodyPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> HandlePivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> RearWheelPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> FrontWheelPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> LeftLegPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> RightLegPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;
    UPROPERTY(VisibleAnywhere, Category="BMX|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UStaticMeshComponent* MakePart(FName Name, USceneComponent* Parent, UStaticMesh* Mesh,
                                   const FVector& Offset, const FVector& Scale,
                                   const FRotator& Rotation = FRotator::ZeroRotator);
    void MakeRod(FName Name, USceneComponent* Parent, UStaticMesh* Cube,
                 const FVector& Start, const FVector& End, float Thickness);
    void MakeWheel(const FString& Prefix, USceneComponent* Parent,
                   UStaticMesh* Cube, UStaticMesh* Cylinder);
    void UpdateVisualMotion(float Dt);
    // Front and rear tire probes are independent of the compact Pawn collider.
    bool ProbeWheel(const FVector& PawnCenter, float ForwardOffset, FBMXWheelContact& Contact,
                    float ProbeHeight = 96.0f) const;
    FBMXRidingSupport ProbeSupport(const FVector& PawnCenter, float ProbeHeight = 96.0f) const;
    bool IsSupportReachable(const FVector& Origin, const FBMXRidingSupport& Support) const;
    void MoveBike(float Dt);
    void UpdateCamera(float Dt);
    void UpdateTricks(float Dt);
    void ResetVisuals();
    void AddTrick(const TCHAR* Label, EBMXTrickType Type, int32 Points, float Duration, float Rotation = 360.0f);
    void BeginNextTrick();
    void BankCombo();
    void Bail(const FString& Reason);
    void ResetBike();
    void ShowStatus(const FString& Message, float Seconds = 1.5f);

    void SetThrottle(float Value);
    void SetSteering(float Value);
    void OrbitYaw(float Value);
    void OrbitPitch(float Value);
    void Hop();
    void Barspin();
    void Tailwhip();
    void SpinLeft();
    void SpinRight();
    void Backflip();
    void Frontflip();
    void RecenterCamera();

    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float MaxForwardSpeed = 1370.0f; // cm/s, about 49 km/h
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float Acceleration = 840.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float SteeringRateDeg = 115.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float HopSpeed = 615.0f;

    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float SpeedCmPerSec = 0.0f;
    float ActualSpeedCmPerSec = 0.0f; // measured speed, independent of wall contact
    float VerticalSpeed = 0.0f;
    bool bGrounded = true;
    bool bBailed = false;
    bool bMovementBlocked = false;
    FString LastBlockerName;
    float StuckAirSeconds = 0.0f;
    float LeftSurfaceSeconds = 0.0f;
    float FrontWheelFloorZ = 0.0f;
    float RearWheelFloorZ = 0.0f;
    bool bFrontWheelContact = false;
    bool bRearWheelContact = false;
    float LandingCooldown = 0.0f;
    float AirTime = 0.0f;
    float JumpGrace = 0.0f;
    float BailCooldown = 0.0f;
    float FloorPitchDeg = 0.0f;
    float WheelRotationDeg = 0.0f;
    float PedalPhase = 0.0f;
    float CameraYawDeg = 0.0f;
    float CameraPitchDeg = -12.0f;
    float CameraIdleSeconds = 0.0f;
    FVector ResetLocation = FVector(-150.0f, -1150.0f, 60.0f);
    FRotator ResetRotation = FRotator(0.0f, 180.0f, 0.0f);

    int32 TotalScore = 0;
    int32 UnbankedPoints = 0;
    TArray<FString> CompletedComboTricks;
    TArray<FBMXTrick> TrickQueue;
    FBMXTrick ActiveTrick;
    bool bHasActiveTrick = false;
    FString StatusMessage;
    float StatusSeconds = 0.0f;
};
