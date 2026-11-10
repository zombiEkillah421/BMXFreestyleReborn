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
    // v0.3.0 upper-body joints: hips (torso/head/arms) and both shoulders.
    RiderUpperPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RiderUpperBody"));
    RiderUpperPivot->SetupAttachment(TrickPivot);
    RiderUpperPivot->SetRelativeLocation(FVector(-23,0,75));
    LeftArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftArmAnimation"));
    LeftArmPivot->SetupAttachment(RiderUpperPivot);
    LeftArmPivot->SetRelativeLocation(FVector(29,-13,38));   // shoulder (6,-13,113) in TrickPivot space
    RightArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RightArmAnimation"));
    RightArmPivot->SetupAttachment(RiderUpperPivot);
    RightArmPivot->SetRelativeLocation(FVector(29,13,38));

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
    // Same shapes and places as before, now relative to the new joints.
    MakeRod(TEXT("RiderTorso"), RiderUpperPivot, Cube, FVector(0,0,0), FVector(33,0,40), 20.0f);
    MakePart(TEXT("RiderHead"), RiderUpperPivot, Sphere, FVector(41,0,60), FVector(0.22f,0.22f,0.24f));
    MakePart(TEXT("RiderHelmet"), RiderUpperPivot, Sphere, FVector(41,0,70), FVector(0.26f,0.26f,0.11f));
    MakeRod(TEXT("UpperArmLeft"), LeftArmPivot, Cube, FVector(0,0,0), FVector(24,-8,-24), 8.0f);
    MakeRod(TEXT("LowerArmLeft"), LeftArmPivot, Cube, FVector(24,-8,-24), FVector(52,-17,-48), 7.0f);
    MakeRod(TEXT("UpperArmRight"), RightArmPivot, Cube, FVector(0,0,0), FVector(24,8,-24), 8.0f);
    MakeRod(TEXT("LowerArmRight"), RightArmPivot, Cube, FVector(24,8,-24), FVector(52,17,-48), 7.0f);
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

namespace BMXStyle
{
    // Name, base points, and either duration (timed) or minimum hold (hold tricks).
    struct FDef { const TCHAR* Name; int32 Points; float Seconds; bool bTimed; };
    static FDef Get(EBMXStyleTrick Trick)
    {
        switch (Trick)
        {
            case EBMXStyleTrick::Barspin:      return { TEXT("BARSPIN"),        150, 0.42f, true  };
            case EBMXStyleTrick::Tailwhip:     return { TEXT("TAILWHIP"),       300, 0.60f, true  };
            case EBMXStyleTrick::NoHander:     return { TEXT("NO-HANDER"),      200, 0.30f, false };
            case EBMXStyleTrick::Superman:     return { TEXT("SUPERMAN"),       300, 0.35f, false };
            case EBMXStyleTrick::CanCan:       return { TEXT("CAN-CAN"),        200, 0.30f, false };
            case EBMXStyleTrick::XUp:          return { TEXT("X-UP"),           180, 0.30f, false };
            case EBMXStyleTrick::Turndown:     return { TEXT("TURNDOWN"),       220, 0.30f, false };
            case EBMXStyleTrick::Tabletop:     return { TEXT("TABLETOP"),       260, 0.35f, false };
            case EBMXStyleTrick::Nothing:      return { TEXT("NOTHING"),        400, 0.40f, false };
            case EBMXStyleTrick::TuckNoHander: return { TEXT("TUCK NO-HANDER"), 250, 0.30f, false };
            default:                           return { TEXT(""),                 0, 0.0f,  true  };
        }
    }
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
    const bool bPedaling = bGrounded && !bInManual && ThrottleInput > 0.05f;
    const float PedalWeight = NormalizedSpeed * (bPedaling ? 1.0f : 0.12f);
    PedalPhase = FMath::Fmod(PedalPhase + Dt * FMath::Abs(SpeedCmPerSec) / 65.0f, 2.0f * PI);
    const float PedalSwing = 20.0f * PedalWeight;

    // ---- Base (riding) pose ----
    FRotator LeftLeg(FMath::Sin(PedalPhase) * PedalSwing, 0, 0);
    FRotator RightLeg(-FMath::Sin(PedalPhase) * PedalSwing, 0, 0);
    FRotator Upper(-8.0f * (bHopCharging ? HopCharge / FMath::Max(0.05f, HopChargeSeconds) : 0.0f), 0, 0);
    FRotator LeftArm = FRotator::ZeroRotator;
    FRotator RightArm = FRotator::ZeroRotator;
    FRotator Bike = FRotator::ZeroRotator;
    FVector BikeOffset = FVector::ZeroVector;
    float BarYaw = SteeringInput * 22.0f;

