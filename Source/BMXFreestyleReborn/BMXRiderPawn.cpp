#include "BMXRiderPawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Math/RotationMatrix.h"

ABMXRiderPawn::ABMXRiderPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;

    Collider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BMXCollision"));
    // v0.1.8: A small, non-simulated central body checks walls and obstacles.
    // Ground support is resolved by TWO independent wheel contact traces;
    // a full-height humanoid capsule must not govern bike ramp motion.
    Collider->InitCapsuleSize(18, 20);
    Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collider->SetCollisionObjectType(ECC_Pawn);
    Collider->SetCollisionResponseToAllChannels(ECR_Block);
    RootComponent = Collider;

    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(Collider);
    VisualRoot->SetRelativeLocation(FVector(0, 0, -55));
    TrickPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TrickPivot"));
    TrickPivot->SetupAttachment(VisualRoot);
    TrickPivot->SetRelativeLocation(FVector(0,0,55));
    BikeBodyPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BikeBody"));
    BikeBodyPivot->SetupAttachment(TrickPivot);
    HandlePivot = CreateDefaultSubobject<USceneComponent>(TEXT("HandlebarPivot"));
    HandlePivot->SetupAttachment(BikeBodyPivot);
    HandlePivot->SetRelativeLocation(FVector(58,0,49));
    RearWheelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RearWheelRotation"));
    RearWheelPivot->SetupAttachment(BikeBodyPivot);
    RearWheelPivot->SetRelativeLocation(FVector(-63,0,-20));
    FrontWheelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FrontWheelRotation"));
    FrontWheelPivot->SetupAttachment(BikeBodyPivot);
    FrontWheelPivot->SetRelativeLocation(FVector(63,0,-20));
    LeftLegPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftLegAnimation"));
    LeftLegPivot->SetupAttachment(TrickPivot);
    LeftLegPivot->SetRelativeLocation(FVector(-23,-14,71));
    RightLegPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RightLegAnimation"));
    RightLegPivot->SetupAttachment(TrickPivot);
    RightLegPivot->SetRelativeLocation(FVector(-23,14,71));

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(Collider);
    CameraBoom->TargetArmLength = 405;
    CameraBoom->SetRelativeLocation(FVector(0,0,108));
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeSize = 16;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12;
    CameraBoom->SetUsingAbsoluteRotation(true);
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeRef(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereRef(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderRef(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UStaticMesh* Cube = CubeRef.Succeeded() ? CubeRef.Object : nullptr;
    UStaticMesh* Sphere = SphereRef.Succeeded() ? SphereRef.Object : nullptr;
    UStaticMesh* Cylinder = CylinderRef.Succeeded() ? CylinderRef.Object : nullptr;

    // First-pass BMX silhouette: actual hollow wheels, spoke pattern and triangular frame.
    // All geometry uses built-in Engine primitives; no marketplace assets are required.
    MakeWheel(TEXT("Rear"), RearWheelPivot, Cube, Cylinder);
    MakeWheel(TEXT("Front"), FrontWheelPivot, Cube, Cylinder);

    MakeRod(TEXT("Chainstay"), BikeBodyPivot, Cube, FVector(-63,0,-20), FVector(-8,0,6), 5.0f);
    MakeRod(TEXT("Seatstay"), BikeBodyPivot, Cube, FVector(-63,0,-20), FVector(-25,0,43), 5.0f);
    MakeRod(TEXT("DownTube"), BikeBodyPivot, Cube, FVector(-8,0,6), FVector(49,0,40), 6.0f);
    MakeRod(TEXT("TopTube"), BikeBodyPivot, Cube, FVector(-25,0,42), FVector(49,0,40), 5.0f);
    MakeRod(TEXT("SeatTube"), BikeBodyPivot, Cube, FVector(-8,0,6), FVector(-25,0,42), 5.0f);
    MakeRod(TEXT("HeadTube"), BikeBodyPivot, Cube, FVector(48,0,35), FVector(52,0,51), 8.0f);
    MakeRod(TEXT("FrontFork"), BikeBodyPivot, Cube, FVector(63,0,-20), FVector(52,0,49), 5.0f);
    MakeRod(TEXT("SeatPost"), BikeBodyPivot, Cube, FVector(-25,0,39), FVector(-27,0,50), 4.5f);
    MakePart(TEXT("Saddle"), BikeBodyPivot, Cube, FVector(-30,0,50), FVector(0.30f,0.17f,0.05f));
    MakeRod(TEXT("HandleStem"), HandlePivot, Cube, FVector(0,0,-5), FVector(0,0,17), 4.5f);
    MakeRod(TEXT("HandleCrossbar"), HandlePivot, Cube, FVector(0,-35,17), FVector(0,35,17), 4.0f);
    MakeRod(TEXT("LeftGrip"), HandlePivot, Cube, FVector(0,-35,17), FVector(0,-44,17), 5.0f);
    MakeRod(TEXT("RightGrip"), HandlePivot, Cube, FVector(0,35,17), FVector(0,44,17), 5.0f);
    MakePart(TEXT("Crank"), BikeBodyPivot, Cylinder, FVector(-8,0,6), FVector(0.13f,0.13f,0.15f), FRotator(0,0,90));

    // Jointed placeholder rider: simple leg motion is visibly linked to pedaling.
    MakeRod(TEXT("RiderTorso"), TrickPivot, Cube, FVector(-23,0,75), FVector(10,0,115), 20.0f);
    MakePart(TEXT("RiderHead"), TrickPivot, Sphere, FVector(18,0,135), FVector(0.22f,0.22f,0.24f));
    MakePart(TEXT("RiderHelmet"), TrickPivot, Sphere, FVector(18,0,145), FVector(0.26f,0.26f,0.11f));
    MakeRod(TEXT("UpperArmLeft"), TrickPivot, Cube, FVector(6,-13,113), FVector(30,-21,89), 8.0f);
    MakeRod(TEXT("LowerArmLeft"), TrickPivot, Cube, FVector(30,-21,89), FVector(58,-30,65), 7.0f);
    MakeRod(TEXT("UpperArmRight"), TrickPivot, Cube, FVector(6,13,113), FVector(30,21,89), 8.0f);
    MakeRod(TEXT("LowerArmRight"), TrickPivot, Cube, FVector(30,21,89), FVector(58,30,65), 7.0f);
    MakeRod(TEXT("LeftThigh"), LeftLegPivot, Cube, FVector::ZeroVector, FVector(26,0,-22), 10.5f);
    MakeRod(TEXT("LeftShin"), LeftLegPivot, Cube, FVector(26,0,-22), FVector(28,0,-64), 9.0f);
    MakePart(TEXT("LeftShoe"), LeftLegPivot, Cube, FVector(32,0,-65), FVector(0.20f,0.12f,0.075f));
    MakeRod(TEXT("RightThigh"), RightLegPivot, Cube, FVector::ZeroVector, FVector(26,0,-22), 10.5f);
    MakeRod(TEXT("RightShin"), RightLegPivot, Cube, FVector(26,0,-22), FVector(28,0,-64), 9.0f);
    MakePart(TEXT("RightShoe"), RightLegPivot, Cube, FVector(32,0,-65), FVector(0.20f,0.12f,0.075f));
}

UStaticMeshComponent* ABMXRiderPawn::MakePart(FName Name, USceneComponent* Parent,
    UStaticMesh* Mesh, const FVector& Offset, const FVector& Scale, const FRotator& Rotation)
{
    UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
    Part->SetupAttachment(Parent);
    if (Mesh) Part->SetStaticMesh(Mesh);
    Part->SetRelativeLocation(Offset);
    Part->SetRelativeRotation(Rotation);
    Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    return Part;
}

// A rod is a Cube aligned between two 3D endpoints; unlike the v0.1.2 boxes,
// it keeps connected bicycle tubes and limbs visually coherent as they move.
void ABMXRiderPawn::MakeRod(FName Name, USceneComponent* Parent, UStaticMesh* Cube,
                            const FVector& Start, const FVector& End, float Thickness)
{
    const FVector Direction = End - Start;
    if (Direction.IsNearlyZero()) return;
    MakePart(Name, Parent, Cube, (Start + End) * 0.5f,
             FVector(Direction.Size() / 100.0f, Thickness / 100.0f, Thickness / 100.0f),
             FRotationMatrix::MakeFromX(Direction).Rotator());
}

void ABMXRiderPawn::MakeWheel(const FString& Prefix, USceneComponent* Parent,
                              UStaticMesh* Cube, UStaticMesh* Cylinder)
{
    constexpr int32 Segments = 18;
    constexpr float Radius = 34.0f;
    for (int32 Side = 0; Side < 2; ++Side)
    {
        const float Y = Side == 0 ? -3.5f : 3.5f;
        for (int32 I = 0; I < Segments; ++I)
        {
            const float A = 2.0f * PI * float(I) / float(Segments);
            const float B = 2.0f * PI * float(I + 1) / float(Segments);
            const FVector Start(Radius * FMath::Cos(A), Y, Radius * FMath::Sin(A));
            const FVector End(Radius * FMath::Cos(B), Y, Radius * FMath::Sin(B));
            MakeRod(FName(*FString::Printf(TEXT("%sRim%d_%02d"), *Prefix, Side, I)),
                    Parent, Cube, Start, End, 5.0f);
        }
        for (int32 I = 0; I < 8; ++I)
        {
            const float A = 2.0f * PI * float(I) / 8.0f;
            MakeRod(FName(*FString::Printf(TEXT("%sSpoke%d_%02d"), *Prefix, Side, I)),
                    Parent, Cube, FVector(0,Y,0),
                    FVector((Radius-2.0f)*FMath::Cos(A),Y,(Radius-2.0f)*FMath::Sin(A)), 1.3f);
        }
    }
    MakePart(FName(*(Prefix + TEXT("Axle"))), Parent, Cylinder, FVector::ZeroVector,
             FVector(0.09f,0.09f,0.16f), FRotator(0,0,90));
}

void ABMXRiderPawn::UpdateVisualMotion(float Dt)
{
    // As the bike translates, the hollow wheel mesh rotates at roughly tire-contact speed.
    const float SignedTravelSpeed = ActualSpeedCmPerSec * (SpeedCmPerSec < 0.0f ? -1.0f : 1.0f);
    WheelRotationDeg = FMath::UnwindDegrees(WheelRotationDeg +
        FMath::RadiansToDegrees((SignedTravelSpeed * Dt) / 34.0f));
    RearWheelPivot->SetRelativeRotation(FRotator(WheelRotationDeg,0,0));
    FrontWheelPivot->SetRelativeRotation(FRotator(WheelRotationDeg,0,0));
    const float NormalizedSpeed = FMath::Clamp(FMath::Abs(SpeedCmPerSec) / 600.0f, 0.0f, 1.0f);
    const float PedalWeight = NormalizedSpeed * (ThrottleInput > 0.05f ? 1.0f : 0.12f);
    PedalPhase = FMath::Fmod(PedalPhase + Dt * FMath::Abs(SpeedCmPerSec) / 65.0f, 2.0f * PI);
    const float PedalSwing = 20.0f * PedalWeight;
    LeftLegPivot->SetRelativeRotation(FRotator(FMath::Sin(PedalPhase) * PedalSwing,0,0));
    RightLegPivot->SetRelativeRotation(FRotator(-FMath::Sin(PedalPhase) * PedalSwing,0,0));
    if (!bHasActiveTrick)
    {
        const float Lean = -SteeringInput * FMath::Clamp(NormalizedSpeed,0.0f,1.0f) * 8.0f;
        TrickPivot->SetRelativeRotation(FRotator(FloorPitchDeg,0,Lean));
    }
    if (!bHasActiveTrick || ActiveTrick.Type != EBMXTrickType::Barspin)
    {
        HandlePivot->SetRelativeRotation(FRotator(0,SteeringInput * 22.0f,0));
    }
    // Vert air: the whole rider + bike visibly turns 180 degrees while the
    // actor keeps its heading; the heading itself flips on landing.
    VisualRoot->SetRelativeRotation(FRotator(0.0f, VertAirYawDeg, 0.0f));
}

void ABMXRiderPawn::BeginPlay()
{
    Super::BeginPlay();
    ResetBike();
    ShowStatus(TEXT("TRAINING PARK  |  HOLD W TO PEDAL, SPACE TO HOP"), 5.0f);
}

void ABMXRiderPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    if (APlayerController* PC = Cast<APlayerController>(NewController))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void ABMXRiderPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    check(Input);
    Input->BindAxis(TEXT("Throttle"), this, &ABMXRiderPawn::SetThrottle);
    Input->BindAxis(TEXT("Steer"), this, &ABMXRiderPawn::SetSteering);
    Input->BindAxis(TEXT("CameraYaw"), this, &ABMXRiderPawn::OrbitYaw);
    Input->BindAxis(TEXT("CameraPitch"), this, &ABMXRiderPawn::OrbitPitch);
    Input->BindAction(TEXT("Hop"), IE_Pressed, this, &ABMXRiderPawn::Hop);
    Input->BindAction(TEXT("Barspin"), IE_Pressed, this, &ABMXRiderPawn::Barspin);
    Input->BindAction(TEXT("Tailwhip"), IE_Pressed, this, &ABMXRiderPawn::Tailwhip);
    Input->BindAction(TEXT("SpinLeft"), IE_Pressed, this, &ABMXRiderPawn::SpinLeft);
    Input->BindAction(TEXT("SpinRight"), IE_Pressed, this, &ABMXRiderPawn::SpinRight);
    Input->BindAction(TEXT("Backflip"), IE_Pressed, this, &ABMXRiderPawn::Backflip);
    Input->BindAction(TEXT("Frontflip"), IE_Pressed, this, &ABMXRiderPawn::Frontflip);
    Input->BindAction(TEXT("Reset"), IE_Pressed, this, &ABMXRiderPawn::ResetBike);
    Input->BindAction(TEXT("RecenterCamera"), IE_Pressed, this, &ABMXRiderPawn::RecenterCamera);
    Input->BindAction(TEXT("ToggleDebug"), IE_Pressed, this, &ABMXRiderPawn::ToggleDebugHUD);
}

void ABMXRiderPawn::ToggleDebugHUD() { bShowDebugHUD = !bShowDebugHUD; }

void ABMXRiderPawn::SetThrottle(float Value) { ThrottleInput = FMath::Clamp(Value,-1.0f,1.0f); }
void ABMXRiderPawn::SetSteering(float Value) { SteeringInput = FMath::Clamp(Value,-1.0f,1.0f); }
void ABMXRiderPawn::OrbitYaw(float Value)
{
    if (FMath::Abs(Value) > 0.01f)
    {
        CameraYawDeg = FMath::UnwindDegrees(CameraYawDeg + Value * 0.85f);
        CameraIdleSeconds = 0.0f;
    }
}
void ABMXRiderPawn::OrbitPitch(float Value)
{
    if (FMath::Abs(Value) > 0.01f)
    {
        CameraPitchDeg = FMath::Clamp(CameraPitchDeg + Value * 0.5f,-50.0f,12.0f);
        CameraIdleSeconds = 0.0f;
    }
}
void ABMXRiderPawn::RecenterCamera()
{
    CameraYawDeg = GetActorRotation().Yaw;
    CameraPitchDeg = -12.0f;
    CameraIdleSeconds = 0.0f;
}

// The visual wheels are 34 cm radius and the lowest point of each tire is
// approximately 54 cm below the Pawn origin (centres are at Z=-20). Traces
// start near that expected tire contact, never far above the rider's head.
bool ABMXRiderPawn::ProbeWheel(const FVector& PawnCenter, float ForwardOffset,
                               FBMXWheelContact& Contact, float ProbeHeight) const
{
    Contact = FBMXWheelContact();
    const UWorld* World = GetWorld();
    if (!World) return false;

    constexpr float TireBottomBelowPawn = 54.0f;
    const FVector WheelWorld = PawnCenter + GetActorForwardVector() * ForwardOffset;
    const FVector Start(WheelWorld.X, WheelWorld.Y,
                        PawnCenter.Z - TireBottomBelowPawn + ProbeHeight);
    const FVector End(WheelWorld.X, WheelWorld.Y,
                      PawnCenter.Z - TireBottomBelowPawn - 180.0f);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(BMXWheelContact), true);
    Params.AddIgnoredActor(this);
    FHitResult Hit;
    if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)) return false;

    // Sidewalls and underside faces do not count as riding surfaces.
    // 0.35 (~69 degrees) allows the ~62 degree tops of the v0.2 quarter-pipes.
    if (Hit.ImpactNormal.Z < 0.35f || Hit.bStartPenetrating) return false;
    Contact.bSupported = true;
    Contact.FloorZ = Hit.ImpactPoint.Z;
    Contact.Normal = Hit.ImpactNormal;
    return true;
}

