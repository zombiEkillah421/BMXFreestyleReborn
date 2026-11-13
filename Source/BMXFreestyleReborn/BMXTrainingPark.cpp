#include "BMXTrainingPark.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/UObjectGlobals.h"

namespace BMXParkGeometry
{
    // UE procedural meshes use the winding opposite to the usual right-handed
    // cross-product normal. A triangle wound the other way may appear to be
    // missing when viewed from above and can fail front-face collision tests.
    // Store the explicit OUTWARD normal and choose winding consistently.
    static void Quad(TArray<FVector>& Vertices, TArray<int32>& Triangles,
                     TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                     const FVector& A, const FVector& B,
                     const FVector& C, const FVector& D,
                     const FVector& Outward, float UExtent, float VExtent)
    {
        const int32 I = Vertices.Num();
        const FVector N = Outward.GetSafeNormal();
        Vertices.Add(A);
        Vertices.Add(B);
        Vertices.Add(C);
        Vertices.Add(D);
        for (int32 K = 0; K < 4; ++K) Normals.Add(N);
        UVs.Add(FVector2D(0.0f, 0.0f));
        UVs.Add(FVector2D(UExtent, 0.0f));
        UVs.Add(FVector2D(UExtent, VExtent));
        UVs.Add(FVector2D(0.0f, VExtent));

        const bool bNeedsReverse =
            FVector::DotProduct(FVector::CrossProduct(B - A, C - A), N) > 0.0f;
        if (bNeedsReverse)
        {
            Triangles.Add(I); Triangles.Add(I + 2); Triangles.Add(I + 1);
            Triangles.Add(I); Triangles.Add(I + 3); Triangles.Add(I + 2);
        }
        else
        {
            Triangles.Add(I); Triangles.Add(I + 1); Triangles.Add(I + 2);
            Triangles.Add(I); Triangles.Add(I + 2); Triangles.Add(I + 3);
        }
    }

    static UMaterialInterface* TintedMaterial(AActor* Owner, UMaterialInterface* Base,
                                               const FLinearColor& Color)
    {
        if (!Base) return nullptr;
        UMaterialInstanceDynamic* Instance = UMaterialInstanceDynamic::Create(Base, Owner);
        if (!Instance) return Base;
        // /Engine/BasicShapes/BasicShapeMaterial supplies this vector parameter.
        Instance->SetVectorParameterValue(FName(TEXT("Color")), Color);
        return Instance;
    }
}

ABMXTrainingPark::ABMXTrainingPark()
{
    PrimaryActorTick.bCanEverTick = false;
    ParkRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ParkRoot"));
    RootComponent = ParkRoot;
}

void ABMXTrainingPark::AddBox(const FVector& Center, const FVector& Dimensions,
                               const FRotator& Rotation, UMaterialInterface* Material)
{
    if (!CubeMesh) return;
    UStaticMeshComponent* Box = NewObject<UStaticMeshComponent>(this);
    Box->SetupAttachment(RootComponent);
    Box->SetStaticMesh(CubeMesh);
    Box->SetRelativeLocation(Center);
    Box->SetRelativeRotation(Rotation);
    Box->SetRelativeScale3D(Dimensions / 100.0f);
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    if (Material == ConcreteMaterial.Get()) Box->ComponentTags.Add(FName(TEXT("BMX_FLOOR")));
    else if (Material == LedgeMaterial.Get()) Box->ComponentTags.Add(FName(TEXT("BMX_LEDGE")));
    else if (Material == RailMaterial.Get()) Box->ComponentTags.Add(FName(TEXT("BMX_RAIL")));
    else if (Material == WallMaterial.Get()) Box->ComponentTags.Add(FName(TEXT("BMX_WALL")));
    else Box->ComponentTags.Add(FName(TEXT("BMX_BOX")));
    if (Material) Box->SetMaterial(0, Material);
    Box->RegisterComponent();
}

void ABMXTrainingPark::AddGrindEdge(const FVector& LocalStart, const FVector& LocalEnd, const TCHAR* Label)
{
    FBMXGrindSegment Segment;
    Segment.Start = GetActorTransform().TransformPosition(LocalStart);
    Segment.End = GetActorTransform().TransformPosition(LocalEnd);
    Segment.Label = Label;
    GrindSegments.Add(Segment);
}