    // ---- Style trick pose, blended by StylePoseAlpha ----
    const float A = StylePoseAlpha;
    // FRotator components are doubles in UE5; blend in double to avoid narrowing.
    auto Blend = [A](double From, double To) { return FMath::Lerp(From, To, static_cast<double>(A)); };
    switch (PoseStyle)
    {
        case EBMXStyleTrick::NoHander:
            LeftArm = FRotator(60.0f * A, 0, -65.0f * A);
            RightArm = FRotator(60.0f * A, 0, 65.0f * A);
            break;
        case EBMXStyleTrick::Superman:
            Upper.Pitch = Blend(Upper.Pitch, -32.0f);
            LeftLeg = FRotator(Blend(LeftLeg.Pitch, -100.0f), 0, -8.0f * A);
            RightLeg = FRotator(Blend(RightLeg.Pitch, -100.0f), 0, 8.0f * A);
            break;
        case EBMXStyleTrick::CanCan:
            LeftLeg = FRotator(Blend(LeftLeg.Pitch, 25.0f), 0, 75.0f * A);
            break;
        case EBMXStyleTrick::XUp:
            BarYaw = Blend(BarYaw, 180.0f);
            break;
        case EBMXStyleTrick::Turndown:
            BarYaw = Blend(BarYaw, 90.0f);
            Bike.Roll = 30.0f * A;
            Upper.Roll = -12.0f * A;
            break;
        case EBMXStyleTrick::Tabletop:
            Bike.Roll = 70.0f * A;
            Upper.Roll = -18.0f * A;
            LeftLeg.Roll = 25.0f * A;
            RightLeg.Roll = 25.0f * A;
            break;
        case EBMXStyleTrick::Nothing:
            Upper.Pitch = Blend(Upper.Pitch, -20.0f);
            LeftArm = FRotator(60.0f * A, 0, -65.0f * A);
            RightArm = FRotator(60.0f * A, 0, 65.0f * A);
            LeftLeg = FRotator(Blend(LeftLeg.Pitch, -95.0f), 0, -25.0f * A);
            RightLeg = FRotator(Blend(RightLeg.Pitch, -95.0f), 0, 25.0f * A);
            break;
        case EBMXStyleTrick::TuckNoHander:
            LeftArm = FRotator(70.0f * A, 0, -40.0f * A);
            RightArm = FRotator(70.0f * A, 0, 40.0f * A);
            LeftLeg.Pitch = Blend(LeftLeg.Pitch, 40.0f);
            RightLeg.Pitch = Blend(RightLeg.Pitch, 40.0f);
            break;
        default: break;
    }
    // Timed tricks animate by progress rather than pose blend.
    if (ActiveStyle == EBMXStyleTrick::Barspin || ActiveStyle == EBMXStyleTrick::Tailwhip)
    {
        const BMXStyle::FDef Def = BMXStyle::Get(ActiveStyle);
        const float Progress = FMath::Clamp(StyleElapsed / FMath::Max(0.05f, Def.Seconds), 0.0f, 1.0f);
        const float Angle = 360.0f * FMath::InterpEaseInOut(0.0f, 1.0f, Progress, 1.4f);
        if (ActiveStyle == EBMXStyleTrick::Barspin)
        {
            BarYaw = Angle;
        }
        else
        {
            // Tailwhip: the frame swings round the head tube while the bars
            // stay in the rider's hands (counter-rotated).
            Bike.Yaw = Angle;
            const FVector HeadTube(52.0f, 0.0f, 0.0f);
            BikeOffset = HeadTube - FRotator(0.0f, Angle, 0.0f).RotateVector(HeadTube);
            BarYaw = -Angle;
        }
    }

    LeftLegPivot->SetRelativeRotation(LeftLeg);
    RightLegPivot->SetRelativeRotation(RightLeg);
    RiderUpperPivot->SetRelativeRotation(Upper);
    LeftArmPivot->SetRelativeRotation(LeftArm);
    RightArmPivot->SetRelativeRotation(RightArm);
    BikeBodyPivot->SetRelativeLocationAndRotation(BikeOffset, Bike);
    HandlePivot->SetRelativeRotation(FRotator(0.0f, BarYaw, 0.0f));

    // ---- Whole-rider orientation: slope + flips + spins + manual ----
    const float Lean = (bGrounded ? -SteeringInput * NormalizedSpeed * 8.0f : 0.0f);
    const float ManualPitch = ManualAlpha * (16.0f + 10.0f * ManualBalance);
    // A manual pivots round the REAR axle, so the back wheel stays planted.
    const FVector RearAxle(-63.0f, 0.0f, -20.0f);
    const FVector ManualOffset = RearAxle - FRotator(ManualPitch, 0.0f, 0.0f).RotateVector(RearAxle);
    TrickPivot->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 55.0f) + ManualOffset,
        FRotator(FloorPitchDeg + FlipAngleDeg + ManualPitch, SpinAngleDeg, Lean));

    // Vert air: the whole rider + bike visibly turns 180 degrees while the
    // actor keeps its heading; the heading itself flips on landing.
    VisualRoot->SetRelativeRotation(FRotator(0.0f, VertAirYawDeg, 0.0f));
}

