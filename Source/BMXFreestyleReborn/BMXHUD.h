#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BMXHUD.generated.h"

UCLASS()
class BMXFREESTYLEREBORN_API ABMXHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
