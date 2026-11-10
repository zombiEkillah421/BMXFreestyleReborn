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

// v0.3.0: Bike/body "style" tricks. They layer on top of free flip/spin
// rotation, so e.g. a backflip + tailwhip or a 360 + superman both work.
// Timed tricks (barspin, tailwhip) play once; hold tricks (no-hander,
// superman...) last while the button is held and score more the longer.
enum class EBMXStyleTrick : uint8
{
    None, Barspin, Tailwhip, NoHander, Superman, CanCan, XUp, Turndown, Tabletop, Nothing, TuckNoHander
};

// Non-reflected runtime contact data, intentionally outside the reflected UCLASS.
struct FBMXWheelContact
{
    bool bSupported = false;
    bool bQuarterPipe = false; // surface tagged BMX_QUARTER (vert airs allowed)
    float FloorZ = 0.0f;
    FVector Normal = FVector::UpVector;
};
struct FBMXRidingSupport
{
    FBMXWheelContact Front;
    FBMXWheelContact Rear;
    bool HasContact() const { return Front.bSupported || Rear.bSupported; }
    bool OnQuarterPipe() const
    {
        return (Front.bSupported && Front.bQuarterPipe) || (Rear.bSupported && Rear.bQuarterPipe);
    }
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
    bool IsVertAir() const { return bVertAir; }
    bool IsInManual() const { return bInManual; }
    float GetManualBalance() const { return ManualBalance; }
    bool IsRidingFakie() const { return bGrounded && SpeedCmPerSec < -40.0f; }
    float GetFlipAngle() const { return FlipAngleDeg; }
    float GetSpinAngle() const { return SpinAngleDeg; }
    bool IsDebugHUDVisible() const { return bShowDebugHUD; }

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
    // v0.3.0: torso/head/arms pivot at the hips, and shoulder pivots, so
    // poses like superman and no-hander can move the upper body.
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> RiderUpperPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> LeftArmPivot;
    UPROPERTY(VisibleAnywhere, Category="BMX|Visuals")
    TObjectPtr<USceneComponent> RightArmPivot;
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
    void UpdateGroundSpeed(float Dt);
    // Leaves the current surface, splitting along-surface speed into
    // horizontal travel and upward velocity. Steep quarter-pipe exits
    // become vert airs that turn the rider back into the ramp.
    void LaunchIntoAir(float ExtraUpSpeed, float MinUpSpeed);
    void UpdateCamera(float Dt);
    void UpdateAirRotation(float Dt);
    void UpdateStyleTrick(float Dt);
    void UpdateManual(float Dt);
    // Scores rotations, checks the landing is clean and turns the actor
    // round for 180s / vert airs. Returns false after calling Bail().
    bool ResolveLanding(float& InOutLandedPitch);
    void AfterCleanLanding();
    void StartManual();
    void EndManual(bool bBank);
    void AddComboTrick(const FString& Name, int32 Points);
    void ResetVisuals();
    void BankCombo();
    void Bail(const FString& Reason);
    void ResetBike();
    void ShowStatus(const FString& Message, float Seconds = 1.5f);

