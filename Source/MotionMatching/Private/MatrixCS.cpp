#include "MatrixCS.h"

#if ENGINE_MINOR_VERSION < 26

#include "ShaderParameterUtils.h"

#else

#include "ShaderCompilerCore.h"

#endif

#include "RHIStaticStates.h"
#include "Eigen/Eigen"
#include <chrono>

#include "SceneView.h"
#include "UniformBuffer.h"
#include <Shader.h>
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FMMconst,)
	SHADER_PARAMETER(float,Responsiveness)
	SHADER_PARAMETER(float,VelocityStrength)
	SHADER_PARAMETER(float,PoseStrength)
	SHADER_PARAMETER(int,rows)
	SHADER_PARAMETER(int,cols)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FMMconst, "FMMconst");

class FMatirxCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMatirxCS, Global);
public:
	FMatirxCS(){}

	FMatirxCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FGlobalShader(Initializer)
	{
		featureData.Bind(Initializer.ParameterMap, TEXT("featureData"));
		featureVector.Bind(Initializer.ParameterMap, TEXT("featureVector"));
		CostArray.Bind(Initializer.ParameterMap, TEXT("CostArray"));
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return GetMaxSupportedFeatureLevel(Parameters.Platform) >= ERHIFeatureLevel::SM5;
	};

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
	}
	void SetUniform(FRHICommandListImmediate& RHICommands,float r,float v,float p,int rows,int cols)
	{
		FMMconst UniformData;
		UniformData.Responsiveness = r;
		UniformData.VelocityStrength = v;
		UniformData.PoseStrength = p;
		UniformData.rows = rows;
		UniformData.cols = cols;
		SetUniformBufferParameterImmediate(RHICommands,RHICommands.GetBoundComputeShader(),GetUniformBufferParameter<FMMconst>(),UniformData);
	}
public:
	LAYOUT_FIELD(FShaderResourceParameter, featureData);
	LAYOUT_FIELD(FShaderResourceParameter, featureVector);
	LAYOUT_FIELD(FShaderResourceParameter, CostArray);

	
};
IMPLEMENT_SHADER_TYPE(, FMatirxCS, TEXT("/MyShadersShaders/MatrixCS.usf"), TEXT("MainComputeShader"), SF_Compute);

class FSortCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSortCS, Global);
public:
	FSortCS(){}

	FSortCS(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FGlobalShader(Initializer)
	{
		CostArray.Bind(Initializer.ParameterMap, TEXT("CostArray"));
		Index.Bind(Initializer.ParameterMap, TEXT("Index"));
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return GetMaxSupportedFeatureLevel(Parameters.Platform) >= ERHIFeatureLevel::SM5;
	};

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(CFLAG_StandardOptimization);
	}
	void SetUniform(FRHICommandListImmediate& RHICommands,float r,float v,float p,int rows,int cols)
	{
		FMMconst UniformData;
		UniformData.Responsiveness = r;
		UniformData.VelocityStrength = v;
		UniformData.PoseStrength = p;
		UniformData.rows = rows;
		UniformData.cols = cols;
		SetUniformBufferParameterImmediate(RHICommands,RHICommands.GetBoundComputeShader(),GetUniformBufferParameter<FMMconst>(),UniformData);
	}
public:
	LAYOUT_FIELD(FShaderResourceParameter, CostArray);
	LAYOUT_FIELD(FShaderResourceParameter, Index);
};
IMPLEMENT_SHADER_TYPE(, FSortCS, TEXT("/MyShadersShaders/FindIndex.usf"), TEXT("MainComputeShader"), SF_Compute);