void ABMXTrainingPark::AddBoxGrindEdges(const FVector& Center, const FVector& Dimensions, const TCHAR* Label)
{
    const FVector Half = Dimensions * 0.5f;
    const float TopZ = Center.Z + Half.Z;
    const bool bLongAlongX = Dimensions.X >= Dimensions.Y;
    for (const float Side : { -1.0f, 1.0f })
    {
        if (bLongAlongX)
        {
            AddGrindEdge(FVector(Center.X - Half.X, Center.Y + Side * Half.Y, TopZ),
                         FVector(Center.X + Half.X, Center.Y + Side * Half.Y, TopZ), Label);
        }
        else
        {
            AddGrindEdge(FVector(Center.X + Side * Half.X, Center.Y - Half.Y, TopZ),
                         FVector(Center.X + Side * Half.X, Center.Y + Half.Y, TopZ), Label);
        }
    }
}

void ABMXTrainingPark::AddRail(const FVector& Center, float Length, float Height)
{
    // Grind line along the top of the 8 cm-thick rail tube.
    AddGrindEdge(Center + FVector(-Length * 0.5f, 0.0f, Height + 4.0f),
                 Center + FVector(Length * 0.5f, 0.0f, Height + 4.0f), TEXT("RAIL"));
    if (!CylinderMesh) return;
    UStaticMeshComponent* Rail = NewObject<UStaticMeshComponent>(this);
    Rail->SetupAttachment(RootComponent);
    Rail->SetStaticMesh(CylinderMesh);
    Rail->SetRelativeLocation(Center + FVector(0, 0, Height));
    Rail->SetRelativeRotation(FRotator(90, 0, 0)); // Z axis of cylinder -> X axis
    Rail->SetRelativeScale3D(FVector(0.08f, 0.08f, Length / 100.0f));
    Rail->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Rail->SetCollisionObjectType(ECC_WorldStatic);
    Rail->SetCollisionResponseToAllChannels(ECR_Block);
    Rail->SetGenerateOverlapEvents(false);
    Rail->ComponentTags.Add(FName(TEXT("BMX_RAIL")));
    if (RailMaterial) Rail->SetMaterial(0, RailMaterial);
    Rail->RegisterComponent();
    AddBox(Center + FVector(-Length * 0.42f, 0, Height * 0.5f),
           FVector(12, 12, Height), FRotator::ZeroRotator, RailMaterial);
    AddBox(Center + FVector(Length * 0.42f, 0, Height * 0.5f),
           FVector(12, 12, Height), FRotator::ZeroRotator, RailMaterial);
}