FBMXRidingSupport ABMXRiderPawn::ProbeSupport(const FVector& Center,
                                                           float ProbeHeight) const
{
    FBMXRidingSupport Support;
    ProbeWheel(Center, 63.0f, Support.Front, ProbeHeight);
    ProbeWheel(Center, -63.0f, Support.Rear, ProbeHeight);
    return Support;
}

bool ABMXRiderPawn::IsSupportReachable(const FVector& Origin,
                                        const FBMXRidingSupport& Support) const
{
    if (!Support.HasContact()) return false;
    constexpr float TireBottomBelowPawn = 54.0f;
    constexpr float GroundClearance = 3.0f;
    const float TargetCenterZ = Support.AverageFloorZ() + TireBottomBelowPawn + GroundClearance;
    const float Rise = TargetCenterZ - Origin.Z;

    // Do not magnetically snap the bike up onto an elevated tabletop or ledge.
    // A ramp may rise farther in a single fast substep, but a square step may not.
    if (Rise > (Support.HasRampContact() ? 75.0f : 28.0f) || Rise < -105.0f)
        return false;
    if (Support.Front.bSupported && Support.Rear.bSupported)
    {
        const float HeightDifference = FMath::Abs(Support.Front.FloorZ - Support.Rear.FloorZ);
        if (!Support.HasRampContact() && HeightDifference > 29.0f)
            return false; // two flat surfaces at different heights = a ledge, not a slope
        if (HeightDifference > 215.0f)
            return false; // physically impossible wheel span for a rideable approach
    }
    return true;
}

void ABMXRiderPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float TotalDt = FMath::Clamp(DeltaSeconds, 0.0f, 0.075f);
    if (StatusSeconds > 0.0f)
    {
        StatusSeconds -= TotalDt;
        if (StatusSeconds <= 0.0f) StatusMessage.Empty();
    }
    if (bBailed)
    {
        BailCooldown -= TotalDt;
        if (BailCooldown <= 0.0f) ResetBike();
        UpdateCamera(TotalDt);
        return;
    }

    // Fast movement is subdivided; a 45 cm single-frame hop across a ramp lip
    // was one cause of the old trace and swept-capsule disagreement.
    const int32 NumSteps = FMath::Max(1, FMath::CeilToInt(TotalDt / (1.0f / 90.0f)));
    const float Dt = TotalDt / float(NumSteps);
    const FVector Before = GetActorLocation();
    bool bBlockedThisFrame = false;
    FString FrameBlocker;
    for (int32 Step = 0; Step < NumSteps; ++Step)
    {
        MoveBike(Dt);
        if (bMovementBlocked)
        {
            bBlockedThisFrame = true;
            FrameBlocker = LastBlockerName;
        }
        if (bBailed) break;
    }
    bMovementBlocked = bBlockedThisFrame;
    LastBlockerName = FrameBlocker;
    // Full 3D travel, so climbing a ramp or falling shows real speed.
    const float MeasuredSpeed = FVector::Dist(Before, GetActorLocation()) /
        FMath::Max(0.001f, TotalDt);
    ActualSpeedCmPerSec = FMath::FInterpTo(ActualSpeedCmPerSec, MeasuredSpeed, TotalDt, 8.0f);
    if (!bBailed)
    {
        UpdateTricks(TotalDt);
        UpdateVisualMotion(TotalDt);
    }
    UpdateCamera(TotalDt);
}

