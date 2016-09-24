// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "ChunkRenderComponent.h"
#include "ChunkMgrComponent.h"
#include "LocalVertexFactory.h"

struct FBrickVertex
{
	uint8 x;
	uint8 y;
	uint8 z;
	FBrickVertex(const FInt3& coordinate) :
		x(coordinate.X), y(coordinate.Y), z(coordinate.Z)
	{}
};


class FBrickVertexBuffer : public FVertexBuffer
{
public:
	TArray<FBrickVertex> Vertices;
	
	virtual void InitRHI() override
	{
		if (Vertices.Num() > 0)
		{
			FRHIResourceCreateInfo createInfo;
			VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FBrickVertex), BUF_Dynamic, createInfo);
			
			void* vertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FBrickVertex), RLM_WriteOnly);
			FMemory::Memcpy(vertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FBrickVertex));
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}
};

class FBrickIndexBuffer : public FIndexBuffer
{
public:
	TArray<uint16> Indices;
	
	virtual void InitRHI() override
	{
		if (Indices.Num())
		{
			FRHIResourceCreateInfo createInfo;
			IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), Indices.Num() * sizeof(uint16), BUF_Static, createInfo);

			void* indexBufferData = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(uint16), RLM_WriteOnly);
			FMemory::Memcpy(indexBufferData, Indices.GetData(), Indices.Num() * sizeof(uint16));
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}
};



class FBrickVertexFactory : public FLocalVertexFactory
{
public:
	void Init(const FBrickVertexBuffer& vertexBuffer, uint8 faceIndex)
	{
		DataType dataType;
		dataType.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FBrickVertex, x, VET_UByte4N);
		dataType.TextureCoordinates.Add(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FBrickVertex, x, VET_UByte4N));
		dataType.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FBrickVertex, x, VET_UByte4N);
		dataType.TangentBasisComponents[0];
		dataType.TangentBasisComponents[1];
		check(!IsInRenderingThread());
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(VertexFactorySetData, 
			FBrickVertexFactory*, vertexFactory, this, 
			DataType, dataType, dataType,
		{
			vertexFactory->SetData(dataType);
		})
	}
};


class FBrickChunkProxy: public FPrimitiveSceneProxy
{
public:
	FBrickVertexBuffer VertexBuffer;

	FBrickIndexBuffer IndexBuffer;

	FBrickVertexFactory VertexFactory;

	FGraphEventRef SetupCompleteEvent;

	FMaterialRelevance MaterialRelevance;

	FBrickChunkProxy(UChunkRenderComponent* pComponent)
		: FPrimitiveSceneProxy(pComponent)
	{}
	virtual ~FBrickChunkProxy()
	{}

	void BeginInitResources()
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(WaitForSetupCompleteEvent, FGraphEventRef, SetupCompleteEvent, SetupCompleteEvent, {
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(SetupCompleteEvent, ENamedThreads::RenderThread);
		});
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		
	}
	
	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

	virtual void OnTransformChanged() override
	{

	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
	{

	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{

	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = View->Family->EngineShowFlags.Wireframe || IsSelected();
		Result.bStaticRelevance = !Result.bDynamicRelevance;
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

};


FPrimitiveSceneProxy* UChunkRenderComponent::CreateSceneProxy()
{
	FBrickChunkProxy *pProxy = NULL;
	pProxy = new FBrickChunkProxy(this);
	pProxy->SetupCompleteEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([=]() {
		int a = 1;
	}, TStatId(), NULL);
	return pProxy;
}

FBoxSphereBounds UChunkRenderComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	return FBoxSphereBounds();
}
