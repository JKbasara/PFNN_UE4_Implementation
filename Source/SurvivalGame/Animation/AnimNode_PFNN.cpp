// Copyright 2018 Sticks & Stones. All Rights Reserved.

#include "AnimNode_PFNN.h"
#include "PFNNAnimInstance.h"
#include "Gameplay/Movement/TrajectoryComponent.h"

#include "AnimInstanceProxy.h"
#include "MachineLearning/PhaseFunctionNeuralNetwork.h"
#include "PlatformFilemanager.h"

#include <ThirdParty/glm/gtx/transform.inl>

UPhaseFunctionNeuralNetwork* FAnimNode_PFNN::PFNN = nullptr;

FAnimNode_PFNN::FAnimNode_PFNN(): Trajectory(nullptr), PFNNAnimInstance(nullptr), PFNNMode(2)
{
}

void FAnimNode_PFNN::LoadData()
{
	LoadXForms();
	LoadPFNN();
}

void FAnimNode_PFNN::LoadXForms()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const FString RelativePath = FPaths::ProjectDir();
	const FString FullPathParents = RelativePath + FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/character_parents.bin"));
	const FString FullPathXforms = RelativePath + FString::Printf(TEXT("Content/MachineLearning/PhaseFunctionNeuralNetwork/character_xforms.bin"));

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FullPathParents);
	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load charater parents"));
		return;
	}
	float JointParentsFloat[JOINT_NUM];
	FileHandle->Read(reinterpret_cast<uint8*>(JointParentsFloat), sizeof(JointParentsFloat));

	for (int i = 0; i < JOINT_NUM; i++)
	{
		JointParents[i] = static_cast<int>(JointParentsFloat[i]);
	}


	FileHandle = PlatformFile.OpenRead(*FullPathXforms);
	if (FileHandle == nullptr)
	{
		//UE_LOG(NeuralNetworkLoading, Fatal, TEXT("Fatal error, Failed to load character xforms"));
		return;
	}
	FileHandle->Read(reinterpret_cast<uint8*>(JointRestXform), sizeof(JointRestXform));

	for (int i = 0; i < JOINT_NUM; i++)
	{
		JointRestXform[i] = glm::transpose(JointRestXform[i]);
	}

	delete FileHandle;
}

void FAnimNode_PFNN::LoadPFNN() const
{
	if(PFNN == nullptr || PFNN->mode != PFNNMode)
	{
		PFNN = NewObject<UPhaseFunctionNeuralNetwork>();
		PFNN->mode = PFNNMode;
		PFNN->LoadNetworkData();
	}
}