void ABMXRiderPawn::MoveBike(float Dt)
{
    constexpr float Gravity = 980.0f;
    constexpr float TireBottomBelowPawn = 54.0f;
    constexpr float GroundClearance = 3.0f;
    constexpr float MaxFallTime = 12.0f;

    bMovementBlocked = false;
    LastBlockerName.Empty();
    JumpGrace = FMath::Max(0.0f, JumpGrace - Dt);
    LandingCooldown = FMath::Max(0.0f, LandingCooldown - Dt);

    auto LabelHit = [](const FHitResult& Hit) -> FString
    {
        const UPrimitiveComponent* Component = Hit.GetComponent();
        if (!Component) return TEXT("UNKNOWN");
        for (const FName& Tag : Component->ComponentTags)
        {
            const FString Name = Tag.ToString();
            if (Name.StartsWith(TEXT("BMX_"))) return Name.RightChop(4);
        }
        return Component->GetName();
    };

    // v0.2.0: On the ground SpeedCmPerSec is speed ALONG the riding surface.
    // In the air it is horizontal speed only and pedaling has no effect.
    if (bGrounded) UpdateGroundSpeed(Dt);

    // Vert airs are committed: the auto-180 handles direction instead.
    if (!bVertAir)
    {
        const float TurningStrength = FMath::Clamp(FMath::Abs(SpeedCmPerSec) / 450.0f, 0.18f, 1.0f);
        const float DirectionSign = SpeedCmPerSec >= 0.0f ? 1.0f : -1.0f;
        AddActorWorldRotation(FRotator(0.0f,
            SteeringInput * SteeringRateDeg * Dt * TurningStrength * DirectionSign *
            (bGrounded ? 1.0f : 0.35f), 0.0f));
    }

    const FVector Start = GetActorLocation();
    // On a slope only the cos(pitch) share of surface speed is horizontal;
    // the ground snap supplies the matching rise. Previously the full speed
    // was applied horizontally, so climbing a ramp made the bike FASTER.
    const float HorizontalScale = bGrounded ?
        FMath::Max(0.2f, FMath::Cos(FMath::DegreesToRadians(FloorPitchDeg))) : 1.0f;
    const FVector RequestedHorizontal = GetActorForwardVector() * SpeedCmPerSec * HorizontalScale * Dt;

    // Check the wheel surface in advance. A quarter-pipe lip is NOT a solid
    // wall even when a sweep touches its vertical closure triangle.
    const FBMXRidingSupport ProposedSupport = ProbeSupport(Start + RequestedHorizontal);
    const bool bRideableRampApproach = bGrounded &&
        IsSupportReachable(Start, ProposedSupport) && ProposedSupport.HasRampContact();

    // Two small forward obstacle sweeps protect the tires AND the bike frame.
    // The ground itself is handled only by independent front/rear wheel rays.
    // Unlike the old human-height capsule, these probes do not prevent riding
    // up the surface of a quarter-pipe.
    float AllowedFraction = 1.0f;
    if (RequestedHorizontal.SizeSquared2D() > 0.01f && GetWorld())
    {
        FCollisionQueryParams Params(SCENE_QUERY_STAT(BMXFrontObstacle), false);
        Params.AddIgnoredActor(this);
        const FVector TravelDirection = RequestedHorizontal.GetSafeNormal();
        for (int32 Probe = 0; Probe < 2; ++Probe)
        {
            const float RelativeZ = Probe == 0 ? -20.0f : 34.0f;
            const FVector Nose = Start + TravelDirection * 69.0f +
                                 FVector(0.0f, 0.0f, RelativeZ);
            FHitResult NoseHit;
            if (GetWorld()->SweepSingleByChannel(NoseHit, Nose,
                Nose + RequestedHorizontal, FQuat::Identity, ECC_Visibility,
                FCollisionShape::MakeSphere(15.0f), Params) &&
                NoseHit.ImpactNormal.Z < 0.38f &&
                !(bRideableRampApproach &&
                  (LabelHit(NoseHit) == TEXT("RAMP") || LabelHit(NoseHit) == TEXT("BOX"))))
            {
                const float Fraction = FMath::Clamp(NoseHit.Time - 0.035f, 0.0f, 1.0f);
                if (Fraction < AllowedFraction)
                {
                    AllowedFraction = Fraction;
                    bMovementBlocked = true;
                    LastBlockerName = LabelHit(NoseHit);
                }
            }
        }
    }
    const FVector HorizontalStep = RequestedHorizontal * AllowedFraction;
    bool bCompletedGroundMotion = false;
    bool bAttemptedGroundHorizontal = false;

    if (bGrounded)
    {
        // This is the critical v0.1.8 change: use both wheel contacts at the
        // *next* position, rather than a single ray through the Pawn's centre.
        const FBMXRidingSupport NextSupport = ProbeSupport(Start + HorizontalStep);
        // v0.1.9: A quarter-pipe has an OPEN lip, NOT a level deck.
        // In v0.1.8 the old launch condition almost never fired here:
        // the front wheel disappeared beyond the lip while the rear wheel
        // continued touching the ramp. A single rear contact then kept the
        // pawn artificially grounded and erased the stored ramp pitch.
        //
        // Launch on that front-wheel loss (or a sudden front-wheel drop)
        // while the rear is still on a rising ramp. Require forward speed
        // and positive stored pitch so rolling off a flat ledge won't
        // trigger an artificial high jump.
        const bool bRearOnRisingRamp =
            NextSupport.Rear.bSupported &&
            NextSupport.Rear.Normal.Z < 0.96f;
        const bool bFrontPastOpenLip =
            !NextSupport.Front.bSupported ||
            (NextSupport.Front.FloorZ + 85.0f < NextSupport.Rear.FloorZ);
        const bool bOpenQuarterPipeTakeoff =
            bRearOnRisingRamp && bFrontPastOpenLip;
        const bool bDeckTransitionTakeoff =
            NextSupport.Front.bSupported && NextSupport.Rear.bSupported &&
            NextSupport.Front.Normal.Z > 0.97f &&
            NextSupport.Rear.Normal.Z < 0.83f &&
            NextSupport.Front.FloorZ > NextSupport.Rear.FloorZ + 24.0f;
        // v0.2.0: An open quarter-pipe lip launches at any rolling speed
        // (slow = small vert air). Rolling up onto a tabletop deck still
        // needs real speed so you can ride onto the deck slowly.
        const bool bLaunchAtLip =
            bGrounded && FloorPitchDeg > 8.0f &&
            ((bOpenQuarterPipeTakeoff && SpeedCmPerSec > 150.0f) ||
             (bDeckTransitionTakeoff && SpeedCmPerSec > 525.0f));
        if (bLaunchAtLip)
        {
            LaunchIntoAir(0.0f, 180.0f);
            JumpGrace = 0.13f;
        }
        else if (IsSupportReachable(Start, NextSupport))
        {
            const float DesiredCenterZ = NextSupport.AverageFloorZ() +
                TireBottomBelowPawn + GroundClearance;
            const FVector Intended = Start + HorizontalStep;

            // With a small root collision body, a vertical lift/settle no longer
            // drives a person-sized capsule into the crest of the ramp.
            const float LiftZ = FMath::Max(Start.Z, DesiredCenterZ) + 3.0f;
            FHitResult LiftHit;
            SetActorLocation(FVector(Start.X, Start.Y, LiftZ), true, &LiftHit);
            FHitResult HorizontalHit;
            bAttemptedGroundHorizontal = true;
            SetActorLocation(FVector(Intended.X, Intended.Y, GetActorLocation().Z),
                             true, &HorizontalHit);
            if (HorizontalHit.bBlockingHit)
            {
                bMovementBlocked = true;
                LastBlockerName = LabelHit(HorizontalHit);
                const FVector Remaining = HorizontalStep * (1.0f - HorizontalHit.Time);
                const FVector Slide = FVector::VectorPlaneProject(Remaining, HorizontalHit.ImpactNormal);
                if (Slide.SizeSquared2D() > 0.01f)
                {
                    FHitResult SlideHit;
                    SetActorLocation(GetActorLocation() + FVector(Slide.X, Slide.Y, 0.0f),
                                     true, &SlideHit);
                }
            }
            FHitResult SettleHit;
            const FVector Current = GetActorLocation();
            SetActorLocation(FVector(Current.X, Current.Y, DesiredCenterZ),
                             true, &SettleHit);
            const FBMXRidingSupport Actual = ProbeSupport(GetActorLocation(), 70.0f);
            const bool bOnSurface = Actual.HasContact() &&
                FMath::Abs(GetActorLocation().Z -
                (Actual.AverageFloorZ() + TireBottomBelowPawn + GroundClearance)) < 24.0f;
            if (bOnSurface)
            {
                bGrounded = true;
                bCompletedGroundMotion = true;
                VerticalSpeed = 0.0f;
                AirTime = 0.0f;
                StuckAirSeconds = 0.0f;
                LeftSurfaceSeconds = 0.0f;
                bFrontWheelContact = Actual.Front.bSupported;
                bRearWheelContact = Actual.Rear.bSupported;
                FrontWheelFloorZ = Actual.Front.FloorZ;
                RearWheelFloorZ = Actual.Rear.FloorZ;
                if (Actual.Front.bSupported && Actual.Rear.bSupported)
                {
                    FloorPitchDeg = FMath::FInterpTo(FloorPitchDeg,
                        FMath::Clamp(Actual.SurfacePitchDegrees(), -62.0f, 62.0f), Dt, 11.0f);
                }
                else
                {
                    // When only the rear tire is still supported by a ramp,
                    // keep the last reliable upward pitch until takeoff.
                    // v0.1.8 flattened it to zero immediately, killing the
                    // upward velocity at the open lip.
                    const bool bRearOnlyRampContact =
                        Actual.Rear.bSupported &&
                        !Actual.Front.bSupported &&
                        Actual.Rear.Normal.Z < 0.96f &&
                        FloorPitchDeg > 0.0f;
                    if (!bRearOnlyRampContact)
                    {
                        FloorPitchDeg = FMath::FInterpTo(FloorPitchDeg, 0.0f, Dt, 3.0f);
                    }
                }
            }
        }

        if (!bCompletedGroundMotion && bMovementBlocked && !bLaunchAtLip)
        {
            // A wall is not a ramp lip. Stay on the current surface rather
            // than repeatedly toggling AIR/GROUND while W is held into it.
            const FBMXRidingSupport Here = ProbeSupport(GetActorLocation(), 70.0f);
            if (IsSupportReachable(GetActorLocation(), Here))
            {
                bGrounded = true;
                bFrontWheelContact = Here.Front.bSupported;
                bRearWheelContact = Here.Rear.bSupported;
                VerticalSpeed = 0.0f;
                AirTime = 0.0f;
                SpeedCmPerSec *= 0.55f;
                return;
            }
        }
        if (!bCompletedGroundMotion && !bLaunchAtLip)
        {
            // Leave the surface as an actual airborne transition. Carry
            // upward momentum from the last riding pitch for arcade-style lips.
            LeftSurfaceSeconds = 0.0f;
            LaunchIntoAir(0.0f, 0.0f);
        }
    }

    if (bCompletedGroundMotion)
    {
        if (bMovementBlocked) SpeedCmPerSec *= 0.65f;
        return;
    }

    // AIR: move horizontally and vertically in separate swept operations so
    // a side collision cannot freeze gravity (the old midair lock).
    AirTime += Dt;
    if (bVertAir)
    {
        VertAirElapsed += Dt;
        const float TurnProgress = FMath::Clamp(VertAirElapsed / FMath::Max(0.1f, VertAirTurnSeconds), 0.0f, 1.0f);
        VertAirYawDeg = 180.0f * FMath::InterpEaseInOut(0.0f, 1.0f, TurnProgress, 2.0f);
        // Nose up at takeoff, level at the apex, nose down the ramp at landing.
        FloorPitchDeg = VertAirLaunchPitchDeg * FMath::Cos(PI * TurnProgress);
    }
    else
    {
        // Ordinary jumps: the bike levels out gradually in the air.
        FloorPitchDeg = FMath::FInterpTo(FloorPitchDeg, 0.0f, Dt, 1.5f);
    }
    const FVector BeforeHorizontal = GetActorLocation();
    FHitResult HorizontalHit;
    const FVector AirHorizontalStep = bAttemptedGroundHorizontal ? FVector::ZeroVector : HorizontalStep;
    SetActorLocation(BeforeHorizontal + AirHorizontalStep, true, &HorizontalHit);
    if (HorizontalHit.bBlockingHit)
    {
        bMovementBlocked = true;
        LastBlockerName = LabelHit(HorizontalHit);
        const FVector Slide = FVector::VectorPlaneProject(
            AirHorizontalStep * (1.0f - HorizontalHit.Time), HorizontalHit.ImpactNormal);
        if (Slide.SizeSquared2D() > 1.0f)
        {
            FHitResult SlideHit;
            SetActorLocation(GetActorLocation() + FVector(Slide.X, Slide.Y, 0.0f),
                             true, &SlideHit);
        }
        SpeedCmPerSec *= 0.78f;
    }
    VerticalSpeed = FMath::Max(-2800.0f, VerticalSpeed - Gravity * Dt);
    const float BeforeZ = GetActorLocation().Z;
    FHitResult VerticalHit;
    SetActorLocation(GetActorLocation() + FVector(0.0f, 0.0f, VerticalSpeed * Dt),
                     true, &VerticalHit);
    if (VerticalHit.bBlockingHit && VerticalSpeed > 0.0f &&
        VerticalHit.ImpactNormal.Z < -0.45f)
    {
        VerticalSpeed = 0.0f; // ceiling
    }

    const FVector EndPosition = GetActorLocation();
    const FBMXRidingSupport LandingSupport = ProbeSupport(EndPosition, 75.0f);
    bFrontWheelContact = LandingSupport.Front.bSupported &&
        FMath::Abs(EndPosition.Z - (LandingSupport.Front.FloorZ + TireBottomBelowPawn + GroundClearance)) < 22.0f;
    bRearWheelContact = LandingSupport.Rear.bSupported &&
        FMath::Abs(EndPosition.Z - (LandingSupport.Rear.FloorZ + TireBottomBelowPawn + GroundClearance)) < 22.0f;
    const float LandingZ = LandingSupport.AverageFloorZ() + TireBottomBelowPawn + GroundClearance;
    const bool bCloseEnoughToLand = LandingSupport.HasContact() &&
        EndPosition.Z <= LandingZ + 9.0f && EndPosition.Z >= LandingZ - 18.0f;
    if (JumpGrace <= 0.0f && LandingCooldown <= 0.0f && VerticalSpeed <= 0.0f &&
        bCloseEnoughToLand)
    {
        FHitResult LandHit;
        SetActorLocation(FVector(EndPosition.X, EndPosition.Y, LandingZ), true, &LandHit);
        float LandedPitch = LandingSupport.SurfacePitchDegrees();
        if (bVertAir)
        {
            // Complete the 180: the actor now faces down the ramp. The wheel
            // probes are symmetric (+/-63 cm), so the measured surface is the
            // same; only its pitch sign and the travel direction flip.
            AddActorWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
            SpeedCmPerSec = -SpeedCmPerSec;
            LandedPitch = -LandedPitch;
            bVertAir = false;
            VertAirYawDeg = 0.0f;
            CameraIdleSeconds = 10.0f; // let the camera swing round behind the rider
        }
        // Keep momentum: project the air velocity onto the landing slope.
        // Coming down a ramp face turns the fall into forward speed, so a
        // vert air drops back in with pace instead of from a standstill.
        const float LandedPitchRad = FMath::DegreesToRadians(FMath::Clamp(LandedPitch, -62.0f, 62.0f));
        SpeedCmPerSec = FMath::Clamp(
            SpeedCmPerSec * FMath::Cos(LandedPitchRad) + VerticalSpeed * FMath::Sin(LandedPitchRad),
            -MaxForwardSpeed, MaxForwardSpeed * 1.5f);
        bGrounded = true;
        VerticalSpeed = 0.0f;
        AirTime = 0.0f;
        StuckAirSeconds = 0.0f;
        LeftSurfaceSeconds = 0.0f;
        FloorPitchDeg = FMath::Clamp(LandedPitch, -62.0f, 62.0f);
        LandingCooldown = 0.13f;
        if (bHasActiveTrick || !TrickQueue.IsEmpty())
            Bail(TEXT("INCOMPLETE TRICK - COMBO LOST"));
        else
            BankCombo();
        return;
    }

    // Recovery is a last resort, never the normal outcome of bumping a ramp.
    if (VerticalSpeed < -70.0f && FMath::Abs(GetActorLocation().Z - BeforeZ) < 0.12f)
        StuckAirSeconds += Dt;
    else
        StuckAirSeconds = 0.0f;
    if (StuckAirSeconds > 2.4f || AirTime > MaxFallTime)
    {
        if (LandingSupport.HasContact() &&
            FMath::Abs(EndPosition.Z - LandingZ) < 32.0f)
        {
            SetActorLocation(FVector(EndPosition.X, EndPosition.Y, LandingZ), false);
            if (bVertAir)
            {
                AddActorWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
                SpeedCmPerSec = -SpeedCmPerSec;
                bVertAir = false;
                VertAirYawDeg = 0.0f;
            }
            FloorPitchDeg = 0.0f;
            bGrounded = true;
            VerticalSpeed = 0.0f;
            StuckAirSeconds = 0.0f;
            AirTime = 0.0f;
            LandingCooldown = 0.13f;
            if (bHasActiveTrick || !TrickQueue.IsEmpty())
                Bail(TEXT("INCOMPLETE TRICK - COMBO LOST"));
            else
                BankCombo();
            return;
        }
        Bail(TEXT("PHYSICS RECOVERY - RESPAWNING"));
    }
    if (GetActorLocation().Z < -500.0f || GetActorLocation().Size2D() > 3650.0f)
        Bail(TEXT("OUT OF BOUNDS"));
}

