#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BMXGameMode.generated.h"

UCLASS()
class BMXFREESTYLEREBORN_API ABMXGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ABMXGameMode();
    virtual void StartPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;
};