    void SetThrottle(float Value);
    void SetSteering(float Value);
    void OrbitYaw(float Value);
    void OrbitPitch(float Value);
    void HopPressed();
    void HopReleased();
    void DoHop(float Power);
    void ManualPressed();
    void ManualReleased();
    void SetFlipAxis(float Value);
    void SetSpinAxis(float Value);
    void SetAirStickX(float Value);
    void SetAirStickY(float Value);
    void StylePressed(EBMXStyleTrick Trick);
    void StyleReleased(EBMXStyleTrick Trick);
    void ResetToDirtLine();
    void ResetToPark();
    void RecenterCamera();
    void ToggleDebugHUD();

    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float MaxForwardSpeed = 1370.0f; // cm/s, about 49 km/h
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float Acceleration = 840.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float SteeringRateDeg = 115.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float HopSpeed = 615.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float ReverseSpeed = 420.0f;
    // Rolling resistance when not pedaling (cm/s^2). Also limits speed above max.
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float CoastDrag = 140.0f;
    // Drag when already faster than pedaling allows (holding W).
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float HighSpeedDrag = 60.0f;
    // Extra acceleration from pumping (holding W) down a transition.
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float PumpAcceleration = 700.0f;
    // How strongly slopes speed you up / slow you down (1.0 = real gravity).
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float SlopeGravityScale = 0.85f;
    // Share of uphill speed that becomes upward velocity at a ramp lip.
    UPROPERTY(EditAnywhere, Category="BMX|Tuning")
    float LaunchLiftScale = 0.85f;
    // Leaving a ramp steeper than this (stored chord pitch) triggers a vert air.
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float VertLaunchPitchDeg = 30.0f; // tabletop wedges are ~25 deg
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float VertLaunchScale = 0.70f;
    // Share of launch speed given back when dropping into a quarter-pipe.
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float VertLandingKeep = 0.95f;
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float MinVertUpSpeed = 300.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float MaxVertUpSpeed = 1100.0f; // ~6 m above the coping
    // How far (cm) the rider drifts back over the ramp during a vert air,
    // so both wheels come down on the transition instead of the coping.
    UPROPERTY(EditAnywhere, Category="BMX|Vert")
    float VertDriftCm = 60.0f;
    // Air rotation speeds (deg/s) for flips (W/S, F/G) and spins (A/D, Q/E).
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float FlipRateDeg = 430.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float SpinRateDeg = 540.0f;
    // How far off a full rotation you can land and still ride away.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float LandingToleranceDeg = 50.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float ManualPointsPerSecond = 60.0f;
    // Manual feel: how fast an off-balance manual tips further, and how
    // strongly W/S correct it.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float ManualInstability = 0.9f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float ManualControl = 1.6f;
    // Holding Space charges the bunny hop: a tap = MinHopScale, full charge = MaxHopScale.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float HopChargeSeconds = 0.35f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float MinHopScale = 0.75f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float MaxHopScale = 1.1f;
    // Gravity multiplier for bunny hops off flat ground: snappier, less floaty.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float HopGravityScale = 1.6f;
    // Bunny hop front-wheel lift (degrees, pivoting on the rear axle):
    // a light tap lifts to Min, a full charge to Max.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float HopFrontLiftMinDeg = 8.0f;
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float HopFrontLiftMaxDeg = 26.0f;
    // A tap waits this long for the front to come up before popping.
    UPROPERTY(EditAnywhere, Category="BMX|Tricks")
    float HopFrontLiftSeconds = 0.08f;

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
    bool bVertAir = false;
    float VertAirElapsed = 0.0f;
    float VertAirTurnSeconds = 0.5f;
    float VertAirYawDeg = 0.0f;
    float VertAirLaunchPitchDeg = 0.0f;
    bool bShowDebugHUD = true;
    bool bOnQuarterPipe = false;
    float SpawnRollSpeed = 0.0f;

    // Air rotation (v0.3.0). Positive flip = nose up (backflip); positive spin = right.
    float FlipAxisInput = 0.0f;
    float SpinAxisInput = 0.0f;
    float FlipAngleDeg = 0.0f;
    float SpinAngleDeg = 0.0f;
    float FlipVelocityDeg = 0.0f;
    float SpinVelocityDeg = 0.0f;
    // W/S and A/D only start rotating once released after takeoff, so holding
    // W to pedal into a jump doesn't throw an accidental frontflip.
    bool bStickFlipArmed = false;
    bool bStickSpinArmed = false;
    float AirStickX = 0.0f;
    float AirStickY = 0.0f;
    bool bManualInputArmed = false;

    // Style tricks.
    EBMXStyleTrick ActiveStyle = EBMXStyleTrick::None;
    EBMXStyleTrick PoseStyle = EBMXStyleTrick::None;
    float StyleElapsed = 0.0f;
    float StylePoseAlpha = 0.0f;
    float StyleHoldPointAccum = 0.0f;
    bool bStyleHeld = false;
    bool bStyleScored = false;

    // Manuals and combo linking.
    bool bManualHeld = false;
    bool bInManual = false;
    float ManualBalance = 0.0f;
    float ManualSeconds = 0.0f;
    float ManualAlpha = 0.0f;
    float ManualPointAccum = 0.0f;
    float LandingLinkSeconds = 0.0f;

    // Charged bunny hop.
    bool bHopCharging = false;
    float HopCharge = 0.0f;
    bool bHopPending = false;
    float HopPendingSeconds = 0.0f;
    float HopPendingPower = 1.0f;
    bool bHopLeveling = false;
    float HopLiftPitchDeg = 0.0f;
    bool bHopAir = false;
    FVector ResetLocation = FVector(-150.0f, -1150.0f, 60.0f);
    FRotator ResetRotation = FRotator(0.0f, 180.0f, 0.0f);

    int32 TotalScore = 0;
    int32 UnbankedPoints = 0;
    TArray<FString> CompletedComboTricks;
    FString StatusMessage;
    float StatusSeconds = 0.0f;
};