void ABMXRiderPawn::UpdateGroundSpeed(float Dt)
{
    constexpr float Gravity = 980.0f;
    const float PitchRad = FMath::DegreesToRadians(FloorPitchDeg);
    if (ThrottleInput > 0.02f)
    {
        const float Target = MaxForwardSpeed * ThrottleInput;
        if (SpeedCmPerSec < Target)
        {
            // Pedaling uphill is weaker, so a quarter-pipe has to be hit with
            // speed carried from the flat rather than pedaled up.
            const float PedalScale = FloorPitchDeg > 0.0f ?
                FMath::Clamp(FMath::Square(FMath::Cos(PitchRad)), 0.25f, 1.0f) : 1.0f;
            SpeedCmPerSec = FMath::Min(Target, SpeedCmPerSec + Acceleration * PedalScale * Dt);
        }
        else
        {
            // Faster than pedaling allows (e.g. dropping in): only drag slows
            // you. Previously holding W while going downhill BRAKED the bike.
            SpeedCmPerSec = FMath::FInterpConstantTo(SpeedCmPerSec, Target, Dt, CoastDrag);
        }
    }
    else if (ThrottleInput < -0.02f)
    {
        SpeedCmPerSec = FMath::FInterpConstantTo(SpeedCmPerSec, ReverseSpeed * ThrottleInput, Dt, Acceleration);
    }
    else
    {
        SpeedCmPerSec = FMath::FInterpConstantTo(SpeedCmPerSec, 0.0f, Dt, CoastDrag);
    }

    // Gravity along the slope: uphill slows you, downhill speeds you up, and
    // a stalled bike on a steep face rolls back down (fakie). Gentle slopes
    // below the drag threshold hold the bike still.
    if (FMath::Abs(FloorPitchDeg) > 1.5f)
    {
        SpeedCmPerSec -= Gravity * SlopeGravityScale * FMath::Sin(PitchRad) * Dt;
    }
    SpeedCmPerSec = FMath::Clamp(SpeedCmPerSec, -MaxForwardSpeed, MaxForwardSpeed * 1.5f);
    if (FMath::Abs(SpeedCmPerSec) < 1.5f) SpeedCmPerSec = 0.0f;
}