void ABMXRiderPawn::BeginPlay()
{
    Super::BeginPlay();
    ResetBike();
    ShowStatus(TEXT("v0.3.3  |  F/G FLIP  Q/E SPIN  SHIFT MANUAL  HOLD W DOWNHILL TO PUMP"), 6.0f);
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

// Lets one handler serve every style-trick button (the trick is the payload).
DECLARE_DELEGATE_OneParam(FBMXStyleInputDelegate, EBMXStyleTrick);

void ABMXRiderPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    check(Input);
    Input->BindAxis(TEXT("Throttle"), this, &ABMXRiderPawn::SetThrottle);
    Input->BindAxis(TEXT("Steer"), this, &ABMXRiderPawn::SetSteering);
    Input->BindAxis(TEXT("CameraYaw"), this, &ABMXRiderPawn::OrbitYaw);
    Input->BindAxis(TEXT("CameraPitch"), this, &ABMXRiderPawn::OrbitPitch);
    Input->BindAxis(TEXT("FlipAxis"), this, &ABMXRiderPawn::SetFlipAxis);
    Input->BindAxis(TEXT("SpinAxis"), this, &ABMXRiderPawn::SetSpinAxis);
    Input->BindAxis(TEXT("AirStickX"), this, &ABMXRiderPawn::SetAirStickX);
    Input->BindAxis(TEXT("AirStickY"), this, &ABMXRiderPawn::SetAirStickY);
    Input->BindAction(TEXT("Hop"), IE_Pressed, this, &ABMXRiderPawn::HopPressed);
    Input->BindAction(TEXT("Hop"), IE_Released, this, &ABMXRiderPawn::HopReleased);
    Input->BindAction(TEXT("Manual"), IE_Pressed, this, &ABMXRiderPawn::ManualPressed);
    Input->BindAction(TEXT("Manual"), IE_Released, this, &ABMXRiderPawn::ManualReleased);

    struct FStyleAction { const TCHAR* Name; EBMXStyleTrick Trick; };
    const FStyleAction StyleActions[] = {
        { TEXT("Barspin"),      EBMXStyleTrick::Barspin },
        { TEXT("Tailwhip"),     EBMXStyleTrick::Tailwhip },
        { TEXT("NoHander"),     EBMXStyleTrick::NoHander },
        { TEXT("Superman"),     EBMXStyleTrick::Superman },
        { TEXT("CanCan"),       EBMXStyleTrick::CanCan },
        { TEXT("XUp"),          EBMXStyleTrick::XUp },
        { TEXT("Turndown"),     EBMXStyleTrick::Turndown },
        { TEXT("Tabletop"),     EBMXStyleTrick::Tabletop },
        { TEXT("Nothing"),      EBMXStyleTrick::Nothing },
        { TEXT("TuckNoHander"), EBMXStyleTrick::TuckNoHander },
    };
    for (const FStyleAction& Action : StyleActions)
    {
        Input->BindAction<FBMXStyleInputDelegate>(FName(Action.Name), IE_Pressed, this,
            &ABMXRiderPawn::StylePressed, Action.Trick);
        Input->BindAction<FBMXStyleInputDelegate>(FName(Action.Name), IE_Released, this,
            &ABMXRiderPawn::StyleReleased, Action.Trick);
    }

    Input->BindAction(TEXT("Reset"), IE_Pressed, this, &ABMXRiderPawn::ResetBike);
    Input->BindAction(TEXT("DirtLine"), IE_Pressed, this, &ABMXRiderPawn::ResetToDirtLine);
    Input->BindAction(TEXT("ParkStart"), IE_Pressed, this, &ABMXRiderPawn::ResetToPark);
    Input->BindAction(TEXT("RecenterCamera"), IE_Pressed, this, &ABMXRiderPawn::RecenterCamera);
    Input->BindAction(TEXT("ToggleDebug"), IE_Pressed, this, &ABMXRiderPawn::ToggleDebugHUD);
}