void ABMXTrainingPark::AddRamp(const FVector& Origin, const FRotator& Rotation,
                               float Length, float Width, float Height, bool bCurved)
{
    // v0.1.6: Replace the unclosed, back-facing ramp plane with one CLOSED,
    // visible solid. All exterior faces are in a single ProceduralMesh section.
    // Collision uses the exact triangles rendered, never an invisible box.
    UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetRelativeLocation(Origin);
    Mesh->SetRelativeRotation(Rotation);
    Mesh->bUseAsyncCooking = false; // collision ready before first play frame
    Mesh->bUseComplexAsSimpleCollision = true;
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldStatic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    Mesh->SetGenerateOverlapEvents(false);
    Mesh->ComponentTags.Add(FName(TEXT("BMX_RAMP")));
    // Curved ramps are quarter-pipes: the rider may vert-air off them.
    if (bCurved) Mesh->ComponentTags.Add(FName(TEXT("BMX_QUARTER")));
    Mesh->SetVisibility(true);
    Mesh->SetCastShadow(true);
    Mesh->RegisterComponent();

    const int32 Steps = bCurved ? 30 : 8;
    const float HalfWidth = Width * 0.5f;
    const float BottomZ = -14.0f; // slightly below the ground pad (top = Z 0)
    // v0.2.0: 60-degree arc parameter. With the park's height/length ratios the
    // tops finish at ~58-62 degrees: steep enough for vert airs, while the
    // compact rider collider still clears the face (needs < ~66 degrees).
    constexpr float MaxAngle = PI / 3.0f;
    const float SinAngle = FMath::Sin(MaxAngle);
    const float Rise = 1.0f - FMath::Cos(MaxAngle);

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    Vertices.Reserve(Steps * 16 + 16);
    Triangles.Reserve(Steps * 24 + 24);
    Normals.Reserve(Steps * 16 + 16);
    UVs.Reserve(Steps * 16 + 16);

    auto Profile = [=](float T) -> FVector2D
    {
        if (!bCurved) return FVector2D(Length * T, Height * T);
        const float Theta = T * MaxAngle;
        return FVector2D(Length * FMath::Sin(Theta) / SinAngle,
                         Height * (1.0f - FMath::Cos(Theta)) / Rise);
    };

    for (int32 I = 0; I < Steps; ++I)
    {
        const FVector2D P0 = Profile(float(I) / float(Steps));
        const FVector2D P1 = Profile(float(I + 1) / float(Steps));
        const float X0 = P0.X;
        const float X1 = P1.X;
        const float Z0 = P0.Y;
        const float Z1 = P1.Y;
        const float Slope = (Z1 - Z0) / FMath::Max(1.0f, X1 - X0);
        const FVector TopNormal(-Slope, 0.0f, 1.0f);

        // Front-facing riding surface (the old index order faced DOWN).
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(X0, -HalfWidth, Z0), FVector(X0, HalfWidth, Z0),
            FVector(X1, HalfWidth, Z1), FVector(X1, -HalfWidth, Z1),
            TopNormal, Width / 180.0f, (X1-X0) / 180.0f);

        // Visible and collidable side panels. They stop the ramp looking like
        // a nearly invisible floating sheet when viewed from an angle.
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(X0, -HalfWidth, BottomZ), FVector(X0, -HalfWidth, Z0),
            FVector(X1, -HalfWidth, Z1), FVector(X1, -HalfWidth, BottomZ),
            FVector(0, -1, 0), (X1-X0)/180.0f, Height/180.0f);
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(X0, HalfWidth, BottomZ), FVector(X1, HalfWidth, BottomZ),
            FVector(X1, HalfWidth, Z1), FVector(X0, HalfWidth, Z0),
            FVector(0, 1, 0), (X1-X0)/180.0f, Height/180.0f);
    }

    // End and underside close the collision mesh. These faces exactly match
    // their visible geometry; no separate invisible collision cuboids exist.
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(Length, -HalfWidth, BottomZ), FVector(Length, HalfWidth, BottomZ),
        FVector(Length, HalfWidth, Height), FVector(Length, -HalfWidth, Height),
        FVector(1, 0, 0), Width / 180.0f, Height / 180.0f);
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(0, -HalfWidth, BottomZ), FVector(0, -HalfWidth, 0),
        FVector(0, HalfWidth, 0), FVector(0, HalfWidth, BottomZ),
        FVector(-1, 0, 0), Width / 180.0f, 1.0f);
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(0, -HalfWidth, BottomZ), FVector(Length, -HalfWidth, BottomZ),
        FVector(Length, HalfWidth, BottomZ), FVector(0, HalfWidth, BottomZ),
        FVector(0, 0, -1), Length / 180.0f, Width / 180.0f);

    Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs,
                            TArray<FColor>(), TArray<FProcMeshTangent>(), true);
    if (RampMaterial) Mesh->SetMaterial(0, RampMaterial);

    // Real collision sanity check using the game world's visibility trace.
    // Log the component identity: hitting the pad underneath is NOT success.
    if (UWorld* World = GetWorld())
    {
        const FVector2D Middle = Profile(0.5f);
        const FVector LocalProbe(Middle.X, 0.0f, Middle.Y);
        const FVector WorldProbe = Origin + Rotation.RotateVector(LocalProbe);
        FHitResult Hit;
        const FVector TraceStart(WorldProbe.X, WorldProbe.Y, WorldProbe.Z + 400.0f);
        const FVector TraceEnd(WorldProbe.X, WorldProbe.Y, WorldProbe.Z - 200.0f);
        // Do not ignore the park actor: the trace must be able to hit this ramp.
        // Checking the actual hit component avoids mistaking the floor for a ramp.
        const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility);
        UE_LOG(LogTemp, Log, TEXT("[BMX PARK v0.3.0] Ramp at %s; triangles=%d; trace=%s; target=%s"),
               *Origin.ToString(), Triangles.Num() / 3,
               bHit ? TEXT("HIT") : TEXT("MISS"),
               (bHit && Hit.GetComponent() == Mesh) ? TEXT("RAMP") : TEXT("OTHER"));
    }
}