void FAnimNode_PFNN::ApplyPFNN()
{
	const glm::vec3 RootPosition = glm::vec3(
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2].x + Trajectory->GetOwner()->GetActorLocation().X,
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2].y + Trajectory->GetOwner()->GetActorLocation().Y,
		/*Trajectory->Heights[LENGTH / Half]*/0.0f);

	//const glm::vec3 RootPosition = glm::vec3(
	//	 GetActorLocation().X,
	//	 GetActorLocation().Y,
	//	/*Trajectory->Heights[LENGTH / Half]*/0.0f);


	const glm::mat3 RootRotation = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2];

	//Input trajectiory positions and directions
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		int w = UTrajectoryComponent::LENGTH / 10;
		const glm::vec3 Position = glm::inverse((RootRotation)) * (Trajectory->Positions[i] - RootPosition);
		const glm::vec3 Direction = glm::inverse((RootRotation)) * Trajectory->Directions[i];
		PFNN->Xp((w * 0) + i / 10) = Position.x;
		PFNN->Xp((w * 1) + i / 10) = Position.y;
		PFNN->Xp((w * 2) + i / 10) = Direction.x;
		PFNN->Xp((w * 3) + i / 10) = Direction.y;
	}

	// Input trajectory gaits
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		const int w = UTrajectoryComponent::LENGTH / 10;
		PFNN->Xp((w * 4) + i / 10) = Trajectory->GaitStand[i];
		PFNN->Xp((w * 5) + i / 10) = Trajectory->GaitWalk[i];
		PFNN->Xp((w * 6) + i / 10) = Trajectory->GaitJog[i];
		PFNN->Xp((w * 7) + i / 10) = 0; //Unused input for crouch?;
		PFNN->Xp((w * 8) + i / 10) = Trajectory->GaitJump[i];
		PFNN->Xp((w * 9) + i / 10) = 0; //Unused input
	}

	//Input previous join position / velocity / rotations
	const glm::vec3 PreviousRootPosition = glm::vec3(
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2 - 1].x,
		Trajectory->Positions[UTrajectoryComponent::LENGTH / 2 - 1].y,
		/*Trajectory->Heights[UTrajectoryComponent::LENGTH / 2 - 1]*/0.0f);
	const glm::mat3 PreviousRootRotation = Trajectory->Rotations[UTrajectoryComponent::LENGTH / 2 - 1];
	for (int i = 0; i < JOINT_NUM; i++)
	{
		const int o = (((UTrajectoryComponent::LENGTH) / 10)*10);
		const glm::vec3 Position = glm::inverse(PreviousRootRotation) * (JointPosition[i] - PreviousRootPosition);
		const glm::vec3 Previous = glm::inverse(PreviousRootRotation) * JointVelocitys[i];
		//Magical numbers are indexes for the PFNN
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 0) = Position.x;
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 1) = Position.z;
		PFNN->Xp(o + (JOINT_NUM * 3 * 0) + i * 3 + 2) = Position.y;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 0) = Previous.x;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 1) = Previous.z;
		PFNN->Xp(o + (JOINT_NUM * 3 * 1) + i * 3 + 2) = Previous.y;
	}

	//Input heights for the trajectory
	for (int i = 0; i < UTrajectoryComponent::LENGTH; i += 10)
	{
		const int o = (((UTrajectoryComponent::LENGTH) / 10)*10) + JOINT_NUM * 3 * 2;
		const int w = UTrajectoryComponent::LENGTH / 10;

		const glm::vec3 PositionRight	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(Trajectory->Width, 0, 0));
		const glm::vec3 PositionLeft	= Trajectory->Positions[i] + (Trajectory->Rotations[i] * glm::vec3(-Trajectory->Width, 0, 0));

		FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("GroundGeometryTrace")), true, Trajectory->GetOwner());
		TraceParams.bTraceComplex = true;
		TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = false;

		const float DistanceLenght = 10000;
		FHitResult HitResultLeft(ForceInit);
		FHitResult HitResultRight(ForceInit);

		const FVector UPositionRight = FVector(PositionRight.x, PositionRight.y, PositionRight.z);
		const FVector UPositionLeft = FVector(PositionLeft.x, PositionLeft.y, PositionLeft.z);

		Trajectory->GetOwner()->GetWorld()->LineTraceSingleByChannel(HitResultRight, UPositionLeft, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);
		Trajectory->GetOwner()->GetWorld()->LineTraceSingleByChannel(HitResultLeft, UPositionRight, -FVector::UpVector * DistanceLenght, ECC_Pawn, TraceParams);

		//TODO: Add height addition
		PFNN->Xp(o + (w * 0) + i / 10) = /*HitResultRight.Location.Z*/ 0 - RootPosition.z;
		PFNN->Xp(o + (w * 1) + i / 10) = Trajectory->Positions[i].z; /*- RootPosition.z*/
		PFNN->Xp(o + (w * 2) + i / 10) = /*HitResultLeft.Location.Z-*/ 0 - RootPosition.z;
	}

	//Preform regression
	PFNN->Predict(Phase);

	const float StandAmount = powf(1.0f - Trajectory->GaitStand[UTrajectoryComponent::LENGTH / 2], 0.25f);
	Phase = fmod(Phase + (StandAmount * 0.9f + 0.1f) * 2.0f * PI * PFNN->Yp(3), 2.0f * PI);
}

UPFNNAnimInstance * FAnimNode_PFNN::GetPFNNInstanceFromContext(const FAnimationInitializeContext& Context)
{
	FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy;
	if (AnimProxy)
	{
		return Cast<UPFNNAnimInstance>(AnimProxy->GetAnimInstanceObject());
	}
	return nullptr;
}

UPFNNAnimInstance * FAnimNode_PFNN::GetPFNNInstanceFromContext(const FAnimationUpdateContext & Context)
{
	FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy;
	if (AnimProxy)
	{
		return Cast<UPFNNAnimInstance>(AnimProxy->GetAnimInstanceObject());
	}
	return nullptr;
}

void FAnimNode_PFNN::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	
	EvaluateGraphExposedInputs.Execute(Context);

	PFNNAnimInstance = GetPFNNInstanceFromContext(Context);
	if (!PFNNAnimInstance) 
	{
		UE_LOG(LogTemp, Error, TEXT("PFNN Animation node should only be added to a PFNNAnimInstance child class!"));
	}

	LoadData();
}

void FAnimNode_PFNN::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	FAnimNode_Base::Update_AnyThread(Context);

	EvaluateGraphExposedInputs.Execute(Context);

	if (PFNNAnimInstance) 
	{
		Trajectory = PFNNAnimInstance->GetOwningTrajectoryComponent();
	}
	

	if(Trajectory != nullptr)
		ApplyPFNN();

}

void FAnimNode_PFNN::Evaluate_AnyThread(FPoseContext& Output)
{
	const FTransform& CharacterTransform = Output.AnimInstanceProxy->GetActorTransform();

	for (int32 i = 0; i < Output.Pose.GetNumBones(); i++)
	{
		const FCompactPoseBoneIndex RootBoneIndex(i);

		Output.Pose[RootBoneIndex].SetLocation(Output.Pose[RootBoneIndex].GetLocation());
		Output.Pose[RootBoneIndex].SetRotation(Output.Pose[RootBoneIndex].GetRotation());

		Output.AnimInstanceProxy->AnimDrawDebugSphere(Output.Pose[RootBoneIndex].GetLocation() + CharacterTransform.GetLocation(), 2.5f, 12, FColor::Green, false, -1.0f);
	}
	Output.Pose.NormalizeRotations();
}
