// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "ChunkRenderComponent.h"
#include "ChunkMgrComponent.h"
#include "LocalVertexFactory.h"

enum EGridMaterialType
{
	EGMT_Empty,
	EGMT_Translucent,
	EGMT_Opaque,
	EGMT_Count,
};

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
	TArray<EGridMaterialType> materialType;
	for (int8 index = 0; index < this->Mgr->GridParameters.GridMaterials.Num(); ++ index)
	{
		if (index == this->Mgr->GridParameters.EmptyMaterialIndex)
			materialType.Add(EGMT_Empty);
		else
		{
			if (this->Mgr->GridParameters.GridMaterials[index].SurfaceMaterial->GetBlendMode() == BLEND_Translucent)
				materialType.Add(EGMT_Translucent);
			else
				materialType.Add(EGMT_Opaque);
		}
	}
	FChunkGridData& chunkData = this->Mgr->Coord2ChunkData.FindRef(this->Coordinate);
	bool hasNotEmptyGrid = false;
	for (uint32 index = 0; index < chunkData.GridMaterialIndex.Num(); ++ index)
	{
		if (materialType[chunkData.GridMaterialIndex[index]] != EGMT_Empty)
			hasNotEmptyGrid = true;
	}
	if (hasNotEmptyGrid == false)
		return NULL;
	FBrickChunkProxy *pProxy = NULL;
	pProxy = new FBrickChunkProxy(this);
	pProxy->SetupCompleteEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([=]() {

		FInt3 minCoordinate = FInt3::Max(this->Mgr->GridParameters.MinCoordinate, this->Coordinate);
		FInt3 maxCoordinate = FInt3::Min(this->Mgr->GridParameters.MaxCoordinate, this->Coordinate + this->Mgr->GridParameters.GridPerChunk);
		for (int32 x = minCoordinate.X; x < maxCoordinate.X; ++x)
		{
			for (int32 y = minCoordinate.Y; y < maxCoordinate.Y; ++y)
			{
				for (int32 z = minCoordinate.Z; z < maxCoordinate.Z; ++z)
				{
					FInt3 gridPos = FInt3(x, y, z);

				}
			}
		}

	}, TStatId(), NULL);
	return pProxy;
}

FBoxSphereBounds UChunkRenderComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	return FBoxSphereBounds();
}
