// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PFNNGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UPFNNGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPFNNGameInstance(const FObjectInitializer& arg_ObjectInitializer);

	class UPFNNDataContainer* GetPFNNDataContainer();

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"))
	class UPFNNDataContainer* PFNNDataContainer;

};
