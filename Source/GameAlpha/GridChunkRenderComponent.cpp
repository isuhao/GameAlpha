// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "GridChunkRenderComponent.h"
#include "GridChunkMgrComponent.h"
#include "LocalVertexFactory.h"
#include "CoreUObject.h"
#include "Engine.h"

enum EGridMaterialType
{
	EGMT_Empty,
	EGMT_Translucent,
	EGMT_Opaque,
	EGMT_Count,
};

//获取立方体格子各个顶点的偏移
FInt3 GetGridCornerOffset(uint8 cornerIndex)
{
	return (FInt3::Scalar(cornerIndex) >> FInt3(2, 1, 0)) & FInt3::Scalar(1);
}

//获取顶点相邻的格子的偏移
FInt3 GetVertexAdjGridOffset(uint8 gridIndex)
{
	return ((FInt3::Scalar(gridIndex) >> FInt3(2, 1, 0)) & FInt3::Scalar(1)) + FInt3::Scalar(-1);
}

//获取立方体格子某个面的某个顶点的偏移
uint8 GetFaceCornerIndex(uint8 faceIndex, uint8 cornerIndex)
{
	//TODO
	static uint8 faceCornerIndex[6][4] = {
		{4, 5, 7, 6},	//+x
		{0, 2, 3, 1},	//-x	
		{2, 6, 7, 3},	//+y
		{0, 1, 5, 4},	//-y
		{1, 3, 7, 5},	//+z
		{0, 4, 6, 2},	//-z
	};
	return faceCornerIndex[faceIndex][cornerIndex];
}

//获取立方体格子相邻的格子的偏移
FInt3 GetGridAdjGridOffset(uint8 faceIndex)
{
	static FInt3 gridOffset[6] = {
		FInt3(1, 0, 0),		//+x
		FInt3(-1, 0, 0),	//-x
		FInt3(0, 1, 0),		//+y
		FInt3(0, -1, 0),	//-y
		FInt3(0, 0, 1),		//+z
		FInt3(0, 0, -1),	//-z
	};
	return gridOffset[faceIndex];
}

FInt3 GetFaceNormal(uint8 faceIndex)
{
	static FInt3 FaceNormal[6] = {
		FInt3(1, 0, 0),	//+x
		FInt3(-1, 0, 0),	//-x
		FInt3(0, 1, 0),	//+y
		FInt3(0, -1, 0),	//-y
		FInt3(0, 0, 1),	//+z
		FInt3(0, 0, -1)	//-z
	};
	return FaceNormal[faceIndex];
}

struct FGridVertex
{
	uint8 X;
	uint8 Y;
	uint8 Z;
	uint8 AmbientOcclusionFactor;
	FGridVertex(const FInt3& coordinate) :
		X(coordinate.X), Y(coordinate.Y), Z(coordinate.Z)
	{}
};


class FGridVertexBuffer : public FVertexBuffer
{
public:
	TArray<FGridVertex> Vertices;
	
	virtual void InitRHI() override
	{
		if (Vertices.Num() > 0)
		{
			FRHIResourceCreateInfo createInfo;
			VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FGridVertex), BUF_Dynamic, createInfo);
			
			void* vertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FGridVertex), RLM_WriteOnly);
			FMemory::Memcpy(vertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FGridVertex));
			RHIUnlockVertexBuffer(VertexBufferRHI);
		}
	}
};

class FGridIndexBuffer : public FIndexBuffer
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

class FGridVertexTangentBuffer : public FVertexBuffer
{
	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo createInfo;
		VertexBufferRHI = RHICreateVertexBuffer(6 * 2 * sizeof(FPackedNormal), BUF_Dynamic, createInfo);
		FPackedNormal* vertexTangentData = (FPackedNormal*)RHILockVertexBuffer(VertexBufferRHI, 0, 6 * 2 * sizeof(FPackedNormal), RLM_WriteOnly);
		for (int32 i = 0; i < 6; i++)
		{
			FVector normal = GetFaceNormal(i).ToFloat().GetSafeNormal();
			FVector unProjectTangentT = FVector(1, -1, 0).GetSafeNormal();
			FVector projectTangentT = (unProjectTangentT - normal * unProjectTangentT * normal).GetSafeNormal();
			FVector projectTangentB = (normal ^ projectTangentT).GetSafeNormal();
			*vertexTangentData++ = projectTangentT;
			*vertexTangentData++ = projectTangentB;
		}
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};
TGlobalResource<FGridVertexTangentBuffer> TangentBuffer;