void ABMXRiderPawn::LaunchIntoAir(float ExtraUpSpeed, float MinUpSpeed)
{
    constexpr float Gravity = 980.0f;
    const float PitchRad = FMath::DegreesToRadians(FloorPitchDeg);
    const float AlongSurface = SpeedCmPerSec;
    const bool bVert = FloorPitchDeg >= VertLaunchPitchDeg && AlongSurface > 0.0f;

    bGrounded = false;
    AirTime = 0.0f;
    bFrontWheelContact = false;
    bRearWheelContact = false;

    if (bVert)
    {
        // Straight up out of the quarter-pipe, then back in. Horizontal speed
        // is replaced by a slow drift back over the ramp (negative = toward
        // the ramp, since the actor still faces up the ramp).
        VerticalSpeed = FMath::Clamp(
            AlongSurface * FMath::Sin(PitchRad) * VertLaunchScale + ExtraUpSpeed * 0.5f,
            MinVertUpSpeed, MaxVertUpSpeed);
        const float PredictedAirTime = FMath::Max(0.4f, 2.0f * VerticalSpeed / Gravity);
        SpeedCmPerSec = -VertDriftCm / PredictedAirTime;
        bVertAir = true;
        VertAirElapsed = 0.0f;
        VertAirYawDeg = 0.0f;
        VertAirLaunchPitchDeg = FloorPitchDeg;
        // Finish turning well before touchdown so the landing is clean.
        VertAirTurnSeconds = FMath::Max(0.3f, PredictedAirTime * 0.6f);
        ShowStatus(TEXT("VERT AIR!"), 0.9f);
        return;
    }

    float Up = AlongSurface * FMath::Sin(PitchRad);
    if (Up > 0.0f) Up *= LaunchLiftScale;
    VerticalSpeed = FMath::Clamp(Up + ExtraUpSpeed, MinUpSpeed, 1200.0f);
    SpeedCmPerSec = AlongSurface * FMath::Cos(PitchRad);
    if (FloorPitchDeg > 8.0f && Up > 150.0f)
        ShowStatus(TEXT("RAMP LIP - AIRBORNE"), 0.75f);
}

