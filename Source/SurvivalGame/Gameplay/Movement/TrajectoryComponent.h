// Copyright 2018 Sticks & Stones. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ThirdParty/glm/glm.hpp"
#include "TrajectoryComponent.generated.h"


UCLASS(BlueprintType)
class SURVIVALGAME_API UTrajectoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTrajectoryComponent();
	~UTrajectoryComponent();

	enum { LENGTH = 120 };

	//LOG THESE VARIABLES

	//Basic values
	glm::vec3 TargetDirection;
	glm::vec3 TargetVelocity;
	UPROPERTY(EditAnywhere, Category = BaseValues)
	float Width;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseValues)
	float StrafeAmount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseValues)
	float StrafeTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = BaseValues)
	float Responsive;

	//Extra smoothing values
	UPROPERTY(EditAnywhere, Category = ExtraSmoothing)
	float ExtraVelocitySmooth;
	UPROPERTY(EditAnywhere, Category = ExtraSmoothing)
	float ExtraStrafeVelocity;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExtraSmoothing)
	float ExtraDirectionSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExtraSmoothing)
	float ExtraStrafeSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExtraSmoothing)
	float ExtraGaitSmooth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExtraSmoothing)
	float ExtraJointSmooth;

	//Positional data
	glm::vec3 Positions[LENGTH];
	glm::vec3 Directions[LENGTH];
	glm::mat3 Rotations[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = PositionalData)
	float Heights[LENGTH];

	//Gaits
	UPROPERTY(VisibleAnywhere, Category = Gaits)
	float GaitStand[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = Gaits)
	float GaitWalk[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = Gaits)
	float GaitJog[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = Gaits)
	float GaitJump[LENGTH];
	UPROPERTY(VisibleAnywhere, Category = Gaits)
	float GaitBump[LENGTH];

	glm::vec3 GetRootPosition() const;
	glm::vec3 GetPreviousRootPosition() const;
	glm::mat3 GetRootRotation() const;
	glm::mat3 GetPreviousRootRotation() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* arg_ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:



	APawn* OwnerPawn;

	glm::vec2 CurrentFrameInput;

	void TickGaits();
	void PredictFutureTrajectory(const float DeltaSeconds);
	void TickRotations();
	void TickHeights();
	void UpdatePastTrajectory();

	/*
	* @Description Returnes a liniar bled between the X and Y vector direction by using the floating point scalar
	* @Param[in] X vector
	* @Param[in] Y vector
	* @Param[in] Scalar
	* @Return The result of liniar blending X, Y and the scalar
	*/
	static glm::vec3 MixDirections(const glm::vec3 arg_XDirection, const glm::vec3 arg_YDirection, const float arg_Scalar);



#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Debug)
	bool bShowDebugInformation;
#endif

#if !UE_BUILD_SHIPPING //Debug functions are excluded from the shipping build
	void DrawDebugTrajectory();
	//void DrawDebugUI();
#endif
};