class FGridVertexFactory : public FLocalVertexFactory
{
public:
	void Init(const FGridVertexBuffer& vertexBuffer, uint8 faceIndex)
	{
		DataType dataType;
		dataType.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FGridVertex, X, VET_UByte4N);
		dataType.TextureCoordinates.Add(STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FGridVertex, X, VET_UByte4N));
		dataType.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(&vertexBuffer, FGridVertex, X, VET_Color);
		dataType.TangentBasisComponents[0] = FVertexStreamComponent(&TangentBuffer, sizeof(FGridVertexTangentBuffer) * (2 * faceIndex + 0), 0, VET_PackedNormal);
		dataType.TangentBasisComponents[1] = FVertexStreamComponent(&TangentBuffer, sizeof(FGridVertexTangentBuffer) * (2 * faceIndex + 1), 0, VET_PackedNormal);
		check(!IsInRenderingThread());
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(VertexFactorySetData, 
			FGridVertexFactory*, vertexFactory, this, 
			DataType, dataType, dataType,
		{
			vertexFactory->SetData(dataType);
		})
	}
};

struct FFaceBatch
{
	TArray<uint16> Indices;
};

struct FMaterialBatch
{
	FFaceBatch FaceBatches[6];
};

class FGridChunkProxy: public FPrimitiveSceneProxy
{
public:
	FGridVertexBuffer VertexBuffer;

	FGridIndexBuffer IndexBuffer;

	FGridVertexFactory VertexFactory[6];

	FGraphEventRef SetupCompleteEvent;

	FMaterialRelevance MaterialRelevance;

	TArray<UMaterialInterface*> Materails;

	FColoredMaterialRenderProxy WireframeRenderProxy;

	struct FElement
	{
		uint16 FirstIndex;
		uint32 PrimitiveNum;
		uint16 MaterialIndex;
		uint8 FaceIndex;
	};
	TArray<FElement> Elements;

	TUniformBufferRef<FPrimitiveUniformShaderParameters> PrimitiveUniformBuffer;