void ABMXRiderPawn::Hop()
{
    if (bBailed || !bGrounded) return;
    // A hop on a slope keeps the slope's momentum (and can start a vert air
    // near the top of a quarter-pipe).
    LaunchIntoAir(HopSpeed + FMath::Clamp(FMath::Abs(SpeedCmPerSec)*0.055f,0.0f,65.0f),
                  HopSpeed * 0.6f);
    JumpGrace = 0.22f;
    if (!bVertAir) FloorPitchDeg *= 0.5f;
}

void ABMXRiderPawn::AddTrick(const TCHAR* Label, EBMXTrickType Type,
                              int32 Points, float Duration, float Rotation)
{
    if (bGrounded || bBailed) return;
    if (CompletedComboTricks.Num() + TrickQueue.Num() + (bHasActiveTrick ? 1 : 0) >= 6) return;
    FBMXTrick NewTrick;
    NewTrick.Name = Label;
    NewTrick.Type = Type;
    NewTrick.Points = Points;
    NewTrick.Duration = Duration;
    NewTrick.Rotation = Rotation;
    TrickQueue.Add(NewTrick);
    if (!bHasActiveTrick) BeginNextTrick();
}
void ABMXRiderPawn::Barspin() { AddTrick(TEXT("BARSPIN"), EBMXTrickType::Barspin, 150, 0.46f); }
void ABMXRiderPawn::Tailwhip() { AddTrick(TEXT("TAILWHIP"), EBMXTrickType::Tailwhip, 260, 0.64f); }
void ABMXRiderPawn::SpinLeft() { AddTrick(TEXT("360 LEFT"), EBMXTrickType::YawSpin, 210, 0.69f, -360.0f); }
void ABMXRiderPawn::SpinRight() { AddTrick(TEXT("360 RIGHT"), EBMXTrickType::YawSpin, 210, 0.69f, 360.0f); }
void ABMXRiderPawn::Backflip() { AddTrick(TEXT("BACKFLIP"), EBMXTrickType::Flip, 410, 0.83f, -360.0f); }
void ABMXRiderPawn::Frontflip() { AddTrick(TEXT("FRONTFLIP"), EBMXTrickType::Flip, 450, 0.83f, 360.0f); }