void ABMXTrainingPark::AddProfileMound(const FVector& Origin, float Yaw, float Width,
                                       const TArray<FVector2D>& Profile, UMaterialInterface* Material)
{
    if (Profile.Num() < 2) return;
    UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this);
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetRelativeLocation(Origin);
    Mesh->SetRelativeRotation(FRotator(0.0f, Yaw, 0.0f));
    Mesh->bUseAsyncCooking = false;
    Mesh->bUseComplexAsSimpleCollision = true;
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldStatic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    Mesh->SetGenerateOverlapEvents(false);
    // Rideable like a ramp, but NOT a quarter-pipe (no vert airs).
    Mesh->ComponentTags.Add(FName(TEXT("BMX_RAMP")));
    Mesh->ComponentTags.Add(FName(TEXT("BMX_DIRT")));
    Mesh->SetCastShadow(true);
    Mesh->RegisterComponent();

    const float HalfWidth = Width * 0.5f;
    const float BottomZ = -14.0f;
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    for (int32 I = 0; I + 1 < Profile.Num(); ++I)
    {
        const FVector2D P0 = Profile[I];
        const FVector2D P1 = Profile[I + 1];
        const float Slope = (P1.Y - P0.Y) / FMath::Max(1.0f, float(P1.X - P0.X));
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(P0.X, -HalfWidth, P0.Y), FVector(P0.X, HalfWidth, P0.Y),
            FVector(P1.X, HalfWidth, P1.Y), FVector(P1.X, -HalfWidth, P1.Y),
            FVector(-Slope, 0.0f, 1.0f), Width / 180.0f, (P1.X - P0.X) / 180.0f);
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(P0.X, -HalfWidth, BottomZ), FVector(P0.X, -HalfWidth, P0.Y),
            FVector(P1.X, -HalfWidth, P1.Y), FVector(P1.X, -HalfWidth, BottomZ),
            FVector(0, -1, 0), (P1.X - P0.X) / 180.0f, 1.0f);
        BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
            FVector(P0.X, HalfWidth, BottomZ), FVector(P1.X, HalfWidth, BottomZ),
            FVector(P1.X, HalfWidth, P1.Y), FVector(P0.X, HalfWidth, P0.Y),
            FVector(0, 1, 0), (P1.X - P0.X) / 180.0f, 1.0f);
    }
    const FVector2D First = Profile[0];
    const FVector2D Last = Profile.Last();
    // End caps (the lip's steep back face) and underside close the solid.
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(First.X, -HalfWidth, BottomZ), FVector(First.X, -HalfWidth, First.Y + 0.5f),
        FVector(First.X, HalfWidth, First.Y + 0.5f), FVector(First.X, HalfWidth, BottomZ),
        FVector(-1, 0, 0), Width / 180.0f, 1.0f);
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(Last.X, -HalfWidth, BottomZ), FVector(Last.X, HalfWidth, BottomZ),
        FVector(Last.X, HalfWidth, Last.Y + 0.5f), FVector(Last.X, -HalfWidth, Last.Y + 0.5f),
        FVector(1, 0, 0), Width / 180.0f, 1.0f);
    BMXParkGeometry::Quad(Vertices, Triangles, Normals, UVs,
        FVector(First.X, -HalfWidth, BottomZ), FVector(Last.X, -HalfWidth, BottomZ),
        FVector(Last.X, HalfWidth, BottomZ), FVector(First.X, HalfWidth, BottomZ),
        FVector(0, 0, -1), (Last.X - First.X) / 180.0f, Width / 180.0f);

    Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs,
                            TArray<FColor>(), TArray<FProcMeshTangent>(), true);
    if (Material) Mesh->SetMaterial(0, Material);
}

// One dirt jump set heading +X: a 130 cm lip (curving up to ~40 degrees), a
// 200 cm gap, then a long landing: 30-degree backside, short deck, rounded
// knuckle and a two-stage downslope. Tuned (2D trajectory check) so roughly
// 36-41 km/h approach lands on the downslope, ~32 km/h on the deck; faster
// rolls onto the flat run-out, slower cases the backside and rides over.
void ABMXTrainingPark::AddDirtJump(float LipStartX, float Y)
{
    constexpr float LipLength = 200.0f;
    constexpr float LipHeight = 130.0f;
    constexpr float LipAngle = 40.0f * PI / 180.0f;
    TArray<FVector2D> Lip;
    for (int32 I = 0; I <= 14; ++I)
    {
        const float Theta = LipAngle * float(I) / 14.0f;
        Lip.Add(FVector2D(LipLength * FMath::Sin(Theta) / FMath::Sin(LipAngle),
                          LipHeight * (1.0f - FMath::Cos(Theta)) / (1.0f - FMath::Cos(LipAngle))));
    }
    AddProfileMound(FVector(LipStartX, Y, 0.0f), 0.0f, 300.0f, Lip, DirtMaterial);

    // v0.3.2: 30-degree backside (was 45) so a short landing can still be
    // ridden up and over instead of leaving you stuck at its base.
    const TArray<FVector2D> Landing = {
        FVector2D(0.0f, 0.0f), FVector2D(260.0f, 150.0f), FVector2D(320.0f, 150.0f),
        FVector2D(350.0f, 148.0f), FVector2D(380.0f, 141.0f), FVector2D(420.0f, 128.0f),
        FVector2D(670.0f, 66.0f), FVector2D(880.0f, 12.0f), FVector2D(930.0f, 2.0f),
        FVector2D(970.0f, 0.0f) };
    // v0.3.5: gap 280 -> 200 cm (riders were coming up short).
    AddProfileMound(FVector(LipStartX + LipLength + 200.0f, Y, 0.0f), 0.0f, 340.0f, Landing, DirtMaterial);
}

