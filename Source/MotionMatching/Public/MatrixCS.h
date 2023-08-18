#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"

#include "Eigen/Eigen"

#include <atomic>

class MOTIONMATCHING_API MatrixCS_Manager
{
public:
	MatrixCS_Manager(){};
	TArray<float> CostArray;
	TArray<float> resultIndex;
	float WinIndex = -1;
	int vitualNum = 0;
	int trueNum = 0;
public:
	FStructuredBufferRHIRef _featureDataBuffer;
	FUnorderedAccessViewRHIRef _featureDataBufferUAV;    

	FStructuredBufferRHIRef _featureVectorBuffer;
	FUnorderedAccessViewRHIRef _featureVectorBufferUAV;

	FStructuredBufferRHIRef _CostArrayBuffer;
	FUnorderedAccessViewRHIRef _CostArrayBufferUAV;

	FStructuredBufferRHIRef _WinnerIndexBuffer;
	FUnorderedAccessViewRHIRef _WinnerIndexBufferUAV;
public:
	void Init(Eigen::MatrixXf& featureDataBase);
	void Compute(FRHICommandListImmediate& RHICommands,const Eigen::VectorXf& featrueVector,const float Responsiveness, 
							const float VelocityStrength, 
							const float PoseStrength);
};