void ABMXRiderPawn::BeginNextTrick()
{
    bHasActiveTrick = false;
    if (TrickQueue.IsEmpty()) return;
    ActiveTrick = TrickQueue[0];
    TrickQueue.RemoveAt(0);
    bHasActiveTrick = true;
    ShowStatus(ActiveTrick.Name, 0.85f);
}

void ABMXRiderPawn::UpdateTricks(float Dt)
{
    if (!bHasActiveTrick)
    {
        ResetVisuals();
        return;
    }
    ActiveTrick.Elapsed = FMath::Min(ActiveTrick.Elapsed + Dt, ActiveTrick.Duration);
    const float Progress = FMath::Clamp(ActiveTrick.Elapsed / FMath::Max(ActiveTrick.Duration,0.001f),0.0f,1.0f);
    const float Angle = ActiveTrick.Rotation * FMath::InterpEaseInOut(0.0f,1.0f,Progress,1.4f);
    switch (ActiveTrick.Type)
    {
        case EBMXTrickType::Barspin:  HandlePivot->SetRelativeRotation(FRotator(0,Angle,0)); break;
        case EBMXTrickType::Tailwhip: BikeBodyPivot->SetRelativeRotation(FRotator(0,Angle,0)); break;
        case EBMXTrickType::YawSpin:  TrickPivot->SetRelativeRotation(FRotator(0,Angle,0)); break;
        case EBMXTrickType::Flip:     TrickPivot->SetRelativeRotation(FRotator(FloorPitchDeg+Angle,0,0)); break;
        default: break;
    }
    if (Progress >= 1.0f)
    {
        // Repeating a trick inside one combo is worth less each time
        // (100%, 50%, 25%...), rewarding variety like classic BMX games.
        int32 Repeats = 0;
        for (const FString& Done : CompletedComboTricks)
        {
            if (Done == ActiveTrick.Name) ++Repeats;
        }
        UnbankedPoints += FMath::Max(10, ActiveTrick.Points >> FMath::Min(Repeats, 4));
        CompletedComboTricks.Add(ActiveTrick.Name);
        ResetVisuals();
        BeginNextTrick();
    }
}

void ABMXRiderPawn::ResetVisuals()
{
    TrickPivot->SetRelativeRotation(FRotator(FloorPitchDeg,0,0));
    BikeBodyPivot->SetRelativeRotation(FRotator::ZeroRotator);
    HandlePivot->SetRelativeRotation(FRotator::ZeroRotator);
}

float ABMXRiderPawn::GetComboMultiplier() const
{
    return 1.0f + FMath::Max(0,CompletedComboTricks.Num()-1) * 0.35f;
}
int32 ABMXRiderPawn::GetComboPoints() const
{
    return FMath::RoundToInt(UnbankedPoints * GetComboMultiplier());
}
FString ABMXRiderPawn::GetComboNames() const
{
    return FString::Join(CompletedComboTricks, TEXT(" + "));
}
void ABMXRiderPawn::BankCombo()
{
    if (CompletedComboTricks.IsEmpty()) return;
    const int32 Award = GetComboPoints();
    TotalScore += Award;
    ShowStatus(FString::Printf(TEXT("LANDED! +%d POINTS"),Award), 2.3f);
    CompletedComboTricks.Reset();
    TrickQueue.Reset();
    UnbankedPoints = 0;
}
void ABMXRiderPawn::Bail(const FString& Reason)
{
    if (bBailed) return;
    bBailed = true;
    BailCooldown = 1.1f;
    SpeedCmPerSec = 0.0f;
    ActualSpeedCmPerSec = 0.0f;
    VerticalSpeed = 0.0f;
    bVertAir = false;
    VertAirYawDeg = 0.0f;
    bHasActiveTrick = false;
    TrickQueue.Reset();
    CompletedComboTricks.Reset();
    UnbankedPoints = 0;
    ResetVisuals();
    ShowStatus(Reason, 2.3f);
}
void ABMXRiderPawn::ResetBike()
{
    bBailed = false;
    bGrounded = true;
    BailCooldown = 0.0f;
    AirTime = 0.0f;
    JumpGrace = 0.0f;
    StuckAirSeconds = 0.0f;
    LeftSurfaceSeconds = 0.0f;
    bFrontWheelContact = false;
    bRearWheelContact = false;
    FrontWheelFloorZ = 0.0f;
    RearWheelFloorZ = 0.0f;
    LandingCooldown = 0.0f;
    bMovementBlocked = false;
    LastBlockerName.Empty();
    SpeedCmPerSec = 0.0f;
    ActualSpeedCmPerSec = 0.0f;
    VerticalSpeed = 0.0f;
    ThrottleInput = 0.0f;
    SteeringInput = 0.0f;
    FloorPitchDeg = 0.0f;
    bVertAir = false;
    VertAirElapsed = 0.0f;
    VertAirYawDeg = 0.0f;
    if (VisualRoot) VisualRoot->SetRelativeRotation(FRotator::ZeroRotator);
    WheelRotationDeg = 0.0f;
    PedalPhase = 0.0f;
    CompletedComboTricks.Reset();
    TrickQueue.Reset();
    bHasActiveTrick = false;
    UnbankedPoints = 0;
    SetActorLocationAndRotation(ResetLocation, ResetRotation, false, nullptr, ETeleportType::TeleportPhysics);
    ResetVisuals();
    RecenterCamera();
}
void ABMXRiderPawn::ShowStatus(const FString& Message, float Seconds)
{
    StatusMessage = Message;
    StatusSeconds = Seconds;
}
void ABMXRiderPawn::UpdateCamera(float Dt)
{
    CameraIdleSeconds += Dt;
    if (CameraIdleSeconds > 1.8f && FMath::Abs(SpeedCmPerSec) > 160.0f)
    {
        CameraYawDeg = FMath::FInterpTo(CameraYawDeg,GetActorRotation().Yaw,Dt,1.2f);
    }
    CameraBoom->SetWorldRotation(FRotator(CameraPitchDeg,CameraYawDeg,0));
}