void ABMXRiderPawn::SetFlipAxis(float Value) { FlipAxisInput = FMath::Clamp(Value,-1.0f,1.0f); }
void ABMXRiderPawn::SetSpinAxis(float Value) { SpinAxisInput = FMath::Clamp(Value,-1.0f,1.0f); }
void ABMXRiderPawn::SetAirStickX(float Value) { AirStickX = FMath::Clamp(Value,-1.0f,1.0f); }
void ABMXRiderPawn::SetAirStickY(float Value) { AirStickY = FMath::Clamp(Value,-1.0f,1.0f); }

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
    Contact.bQuarterPipe = Hit.GetComponent() && Hit.GetComponent()->ComponentHasTag(FName(TEXT("BMX_QUARTER")));
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

    if (bHopCharging)
    {
        if (bGrounded) HopCharge = FMath::Min(HopChargeSeconds, HopCharge + TotalDt);
        else bHopCharging = false;
    }
    UpdateManual(TotalDt);
    // Combo link window after a clean landing: press manual to keep the
    // combo going, otherwise it banks. Leaving the ground also banks it.
    if (LandingLinkSeconds > 0.0f)
    {
        LandingLinkSeconds -= TotalDt;
        if (bGrounded && bManualHeld)
        {
            LandingLinkSeconds = 0.0f;
            StartManual();
        }
        else if (LandingLinkSeconds <= 0.0f || !bGrounded)
        {
            LandingLinkSeconds = 0.0f;
            BankCombo();
        }
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
        if (!bGrounded) UpdateAirRotation(TotalDt);
        UpdateStyleTrick(TotalDt);
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

    // v0.3.0: steering only on the ground. In the air A/D spin the rider
    // instead (see UpdateAirRotation); the landing carries the new heading.
    if (bGrounded)
    {
        const float TurningStrength = FMath::Clamp(FMath::Abs(SpeedCmPerSec) / 450.0f, 0.18f, 1.0f);
        const float DirectionSign = SpeedCmPerSec >= 0.0f ? 1.0f : -1.0f;
        AddActorWorldRotation(FRotator(0.0f,
            SteeringInput * SteeringRateDeg * Dt * TurningStrength * DirectionSign, 0.0f));
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
        FBMXRidingSupport NextSupport = ProbeSupport(Start + HorizontalStep);
        if (NextSupport.HasContact()) bOnQuarterPipe = NextSupport.OnQuarterPipe();
        // v0.3.3: going UP and the front wheel suddenly finds ground lower
        // than the rear (e.g. the gap behind a dirt lip) = the front wheel
        // is past the crest. Treat it as unsupported rather than averaging
        // the gap floor into the ride height (that pulled the bike down into
        // the lip and felt like getting stuck at the top).
        const bool bClimbingForward = FloorPitchDeg > 8.0f && SpeedCmPerSec > 0.0f;
        if (bClimbingForward && NextSupport.Front.bSupported && NextSupport.Rear.bSupported &&
            NextSupport.Front.FloorZ + 15.0f < NextSupport.Rear.FloorZ)
        {
            NextSupport.Front.bSupported = false;
        }
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
        // Quarter-pipes launch as soon as the front wheel clears the coping
        // (vert air). Other lips (dirt, kickers) ride out on the rear wheel
        // first - see the crest branch below.
        const bool bOpenQuarterPipeTakeoff =
            bRearOnRisingRamp && bFrontPastOpenLip && NextSupport.Rear.bQuarterPipe;
        const bool bRearWheelCrest =
            bClimbingForward && bRearOnRisingRamp && !NextSupport.Front.bSupported &&
            !NextSupport.Rear.bQuarterPipe;
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
        else if (bRearWheelCrest)
        {
            // Front wheel is over the lip: carry on up the last part of the
            // lip on the rear wheel, the bike pitched at the lip's angle, and
            // take off when the rear wheel leaves the top (handled by the
            // "left the surface" launch below). Smooth, and the takeoff now
            // happens from the real lip height at the lip's real angle.
            const float RearSlopeDeg = FMath::RadiansToDegrees(
                FMath::Acos(FMath::Clamp(float(NextSupport.Rear.Normal.Z), 0.0f, 1.0f)));
            FloorPitchDeg = FMath::FInterpTo(FloorPitchDeg, FMath::Clamp(RearSlopeDeg, 0.0f, 62.0f), Dt, 10.0f);
            const float PitchRad = FMath::DegreesToRadians(FloorPitchDeg);
            const float DesiredCenterZ = NextSupport.Rear.FloorZ +
                63.0f * FMath::Sin(PitchRad) + (TireBottomBelowPawn + GroundClearance) * FMath::Cos(PitchRad);
            const FVector Intended = Start + HorizontalStep;
            FHitResult CrestLiftHit;
            SetActorLocation(FVector(Start.X, Start.Y, FMath::Max(Start.Z, DesiredCenterZ) + 1.0f), true, &CrestLiftHit);
            FHitResult CrestMoveHit;
            bAttemptedGroundHorizontal = true;
            SetActorLocation(FVector(Intended.X, Intended.Y, GetActorLocation().Z), true, &CrestMoveHit);
            if (!CrestMoveHit.bBlockingHit)
            {
                bGrounded = true;
                bCompletedGroundMotion = true;
                VerticalSpeed = 0.0f;
                AirTime = 0.0f;
                StuckAirSeconds = 0.0f;
                LeftSurfaceSeconds = 0.0f;
                bFrontWheelContact = false;
                bRearWheelContact = true;
                RearWheelFloorZ = NextSupport.Rear.FloorZ;
            }
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
        // v0.3.1: brushing a rideable slope (e.g. a landing's face) is a
        // touchdown, not a wall hit - only real walls scrub speed.
        if (HorizontalHit.ImpactNormal.Z < 0.35f) SpeedCmPerSec *= 0.78f;
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
    // v0.3.1: on steep faces (dirt landing backsides, ramp faces) the body
    // collider touches the slope BEFORE the centre reaches wheel height, which
    // used to leave the bike hanging until "PHYSICS RECOVERY". Touching any
    // rideable surface with wheel support underneath now counts as landing.
    const bool bTouchingRideable =
        (VerticalHit.bBlockingHit && VerticalHit.ImpactNormal.Z > 0.35f) ||
        (HorizontalHit.bBlockingHit && HorizontalHit.ImpactNormal.Z > 0.35f);
    const bool bCloseEnoughToLand = LandingSupport.HasContact() &&
        EndPosition.Z <= LandingZ + 9.0f &&
        (EndPosition.Z >= LandingZ - 18.0f || (bTouchingRideable && EndPosition.Z >= LandingZ - 80.0f));
    if (JumpGrace <= 0.0f && LandingCooldown <= 0.0f && VerticalSpeed <= 0.0f &&
        bCloseEnoughToLand)
    {
        FHitResult LandHit;
        SetActorLocation(FVector(EndPosition.X, EndPosition.Y, LandingZ), true, &LandHit);
        float LandedPitch = LandingSupport.SurfacePitchDegrees();
        const bool bWasVertAir = bVertAir;
        // Checks rotation, turns the bike round for vert airs / 180s and
        // scores flips and spins. A bad landing has already bailed.
        if (!ResolveLanding(LandedPitch)) return;
        // Keep momentum: project the air velocity onto the landing slope.
        // Coming down a ramp face turns the fall into forward speed, so a
        // vert air drops back in with pace instead of from a standstill.
        const float LandedPitchRad = FMath::DegreesToRadians(FMath::Clamp(LandedPitch, -62.0f, 62.0f));
        SpeedCmPerSec = SpeedCmPerSec * FMath::Cos(LandedPitchRad) + VerticalSpeed * FMath::Sin(LandedPitchRad);
        if (bWasVertAir)
        {
            // Dropping back in hands back the speed you launched with (the
            // launch only shows VertLaunchScale of it as height), so energy
            // carries from wall to wall and pumping builds bigger airs.
            const float Magnitude = FMath::Max(FMath::Abs(SpeedCmPerSec),
                -VerticalSpeed / FMath::Max(0.3f, VertLaunchScale) * VertLandingKeep);
            SpeedCmPerSec = SpeedCmPerSec >= 0.0f ? Magnitude : -Magnitude;
        }
        SpeedCmPerSec = FMath::Clamp(SpeedCmPerSec, -MaxForwardSpeed * 1.8f, MaxForwardSpeed * 1.8f);
        bGrounded = true;
        VerticalSpeed = 0.0f;
        AirTime = 0.0f;
        StuckAirSeconds = 0.0f;
        LeftSurfaceSeconds = 0.0f;
        FloorPitchDeg = FMath::Clamp(LandedPitch, -62.0f, 62.0f);
        LandingCooldown = 0.13f;
        AfterCleanLanding();
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
            float RecoveryPitch = 0.0f;
            if (!ResolveLanding(RecoveryPitch)) return;
            FloorPitchDeg = 0.0f;
            bGrounded = true;
            VerticalSpeed = 0.0f;
            StuckAirSeconds = 0.0f;
            AirTime = 0.0f;
            LandingCooldown = 0.13f;
            AfterCleanLanding();
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
    // In a manual W/S balance the bike instead of pedaling: you coast.
    const float Throttle = bInManual ? 0.0f : ThrottleInput;
    if (Throttle > 0.02f)
    {
        const float Target = MaxForwardSpeed * Throttle;
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
            SpeedCmPerSec = FMath::FInterpConstantTo(SpeedCmPerSec, Target, Dt, HighSpeedDrag);
        }
        // Pumping: holding W while dropping down a transition pushes the bike
        // into the slope and adds speed, so back-and-forth airs build up.
        const float Descent = -FloorPitchDeg * (SpeedCmPerSec >= 0.0f ? 1.0f : -1.0f);
        if (Descent > 6.0f)
        {
            SpeedCmPerSec += (SpeedCmPerSec >= 0.0f ? 1.0f : -1.0f) * PumpAcceleration *
                FMath::Sin(FMath::DegreesToRadians(Descent)) * Throttle * Dt;
        }
    }
    else if (Throttle < -0.02f)
    {
        SpeedCmPerSec = FMath::FInterpConstantTo(SpeedCmPerSec, ReverseSpeed * Throttle, Dt, Acceleration);
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
    SpeedCmPerSec = FMath::Clamp(SpeedCmPerSec, -MaxForwardSpeed * 1.8f, MaxForwardSpeed * 1.8f);
    if (FMath::Abs(SpeedCmPerSec) < 1.5f) SpeedCmPerSec = 0.0f;
}

void ABMXRiderPawn::LaunchIntoAir(float ExtraUpSpeed, float MinUpSpeed)
{
    constexpr float Gravity = 980.0f;
    const float PitchRad = FMath::DegreesToRadians(FloorPitchDeg);
    const float AlongSurface = SpeedCmPerSec;
    // v0.3.0: only quarter-pipes give vert airs. Steep dirt lips and the
    // tabletop launch you forward instead.
    const bool bVert = bOnQuarterPipe && FloorPitchDeg >= VertLaunchPitchDeg && AlongSurface > 0.0f;

    bGrounded = false;
    AirTime = 0.0f;
    bHopCharging = false;
    FlipAngleDeg = 0.0f;
    SpinAngleDeg = 0.0f;
    FlipVelocityDeg = 0.0f;
    SpinVelocityDeg = 0.0f;
    bStickFlipArmed = false;
    bStickSpinArmed = false;
    bFrontWheelContact = false;
    bRearWheelContact = false;

    if (bVert)
    {
        // Straight up out of the quarter-pipe, then back in. Horizontal speed
        // is replaced by a slow drift back over the ramp (negative = toward
        // the ramp, since the actor still faces up the ramp).
        // v0.3.1: the coping is near-vertical, so (almost) all of the speed
        // goes upward. Using sin(chord pitch) here threw away ~35% of the
        // speed every air, so airs shrank instead of building.
        VerticalSpeed = FMath::Clamp(
            AlongSurface * VertLaunchScale + ExtraUpSpeed * 0.3f,
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

// ---------------------------------------------------------------------------
// Bunny hop: hold Space to load up, release to pop. Works from manuals too.
// ---------------------------------------------------------------------------
void ABMXRiderPawn::HopPressed()
{
    if (bBailed || !bGrounded) return;
    bHopCharging = true;
    HopCharge = 0.0f;
}

void ABMXRiderPawn::HopReleased()
{
    if (!bHopCharging) return;
    bHopCharging = false;
    if (bBailed || !bGrounded) return;
    const float Charge = FMath::Clamp(HopCharge / FMath::Max(0.05f, HopChargeSeconds), 0.0f, 1.0f);
    DoHop(FMath::Lerp(MinHopScale, MaxHopScale, Charge));
}

void ABMXRiderPawn::DoHop(float Power)
{
    // A hop on a slope keeps the slope's momentum (and can start a vert air
    // near the top of a quarter-pipe).
    LaunchIntoAir(HopSpeed * Power + FMath::Clamp(FMath::Abs(SpeedCmPerSec)*0.055f,0.0f,65.0f),
                  HopSpeed * 0.6f * Power);
    JumpGrace = 0.22f;
    if (!bVertAir) FloorPitchDeg *= 0.5f;
}

// ---------------------------------------------------------------------------
// Manuals: hold Shift on the ground. W/S keep your balance; too far back
// loops out, too far forward drops the nose. A manual keeps the combo alive
// between landings; letting go (or stalling) banks it.
// ---------------------------------------------------------------------------
void ABMXRiderPawn::ManualPressed()
{
    bManualHeld = true;
    if (bGrounded && !bBailed && !bInManual) StartManual();
}

void ABMXRiderPawn::ManualReleased()
{
    bManualHeld = false;
}

void ABMXRiderPawn::StartManual()
{
    if (bInManual || !bGrounded || bBailed || FMath::Abs(SpeedCmPerSec) < 120.0f) return;
    bInManual = true;
    ManualSeconds = 0.0f;
    ManualBalance = 0.08f;
    // W/S only balance once released after the manual starts, so W held
    // from pedaling doesn't slam the nose down.
    bManualInputArmed = FMath::Abs(ThrottleInput) < 0.2f;
    LandingLinkSeconds = 0.0f;
    AddComboTrick(TEXT("MANUAL"), 50);
}

void ABMXRiderPawn::EndManual(bool bBank)
{
    if (!bInManual) return;
    bInManual = false;
    if (bBank) BankCombo();
}

void ABMXRiderPawn::UpdateManual(float Dt)
{
    if (bInManual)
    {
        if (!bGrounded || bBailed)
        {
            // Hopped out of the manual: the combo carries into the air.
            bInManual = false;
        }
        else
        {
            ManualSeconds += Dt;
            if (FMath::Abs(ThrottleInput) < 0.2f) bManualInputArmed = true;
            const float BalanceInput = bManualInputArmed ? ThrottleInput : 0.0f;
            // Slowly self-amplifying tilt plus a gentle wobble; W pushes the
            // nose down, S pulls it back. Tuned so a tap corrects it.
            const float Wobble = 0.25f * FMath::Sin(ManualSeconds * 2.3f) +
                                 0.12f * FMath::Sin(ManualSeconds * 4.7f + 1.3f);
            ManualBalance += (ManualBalance * ManualInstability + Wobble * 0.6f -
                              BalanceInput * ManualControl) * Dt;
            ManualPointAccum += ManualPointsPerSecond * Dt;
            const int32 Whole = FMath::FloorToInt(ManualPointAccum);
            UnbankedPoints += Whole;
            ManualPointAccum -= float(Whole);
            if (ManualBalance > 1.0f)
            {
                Bail(TEXT("LOOPED OUT"));
            }
            else if (ManualBalance < -1.0f)
            {
                Bail(TEXT("NOSE DOWN - MANUAL LOST"));
            }
            else if (!bManualHeld || FMath::Abs(SpeedCmPerSec) < 80.0f)
            {
                EndManual(true);
            }
        }
    }
    ManualAlpha = FMath::FInterpTo(ManualAlpha, bInManual ? 1.0f : 0.0f, Dt, 10.0f);
}

// ---------------------------------------------------------------------------
// Air rotation: flips on W/S (or F/G), spins on A/D (or Q/E). Rotation
// eases in and out, and when you let go near a full turn it settles onto it,
// so landings stay readable without being automatic.
// ---------------------------------------------------------------------------
void ABMXRiderPawn::UpdateAirRotation(float Dt)
{
    // v0.3.1: keyboard rotates only with F/G (flip) and Q/E (spin), so W/S
    // and A/D habits from riding never start a flip. The gamepad stick also
    // rotates, once it has returned to centre after takeoff.
    if (FMath::Abs(AirStickY) < 0.2f) bStickFlipArmed = true;
    if (FMath::Abs(AirStickX) < 0.2f) bStickSpinArmed = true;
    // Stick forward leans forward = frontflip (nose down = negative pitch).
    const float FlipIn = FMath::Clamp(FlipAxisInput - (bStickFlipArmed ? AirStickY : 0.0f), -1.0f, 1.0f);
    const float SpinIn = FMath::Clamp(SpinAxisInput + (bStickSpinArmed ? AirStickX : 0.0f), -1.0f, 1.0f);

    auto Drive = [Dt](float Input, float Rate, float Angle, float Snap, float& Velocity)
    {
        if (FMath::Abs(Input) > 0.1f)
        {
            Velocity = FMath::FInterpTo(Velocity, Input * Rate, Dt, 9.0f);
        }
        else
        {
            // Settle toward the nearest whole rotation if close, else coast down.
            const float Nearest = FMath::RoundToFloat(Angle / Snap) * Snap;
            const float Residual = Angle - Nearest;
            const float Target = FMath::Abs(Residual) < 70.0f ? -Residual * 5.0f : Velocity * 0.6f;
            Velocity = FMath::FInterpTo(Velocity, Target, Dt, 6.0f);
        }
        return Angle + Velocity * Dt;
    };
    FlipAngleDeg = Drive(FlipIn, FlipRateDeg, FlipAngleDeg, 360.0f, FlipVelocityDeg);
    SpinAngleDeg = Drive(SpinIn, SpinRateDeg, SpinAngleDeg, 180.0f, SpinVelocityDeg);
}

// ---------------------------------------------------------------------------
// Style tricks (barspin, tailwhip, no-hander, superman...). One at a time,
// layered on top of any flip/spin.
// ---------------------------------------------------------------------------
void ABMXRiderPawn::StylePressed(EBMXStyleTrick Trick)
{
    if (bBailed || bGrounded || Trick == EBMXStyleTrick::None) return;
    if (ActiveStyle != EBMXStyleTrick::None)
    {
        const BMXStyle::FDef Current = BMXStyle::Get(ActiveStyle);
        // A timed trick must finish; pressing the same hold trick is a no-op.
        if (Current.bTimed || ActiveStyle == Trick) return;
        // Switching hold tricks mid-air: the old one ends (scored if held long enough).
        if (!bStyleScored && StyleElapsed >= Current.Seconds)
            AddComboTrick(Current.Name, Current.Points);
    }
    ActiveStyle = Trick;
    PoseStyle = BMXStyle::Get(Trick).bTimed ? EBMXStyleTrick::None : Trick;
    StyleElapsed = 0.0f;
    StyleHoldPointAccum = 0.0f;
    bStyleHeld = true;
    bStyleScored = false;
    // Snap the blend so a switch reads as a new pose, not a morph.
    if (!BMXStyle::Get(Trick).bTimed) StylePoseAlpha = FMath::Min(StylePoseAlpha, 0.3f);
    ShowStatus(BMXStyle::Get(Trick).Name, 0.8f);
}

void ABMXRiderPawn::StyleReleased(EBMXStyleTrick Trick)
{
    if (ActiveStyle == Trick) bStyleHeld = false;
}

void ABMXRiderPawn::UpdateStyleTrick(float Dt)
{
    if (ActiveStyle == EBMXStyleTrick::None)
    {
        StylePoseAlpha = FMath::FInterpConstantTo(StylePoseAlpha, 0.0f, Dt, 7.0f);
        if (StylePoseAlpha <= 0.0f) PoseStyle = EBMXStyleTrick::None;
        return;
    }
    const BMXStyle::FDef Def = BMXStyle::Get(ActiveStyle);
    StyleElapsed += Dt;
    if (Def.bTimed)
    {
        if (StyleElapsed >= Def.Seconds)
        {
            AddComboTrick(Def.Name, Def.Points);
            ActiveStyle = EBMXStyleTrick::None;
        }
        return;
    }
    StylePoseAlpha = FMath::FInterpConstantTo(StylePoseAlpha, 1.0f, Dt, 8.0f);
    if (!bStyleScored && StyleElapsed >= Def.Seconds)
    {
        AddComboTrick(Def.Name, Def.Points);
        bStyleScored = true;
    }
    if (bStyleScored && bStyleHeld)
    {
        // Holding a pose longer keeps adding points.
        StyleHoldPointAccum += Def.Points * 0.6f * Dt;
        const int32 Whole = FMath::FloorToInt(StyleHoldPointAccum);
        UnbankedPoints += Whole;
        StyleHoldPointAccum -= float(Whole);
    }
    // Released: the pose ends once its minimum hold is done.
    if (!bStyleHeld && StyleElapsed >= Def.Seconds)
    {
        ActiveStyle = EBMXStyleTrick::None;
    }
}

void ABMXRiderPawn::AddComboTrick(const FString& Name, int32 Points)
{
    // Repeating a trick inside one combo is worth less each time
    // (100%, 50%, 25%...), rewarding variety like classic BMX games.
    int32 Repeats = 0;
    for (const FString& Done : CompletedComboTricks)
    {
        if (Done == Name) ++Repeats;
    }
    UnbankedPoints += FMath::Max(10, Points >> FMath::Min(Repeats, 4));
    CompletedComboTricks.Add(Name);
}

// ---------------------------------------------------------------------------
// Landing: checks flips are near a whole turn and spins near a half turn,
// turns the bike for vert airs / 180s (landing fakie), and scores rotations.
// ---------------------------------------------------------------------------
bool ABMXRiderPawn::ResolveLanding(float& InOutLandedPitch)
{
    const float VertYaw = bVertAir ? 180.0f : 0.0f;
    bVertAir = false;
    VertAirYawDeg = 0.0f;

    const float FlipTurns = FMath::RoundToFloat(FlipAngleDeg / 360.0f);
    const float FlipResidual = FlipAngleDeg - FlipTurns * 360.0f;
    const float TotalYaw = VertYaw + SpinAngleDeg;
    const float HalfTurns = FMath::RoundToFloat(TotalYaw / 180.0f);
    const float YawResidual = TotalYaw - HalfTurns * 180.0f;
    const int32 Flips = FMath::RoundToInt(FlipTurns);
    const int32 SpinHalfTurns = FMath::RoundToInt(SpinAngleDeg / 180.0f);

    // An odd number of half turns means the bike now points the other way.
    if (FMath::Abs(FMath::RoundToInt(HalfTurns)) % 2 == 1)
    {
        AddActorWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
        SpeedCmPerSec = -SpeedCmPerSec;
        InOutLandedPitch = -InOutLandedPitch;
        CameraIdleSeconds = 10.0f; // let the camera swing round behind the rider
    }
    // A slightly-off spin carries into the new heading instead of snapping.
    AddActorWorldRotation(FRotator(0.0f, YawResidual, 0.0f));

    FlipAngleDeg = 0.0f;
    SpinAngleDeg = 0.0f;
    FlipVelocityDeg = 0.0f;
    SpinVelocityDeg = 0.0f;

    if (FMath::Abs(FlipResidual) > LandingToleranceDeg)
    {
        Bail(FlipResidual > 0.0f ? TEXT("OVER-ROTATED - BAIL") : TEXT("UNDER-ROTATED - BAIL"));
        return false;
    }
    if (FMath::Abs(YawResidual) > LandingToleranceDeg)
    {
        Bail(TEXT("LANDED SIDEWAYS - BAIL"));
        return false;
    }
    if (ActiveStyle != EBMXStyleTrick::None && BMXStyle::Get(ActiveStyle).bTimed)
    {
        Bail(TEXT("INCOMPLETE TRICK - BAIL"));
        return false;
    }
    if (ActiveStyle != EBMXStyleTrick::None || StylePoseAlpha > 0.5f)
    {
        Bail(TEXT("TIED UP - LET GO BEFORE LANDING"));
        return false;
    }

    if (Flips != 0)
    {
        const bool bBack = Flips > 0;
        const int32 Count = FMath::Abs(Flips);
        const TCHAR* Prefix = Count == 1 ? TEXT("") : (Count == 2 ? TEXT("DOUBLE ") : TEXT("TRIPLE "));
        const int32 Base = bBack ? 450 : 500;
        AddComboTrick(FString::Printf(TEXT("%s%s"), Prefix, bBack ? TEXT("BACKFLIP") : TEXT("FRONTFLIP")),
                      Base * Count + (Count > 1 ? Base * (Count - 1) / 2 : 0));
    }
    if (SpinHalfTurns != 0)
    {
        const int32 Degrees = FMath::Abs(SpinHalfTurns) * 180;
        AddComboTrick(FString::Printf(TEXT("%d"), Degrees), Degrees / 2);
    }
    return true;
}

void ABMXRiderPawn::AfterCleanLanding()
{
    ActiveStyle = EBMXStyleTrick::None;
    if (bManualHeld)
    {
        // Landing straight into a manual keeps the combo going.
        StartManual();
        if (bInManual) return;
    }
    if (!CompletedComboTricks.IsEmpty())
    {
        // Short window to catch a manual before the combo banks.
        LandingLinkSeconds = 0.3f;
    }
}

void ABMXRiderPawn::ResetVisuals()
{
    TrickPivot->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 55.0f), FRotator(FloorPitchDeg,0,0));
    BikeBodyPivot->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
    HandlePivot->SetRelativeRotation(FRotator::ZeroRotator);
    RiderUpperPivot->SetRelativeRotation(FRotator::ZeroRotator);
    LeftArmPivot->SetRelativeRotation(FRotator::ZeroRotator);
    RightArmPivot->SetRelativeRotation(FRotator::ZeroRotator);
    LeftLegPivot->SetRelativeRotation(FRotator::ZeroRotator);
    RightLegPivot->SetRelativeRotation(FRotator::ZeroRotator);
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
    UnbankedPoints = 0;
    LandingLinkSeconds = 0.0f;
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
    ActiveStyle = EBMXStyleTrick::None;
    PoseStyle = EBMXStyleTrick::None;
    StylePoseAlpha = 0.0f;
    bInManual = false;
    ManualAlpha = 0.0f;
    bHopCharging = false;
    LandingLinkSeconds = 0.0f;
    FlipAngleDeg = 0.0f;
    SpinAngleDeg = 0.0f;
    FlipVelocityDeg = 0.0f;
    SpinVelocityDeg = 0.0f;
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
    UnbankedPoints = 0;
    ActiveStyle = EBMXStyleTrick::None;
    PoseStyle = EBMXStyleTrick::None;
    StylePoseAlpha = 0.0f;
    StyleElapsed = 0.0f;
    bInManual = false;
    ManualAlpha = 0.0f;
    ManualBalance = 0.0f;
    bHopCharging = false;
    HopCharge = 0.0f;
    LandingLinkSeconds = 0.0f;
    FlipAngleDeg = 0.0f;
    SpinAngleDeg = 0.0f;
    FlipVelocityDeg = 0.0f;
    SpinVelocityDeg = 0.0f;
    bOnQuarterPipe = false;
    SetActorLocationAndRotation(ResetLocation, ResetRotation, false, nullptr, ETeleportType::TeleportPhysics);
    SpeedCmPerSec = SpawnRollSpeed; // rolling start on the dirt line, 0 in the park
    ResetVisuals();
    RecenterCamera();
}
// Key 2: respawn at the start of the dirt jump line (and keep respawning
// there after bails). Key 1: back to the park start.
void ABMXRiderPawn::ResetToDirtLine()
{
    // v0.3.2: start 5 m off the wall so the camera isn't squashed into the
    // rider, with a rolling start at jump speed (the old 7 m run-in from a
    // standstill left you too slow, landing short in the gap).
    ResetLocation = FVector(-2500.0f, -2350.0f, 60.0f);
    ResetRotation = FRotator(0.0f, 0.0f, 0.0f);
    SpawnRollSpeed = 1150.0f;
    ResetBike();
    ShowStatus(TEXT("DIRT LINE  |  40-45 KM/H LANDS THE DOWNSLOPE - EASE OFF W IF YOU OVERSHOOT"), 4.0f);
}
void ABMXRiderPawn::ResetToPark()
{
    ResetLocation = FVector(-150.0f, -1150.0f, 60.0f);
    ResetRotation = FRotator(0.0f, 180.0f, 0.0f);
    SpawnRollSpeed = 0.0f;
    ResetBike();
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
