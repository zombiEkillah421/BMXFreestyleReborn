#include "BMXHUD.h"
#include "BMXRiderPawn.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

void ABMXHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GEngine || !GetOwningPlayerController()) return;
    const ABMXRiderPawn* Rider = Cast<ABMXRiderPawn>(GetOwningPlayerController()->GetPawn());
    if (!Rider) return;
    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const FLinearColor Dark(0.025f,0.035f,0.055f,0.85f);
    const FLinearColor White(0.96f,0.98f,1.0f,1.0f);
    const FLinearColor Cyan(0.17f,0.92f,0.98f,1.0f);
    const FLinearColor Gold(1.0f,0.78f,0.25f,1.0f);
    const bool bDebug = Rider->IsDebugHUDVisible();
    const float PanelHeight = bDebug ? 192.0f : 104.0f;
    DrawRect(Dark, 26, 24, 460, PanelHeight);
    DrawRect(Cyan, 26, 24, 6, PanelHeight);
    DrawText(TEXT("BMX FREESTYLE REBORN  |  PARK 01 - v0.3.3"), White, 46, 34, GEngine->GetSmallFont(), 1.05f, false);
    DrawText(FString::Printf(TEXT("SCORE   %07d"), Rider->GetTotalScore()), Gold, 45, 64, GEngine->GetMediumFont(), 1.35f, false);
    if (bDebug)
    {
        // Diagnostic readout for playtest recordings. TAB / gamepad View toggles it.
        DrawText(FString::Printf(TEXT("SPEED  %02.0f KM/H"), Rider->GetSpeedKmh()), Cyan, 46, 104, GEngine->GetSmallFont(), 1.04f, false);
        const FString MoveState = Rider->IsMovementBlocked() ? TEXT("BLOCKED") :
            Rider->IsInManual() ? TEXT("MANUAL") :
            Rider->IsRidingFakie() ? TEXT("FAKIE") :
            Rider->IsGrounded() ? TEXT("GROUND") :
            Rider->IsVertAir() ? TEXT("VERT AIR") : TEXT("AIR");
        DrawText(FString::Printf(TEXT("INPUT W/S %+.1f   |   %s"), Rider->GetPedalInput(), *MoveState),
                 White, 46, 132, GEngine->GetSmallFont(), 0.95f, false);
        DrawText(FString::Printf(TEXT("WHEELS  F:%s  R:%s  |  SLOPE %+.0f DEG"),
            Rider->HasFrontWheelContact() ? TEXT("ON") : TEXT("OFF"),
            Rider->HasRearWheelContact() ? TEXT("ON") : TEXT("OFF"),
            Rider->GetPitchDegrees()), Cyan, 46, 157, GEngine->GetSmallFont(), 0.90f, false);
        DrawText(Rider->IsMovementBlocked() ?
                 FString::Printf(TEXT("HIT: %s"), *Rider->GetBlockerName()) :
                 TEXT("TAB: HIDE DEBUG INFO"),
                 Rider->IsMovementBlocked() ? Gold : White,
                 46, 182, GEngine->GetSmallFont(), 0.90f, false);
    }
    if (Rider->HasCombo())
    {
        DrawRect(Dark, W * 0.5f - 245, H - 230, 490, 120);
        DrawRect(Gold, W * 0.5f - 245, H - 230, 490, 5);
        DrawText(FString::Printf(TEXT("COMBO  %d POINTS  |  x%.2f"), Rider->GetComboPoints(), Rider->GetComboMultiplier()), Gold,
                 W * 0.5f - 215, H - 210, GEngine->GetMediumFont(), 1.2f, false);
        DrawText(Rider->GetComboNames(), White, W * 0.5f - 215, H - 167, GEngine->GetSmallFont(), 1.05f, false);
    }
    if (Rider->IsInManual())
    {
        // Balance meter: keep the marker in the middle with W / S.
        const float MeterW = 320.0f;
        const float MeterX = W * 0.5f - MeterW * 0.5f;
        const float MeterY = H * 0.62f;
        const float Balance = FMath::Clamp(Rider->GetManualBalance(), -1.0f, 1.0f);
        DrawRect(Dark, MeterX - 6, MeterY - 26, MeterW + 12, 50);
        DrawRect(FLinearColor(0.8f,0.2f,0.15f,0.9f), MeterX, MeterY, MeterW, 10);
        DrawRect(FLinearColor(0.2f,0.75f,0.35f,0.9f), MeterX + MeterW * 0.3f, MeterY, MeterW * 0.4f, 10);
        DrawRect(White, MeterX + MeterW * 0.5f * (1.0f + Balance) - 3, MeterY - 6, 6, 22);
        DrawText(TEXT("MANUAL  -  W NOSE DOWN / S PULL BACK"), White, MeterX, MeterY - 24, GEngine->GetSmallFont(), 0.9f, false);
    }
    if (!Rider->GetStatusMessage().IsEmpty())
    {
        DrawRect(Dark, W*0.5f - 240, H*0.23f, 480, 46);
        DrawText(Rider->GetStatusMessage(), Cyan, W*0.5f - 215, H*0.23f + 10, GEngine->GetMediumFont(), 1.0f, false);
    }
    DrawRect(Dark, 26, H - 92, W - 52, 67);
    DrawText(TEXT("W/S PEDAL (HOLD W DOWNHILL = PUMP)   A/D STEER   HOLD SPACE BIG HOP   SHIFT MANUAL   AIR: F/G FLIP  Q/E SPIN   1 PARK  2 DIRT"), White,
             42, H - 81, GEngine->GetSmallFont(), 0.94f, false);
    DrawText(TEXT("B BARSPIN  T TAILWHIP  J NO-HANDER  K SUPERMAN  L CAN-CAN  U X-UP  I TURNDOWN  O TABLETOP  H NOTHING  N TUCK"), Cyan,
             42, H - 52, GEngine->GetSmallFont(), 0.94f, false);
}