	FGridChunkProxy(UGridChunkRenderComponent* pComponent):
		FPrimitiveSceneProxy(pComponent),
		WireframeRenderProxy(
			WITH_EDITOR ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0.0, 0.5, 1.0)
		)
	{}
	virtual ~FGridChunkProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		for (int32 i = 0; i < 6; i++)
			VertexFactory[i].ReleaseResource();
	}

	void BeginInitResources()
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(WaitForSetupCompleteEvent, FGraphEventRef, SetupCompleteEvent, SetupCompleteEvent, {
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(SetupCompleteEvent, ENamedThreads::RenderThread);
		});
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		for (int32 i = 0; i < 6; ++i)
		{
			VertexFactory[i].Init(VertexBuffer, i);
			BeginInitResource(&VertexFactory[i]);
		}
	}
	
	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

	virtual void OnTransformChanged() override
	{
		// Create a uniform buffer with the transform for the chunk.
		PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(FScaleMatrix(FVector(255, 255, 255)) * GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
	{
		for (int32 i = 0; i < Elements.Num(); ++i)
		{
			FMeshBatch& meshBatch = Collector.AllocateMesh();
			InitMeshBatch(meshBatch, Elements[i], ViewFamily.EngineShowFlags.Wireframe ? &WireframeRenderProxy : NULL);
			for (int32 viewIndex = 0; viewIndex < Views.Num(); ++viewIndex)
				if (VisibilityMap & (1 << viewIndex))
					Collector.AddMesh(viewIndex, meshBatch);
		}

	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{
		for (int32 i = 0; i < Elements.Num(); ++ i)
		{
			FMeshBatch meshBatch;
			InitMeshBatch(meshBatch, Elements[i], NULL);
			PDI->DrawMesh(meshBatch, FLT_MAX);
		}
	}

	void InitMeshBatch(FMeshBatch& MeshBatch, const FElement& Elem, const FMaterialRenderProxy* WireframeProxy) const
	{
		MeshBatch.bWireframe = WireframeProxy != NULL;
		MeshBatch.CastShadow = true;
		MeshBatch.VertexFactory = &VertexFactory[Elem.FaceIndex];
		MeshBatch.MaterialRenderProxy = WireframeProxy != NULL ? WireframeProxy : Materails[Elem.MaterialIndex]->GetRenderProxy(IsSelected());
		MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
		MeshBatch.Type = PT_TriangleList;
		MeshBatch.DepthPriorityGroup = SDPG_World;
		MeshBatch.Elements[0].NumPrimitives = Elem.PrimitiveNum;
		MeshBatch.Elements[0].FirstIndex = Elem.FirstIndex;
		MeshBatch.Elements[0].UserIndex = Elem.FaceIndex;
		MeshBatch.Elements[0].IndexBuffer = &IndexBuffer;
		MeshBatch.Elements[0].MinVertexIndex = 0;
		MeshBatch.Elements[0].MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
		MeshBatch.Elements[0].PrimitiveUniformBuffer = PrimitiveUniformBuffer;
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

uint16 UGridChunkRenderComponent::GetMaterialIndex(FInt3 coord)
{
	const FInt3& gridPerChunk = this->Mgr->GridParameters.GridPerChunk;
	FInt3 chunkCoord = coord;
	if (chunkCoord.X < 0)
		chunkCoord.X -= gridPerChunk.X;
	if (chunkCoord.Y < 0)
		chunkCoord.Y -= gridPerChunk.Y;
	if (chunkCoord.Z < 0)
		chunkCoord.Z -= gridPerChunk.Z;
	chunkCoord = chunkCoord / gridPerChunk * gridPerChunk;
	FChunkGridData* data = this->Mgr->Coord2ChunkData.Find(chunkCoord);
	if (!data)
	{
		return 0;
	}
	FInt3 offset = coord - chunkCoord;
	return data->GridMaterialIndex[(offset.X * (gridPerChunk.Y + 1) + offset.Y) * (gridPerChunk.Z + 1) + offset.Z];
}


void UGridChunkRenderComponent::Init(const FInt3& cood)
{
	this->Coordinate = cood;
}

FPrimitiveSceneProxy* UGridChunkRenderComponent::CreateSceneProxy()
{	
	TArray<EGridMaterialType> materialType;
	for (int16 index = 0; index < this->Mgr->GridParameters.GridMaterials.Num(); ++ index)
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
	FChunkGridData* chunkData = this->Mgr->Coord2ChunkData.Find(this->Coordinate);

	FInt3 minCoordinate = FInt3::Max(this->Mgr->GridParameters.MinCoordinate, this->Coordinate);
	FInt3 maxCoordinate = FInt3::Min(this->Mgr->GridParameters.MaxCoordinate, this->Coordinate + this->Mgr->GridParameters.GridPerChunk);
	FInt3 chunkSize = maxCoordinate - minCoordinate;

	//如果当前都是空白的格子，则不渲染
	bool hasNotEmptyGrid = false;
	for (int32 x = minCoordinate.X; x < maxCoordinate.X; ++x)
	{
		for (int32 y = minCoordinate.Y; y < maxCoordinate.Y; ++y)
		{
			for (int32 z = minCoordinate.Z; z < maxCoordinate.Z; ++z)
			{
				FInt3 gridPos = FInt3(x, y, z);
				uint16 matrialIndex = GetMaterialIndex(gridPos);
				if (materialType[matrialIndex] != EGMT_Empty)
					hasNotEmptyGrid = true;
			}
		}
	}

// 	if (chunkData)
// 		for (int32 index = 0; index < chunkData->GridMaterialIndex.Num(); ++ index)
// 			if (materialType[chunkData->GridMaterialIndex[index]] != EGMT_Empty)
// 				hasNotEmptyGrid = true;
	if (hasNotEmptyGrid == false)
		return NULL;
	const ERHIFeatureLevel::Type SceneFeatureLevel = GetScene()->GetFeatureLevel();

	//多线程
	FGridChunkProxy *pProxy = NULL;
	pProxy = new FGridChunkProxy(this);
	pProxy->SetupCompleteEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([=]() {
		TArray<uint16> indexBuffer;
		indexBuffer.Empty(chunkSize.X * chunkSize.Y * chunkSize.Z);
		//对于每个点，遍历相领的八个格子，看该点是否需要加到顶点buffer
		for (int32 x = 0; x <= maxCoordinate.X - minCoordinate.X; ++x)
		{
			for (int32 y = 0; y <= maxCoordinate.Y - minCoordinate.Y; ++y)
			{
				for (int32 z = 0; z <= maxCoordinate.Z - minCoordinate.Z; ++z)
				{
					FInt3 vertexPos = FInt3(x, y, z);
					bool hasMatrialType[EGMT_Count] = { false };
					for (int32 i = 0; i < 8; ++i)
					{
						FInt3 gridPos = minCoordinate + vertexPos + GetVertexAdjGridOffset(i);
						uint16 matrialIndex = GetMaterialIndex(gridPos);
						hasMatrialType[materialType[matrialIndex]] = true;
					}
					//如果相邻的八个格子有一种以上种类的材质，则该顶点需要显示
					uint8 typeCnt = 0;
					for (int32 i = 0; i < EGMT_Count; i++)
						if (hasMatrialType[i])
							++typeCnt;
					if (typeCnt > 1)
					{
						indexBuffer.Add(pProxy->VertexBuffer.Vertices.Num());
						new (pProxy->VertexBuffer.Vertices) FGridVertex(vertexPos);
					}
					else
					{
						indexBuffer.Add(0);
					}
				}
			}
		}
		TArray<FMaterialBatch> MaterialBatches;
		MaterialBatches.Init(FMaterialBatch(), this->Mgr->GridParameters.GridMaterials.Num());
		//对于每个格子，遍历其六个面，看该面是否需要显示
		for (int32 x = minCoordinate.X; x < maxCoordinate.X; ++x)
		{
			for (int32 y = minCoordinate.Y; y < maxCoordinate.Y; y++)
			{
				for (int32 z = minCoordinate.Z; z < maxCoordinate.Z; ++z)
				{
					FInt3 gridPos = FInt3(x, y, z);
					uint16 materialIndex = GetMaterialIndex(gridPos);
					for (int32 i = 0; i < 6; ++i)
					{
						FInt3 adjGridPos = gridPos + GetGridAdjGridOffset(i);
						uint16 adjMaterialIndex = GetMaterialIndex(adjGridPos);

						//如果当前格子比这个面的相邻格子更加的不透明，则这个面需要渲染
						if (materialType[materialIndex] > materialType[adjMaterialIndex])
						{
							//当前面的四个顶点
							uint16 faceCornerIndices[4];
							for (int32 j = 0; j < 4; ++j)
							{
								FInt3 faceCornerPos = gridPos + GetGridCornerOffset(GetFaceCornerIndex(i, j));
								FInt3 offset = faceCornerPos - minCoordinate;
								faceCornerIndices[j] = indexBuffer[(offset.X * (chunkSize.Y + 1) + offset.Y) * (chunkSize.Z + 1) + offset.Z];
							}
							FFaceBatch& faceBatch = MaterialBatches[materialIndex].FaceBatches[i];
							uint16* indices = &faceBatch.Indices[faceBatch.Indices.AddUninitialized(6)];
							*(indices++) = faceCornerIndices[0];
							*(indices++) = faceCornerIndices[1];
							*(indices++) = faceCornerIndices[2];
							*(indices++) = faceCornerIndices[0];
							*(indices++) = faceCornerIndices[2];
							*(indices++) = faceCornerIndices[3];
						}
					}
				}
			}
		}
		uint32 IndexNum = 0;
		for (int32 i = 0; i < MaterialBatches.Num(); i++)
			for (int32 j = 0; j < 6; j++)
				IndexNum += MaterialBatches[i].FaceBatches[j].Indices.Num();
		pProxy->IndexBuffer.Indices.Empty(IndexNum);
		for (int32 i = 0; i < MaterialBatches.Num(); i++)
		{
			const FGridMaterial& gridMaterial = this->Mgr->GridParameters.GridMaterials[i];
			UMaterialInterface* surfaceMaterial = gridMaterial.SurfaceMaterial;
			if (!surfaceMaterial)
				surfaceMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
			pProxy->MaterialRelevance |= surfaceMaterial->GetRelevance_Concurrent(SceneFeatureLevel);
			uint16 sufaceMaterialIndex = pProxy->Materails.Add(surfaceMaterial);

			UMaterialInterface* topMaterial = this->Mgr->GridParameters.GridMaterials[i].TopSurfaceMaterial;
			if (!topMaterial)
				topMaterial = this->Mgr->GridParameters.GridMaterials[i].SurfaceMaterial;
			else
				pProxy->MaterialRelevance |= topMaterial->GetRelevance_Concurrent(SceneFeatureLevel);

			uint16 topMaterialIndex = sufaceMaterialIndex;
			if (topMaterial != surfaceMaterial)
				topMaterialIndex = pProxy->Materails.Add(topMaterial);

			for (int32 j = 0; j < 6; j++)
			{
				FFaceBatch &batch = MaterialBatches[i].FaceBatches[j];
				if (batch.Indices.Num() <= 0)
					continue;
				FGridChunkProxy::FElement& newElem = *new(pProxy->Elements)FGridChunkProxy::FElement;
				newElem.FirstIndex = pProxy->IndexBuffer.Indices.Num();
				newElem.PrimitiveNum = batch.Indices.Num() / 3;
				// 2 是正上面
				newElem.MaterialIndex = j == 4 ? topMaterialIndex : sufaceMaterialIndex;
				newElem.FaceIndex = j;
				pProxy->IndexBuffer.Indices.Append(batch.Indices);
			}
		}

	}, TStatId(), NULL);
	pProxy->BeginInitResources();
	return pProxy;
}

FBoxSphereBounds UGridChunkRenderComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	FBoxSphereBounds NewBounds;
	NewBounds.Origin = NewBounds.BoxExtent = this->Mgr->GridParameters.GridPerChunk.ToFloat() / 2.0f;
	NewBounds.SphereRadius = NewBounds.BoxExtent.Size();
	return NewBounds.TransformBy(LocalToWorld);
}