void MatrixCS_Manager::Init(Eigen::MatrixXf& featureDataBase)
{
	vitualNum = ((featureDataBase.rows()/10)+1)*10;
	trueNum = featureDataBase.rows();
	FRHICommandListImmediate& RHICommands = GRHICommandList.GetImmediateCommandList();
	{
		TResourceArray<float> FeatureDataBaseResourceArray;
		FeatureDataBaseResourceArray.Init(9999, vitualNum*featureDataBase.cols());
		for(int i = 0;i<featureDataBase.rows();i++)
		{
			for(int j = 0; j<featureDataBase.cols();j++)
			{
				FeatureDataBaseResourceArray[i * featureDataBase.cols()+ j] = featureDataBase(i,j);
			}
		}
		FRHIResourceCreateInfo createInfo;
		createInfo.ResourceArray = &FeatureDataBaseResourceArray;

		_featureDataBuffer = RHICreateStructuredBuffer(sizeof(float), sizeof(float) * FeatureDataBaseResourceArray.Num(), BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
		_featureDataBufferUAV = RHICreateUnorderedAccessView(_featureDataBuffer, false, false);
	}

	{
		TResourceArray<float> CostResourceArray;
		CostResourceArray.Init(0, vitualNum);

		FRHIResourceCreateInfo createInfo;
		createInfo.ResourceArray = &CostResourceArray;

		_CostArrayBuffer = RHICreateStructuredBuffer(sizeof(float), sizeof(float) * CostResourceArray.Num(), BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
		_CostArrayBufferUAV = RHICreateUnorderedAccessView(_CostArrayBuffer, false, false);
	}

	{
		TResourceArray<float> IndexArray;
		IndexArray.Init(0, 1);

		FRHIResourceCreateInfo createInfo;
		createInfo.ResourceArray = &IndexArray;
		
		_WinnerIndexBuffer = RHICreateStructuredBuffer(sizeof(float), sizeof(float), BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
		_WinnerIndexBufferUAV = RHICreateUnorderedAccessView(_WinnerIndexBuffer, false, false);
	}
	

	if(CostArray.Num()!=vitualNum)
		CostArray.Init(0,vitualNum);
	resultIndex.Init(0,1);
	
}

void MatrixCS_Manager::Compute(FRHICommandListImmediate& RHICommands,const Eigen::VectorXf& featrueVector,const float Responsiveness, 
							const float VelocityStrength, 
							const float PoseStrength )
{
	auto t1 = std::chrono::system_clock::now();
	// Caculate Cost Array
	TShaderMapRef<FMatirxCS> cs(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	FRHIComputeShader* rhiComputeShader = cs.GetComputeShader();
	{
		TResourceArray<float> VectorArray;
		VectorArray.Init(0, featrueVector.rows());
		for(int i=0;i<featrueVector.rows();i++)
			VectorArray[i] = featrueVector(i);

		FRHIResourceCreateInfo createInfo;
		createInfo.ResourceArray = &VectorArray;
		
		_featureVectorBuffer = RHICreateStructuredBuffer(sizeof(float), sizeof(float) * VectorArray.Num(), BUF_UnorderedAccess | BUF_ShaderResource, createInfo);
		_featureVectorBufferUAV = RHICreateUnorderedAccessView(_featureVectorBuffer, false, false);
	}
	RHICommands.SetComputeShader(rhiComputeShader);
	RHICommands.SetUAVParameter(rhiComputeShader, cs->featureData.GetBaseIndex(), _featureDataBufferUAV);
	RHICommands.SetUAVParameter(rhiComputeShader, cs->featureVector.GetBaseIndex(), _featureVectorBufferUAV);
	RHICommands.SetUAVParameter(rhiComputeShader, cs->CostArray.GetBaseIndex(), _CostArrayBufferUAV);
	cs->SetUniform(RHICommands,Responsiveness,VelocityStrength,PoseStrength,CostArray.Num(),featrueVector.rows());
	
	DispatchComputeShader(RHICommands, cs, CostArray.Num()/10, 1, 1);
	auto t2 = std::chrono::system_clock::now();
	//float* data = (float*)RHILockStructuredBuffer(_CostArrayBuffer, 0, CostArray.Num()*sizeof(float), RLM_ReadOnly);
	//FMemory::Memcpy(CostArray.GetData(), data, CostArray.Num()*sizeof(float));		
	//RHIUnlockStructuredBuffer(_featureDataBuffer);

	//Find the winner Index
	TShaderMapRef<FSortCS> Scs(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	FRHIComputeShader* rhiSortComputeShader = Scs.GetComputeShader();
	
	RHICommands.SetComputeShader(rhiSortComputeShader);
	RHICommands.SetUAVParameter(rhiSortComputeShader, Scs->CostArray.GetBaseIndex(), _CostArrayBufferUAV);
	RHICommands.SetUAVParameter(rhiSortComputeShader, Scs->Index.GetBaseIndex(), _WinnerIndexBufferUAV);
	Scs->SetUniform(RHICommands,Responsiveness,VelocityStrength,PoseStrength,CostArray.Num(),featrueVector.rows());

	

	DispatchComputeShader(RHICommands, Scs, 1, 1, 1);
	auto t3 = std::chrono::system_clock::now();
	
	float* Indexdata = (float*)RHILockStructuredBuffer(_WinnerIndexBuffer, 0, sizeof(float), RLM_ReadOnly);
	FMemory::Memcpy(resultIndex.GetData(), Indexdata, sizeof(float));

	auto t4 = std::chrono::system_clock::now();

	RHIUnlockStructuredBuffer(_WinnerIndexBuffer);

	this->WinIndex = resultIndex[0];
	
	auto t2t1 = t2 - t1;
	auto t3t2 = t3 - t2;
	auto t4t3 = t4 - t3;

	auto total = t4 - t1;

	int i = 0;
	
}