void ABMXTrainingPark::BeginPlay()
{
    Super::BeginPlay();
    CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    // Clear high-contrast differentiation: dark concrete, amber ramps,
    // lighter ledges, silver rails. Avoid near-white-on-white surfaces.
    ConcreteMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.19f,0.23f,0.29f,1.0f));
    RampMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.93f,0.47f,0.13f,1.0f));
    LedgeMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.30f,0.62f,0.76f,1.0f));
    RailMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.77f,0.83f,0.87f,1.0f));
    WallMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.13f,0.14f,0.21f,1.0f));
    DirtMaterial = BMXParkGeometry::TintedMaterial(this, Basic, FLinearColor(0.36f,0.22f,0.12f,1.0f));

    // 60x60 metre self-contained test park. Floor TOP is Z=0.
    AddBox(FVector(0, 0, -35), FVector(6000, 6000, 70), FRotator::ZeroRotator, ConcreteMaterial);

    // v0.1.8: Quarter-pipes face INWARD toward the central riding lanes.
    // Previously the high, vertical BACK wall ended at X=-590 or X=740,
    // directly in the riding area. Players collided with those tall walls
    // instead of approaching the curved face. The low lip now faces X=0.
    // Left group: low lip X=-690; high back X=-1250/-1190 (away from center).
    // Right group: low lip X=690; high back X=1250/1190 (away from center).
    AddRamp(FVector(-690, -1150, 0), FRotator(0, 180, 0), 560, 1150, 350, true);
    AddRamp(FVector( 690, -1150, 0), FRotator::ZeroRotator, 560, 1150, 350, true);
    AddRamp(FVector(-690,  1350, 0), FRotator(0, 180, 0), 500, 1000, 270, true);
    AddRamp(FVector( 690,  1350, 0), FRotator::ZeroRotator, 500, 1000, 270, true);

    // Quarter-pipes intentionally end at their lips, with clear space behind.
    // When a wheel leaves the curved surface the new riding controller carries
    // its upward momentum into the air; a raised deck here would leave another
    // vertical wall in the approach lane and recreate the old snag.

    // Tabletop: matching wedge heights at each end and a visible solid deck.
    AddRamp(FVector(-150, -100, 0), FRotator::ZeroRotator, 360, 550, 165, false);
    AddBox(FVector(485, -100, 82.5f), FVector(550, 550, 165), FRotator::ZeroRotator, LedgeMaterial);
    AddBoxGrindEdges(FVector(485, -100, 82.5f), FVector(550, 550, 165), TEXT("DECK EDGE"));
    AddRamp(FVector(1120, -100, 0), FRotator(0, 180, 0), 360, 550, 165, false);

    AddBox(FVector(-1550, 650, 40), FVector(540, 300, 80), FRotator::ZeroRotator, LedgeMaterial);
    AddBoxGrindEdges(FVector(-1550, 650, 40), FVector(540, 300, 80), TEXT("LEDGE"));
    AddRail(FVector(-300, 650, 0), 440, 98);
    AddRail(FVector(800, 800, 0), 350, 80);

    // v0.3.0 dirt jump line along the south edge (Y=-2350), heading +X.
    // Press 2 in game for a rolling start at its run-in. Three sets, 1710 cm apart.
    AddDirtJump(-2150.0f, -2350.0f);
    AddDirtJump(-440.0f, -2350.0f);
    AddDirtJump(1270.0f, -2350.0f);

    // Perimeter walls are deliberately visible, not invisible world blockers.
    AddBox(FVector(0,-2990,100), FVector(6000,20,200), FRotator::ZeroRotator, WallMaterial);
    AddBox(FVector(0, 2990,100), FVector(6000,20,200), FRotator::ZeroRotator, WallMaterial);
    AddBox(FVector(-2990,0,100), FVector(20,6000,200), FRotator::ZeroRotator, WallMaterial);
    AddBox(FVector(2990,0,100), FVector(20,6000,200), FRotator::ZeroRotator, WallMaterial);

    UE_LOG(LogTemp, Log, TEXT("[BMX PARK v0.3.0] Quarter-pipes (vert), tabletop, ledge, rails and dirt jump line initialized."));
}
