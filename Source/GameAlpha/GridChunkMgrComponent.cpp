// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "GridChunkMgrComponent.h"
#include "GridChunkRenderComponent.h"

FChunkGridData::FChunkGridData(const FInt3& coord, const FGridParam& param)
{
	noise::module::Perlin myModule;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(myModule);
	heightMapBuilder.SetDestNoiseMap(m_HeightMap);
	FInt3 bound = param.GridPerChunk + FInt3::Scalar(1);
	heightMapBuilder.SetDestSize(bound.X, bound.Y);
	heightMapBuilder.SetBounds(coord.X, coord.X + bound.X, coord.Y, coord.Y + bound.Y);
	heightMapBuilder.Build();
	for (int32 i = 0; i < param.GridPerChunk.X + 1; ++i)
	{
		for (int32 j = 0; j < param.GridPerChunk.Y + 1; ++j)
		{
			FInt3 gridPos = coord + FInt3(i, j, 0);
			float fNoise = m_HeightMap.GetValue(gridPos.X, gridPos.Y);
			fNoise = fmin(fNoise, 1.0f);
			fNoise = fmax(fNoise, -1.0f);
			int32 height = int32((fNoise + 1.0f) * 0.5 * param.MaxHeight);
			for (int32 k = 0; k < param.GridPerChunk.Z + 1; ++k)
			{
				int32 curGridHeight = coord.Z + k;
				if (curGridHeight < height)
					GridMaterialIndex.Add(1);
				else
					GridMaterialIndex.Add(0);
			}

		}
	}
}


// Sets default values for this component's properties
UGridChunkMgrComponent::UGridChunkMgrComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UGridChunkMgrComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UGridChunkMgrComponent::Update(const FVector& WorldViewPosition)
{
	//FVector localViewPosition = GetComponentTransform().InverseTransformPosition(WorldViewPosition);
	FVector localViewPosition = FVector(0, 0, 0);
	FInt3 minCoordinate = FInt3::Max(GridParameters.MinCoordinate, FInt3::Floor(localViewPosition - FVector(GridParameters.MaxRenderDistance)));
	FInt3 maxCoordinate = FInt3::Min(GridParameters.MaxCoordinate, FInt3::Ceil(localViewPosition + FVector(GridParameters.MaxRenderDistance)));
	if (minCoordinate.X < 0)
		minCoordinate.X -= GridParameters.GridPerChunk.X;
	if (minCoordinate.Y < 0)
		minCoordinate.Y -= GridParameters.GridPerChunk.Y;
	if (minCoordinate.Z < 0)
		minCoordinate.Z -= GridParameters.GridPerChunk.Z;
	if (maxCoordinate.X < 0)
		maxCoordinate.X -= GridParameters.GridPerChunk.X;
	if (maxCoordinate.Y < 0)
		maxCoordinate.Y -= GridParameters.GridPerChunk.Y;
	if (maxCoordinate.Z < 0)
		maxCoordinate.Z -= GridParameters.GridPerChunk.Z;
	FInt3 minChunkIndex = minCoordinate / GridParameters.GridPerChunk;
	FInt3 maxChunkIndex = maxCoordinate / GridParameters.GridPerChunk;


	//删除超出视距的块
	for (auto chunkIt = Coord2ChunkRenderComponent.CreateIterator(); chunkIt; ++chunkIt)
	{
		FInt3 coord = chunkIt.Key();
		FInt3 center = coord + GridParameters.GridPerChunk / FInt3::Scalar(2);
		if (center.X * center.X + center.Y * center.Y >= GridParameters.MaxRenderDistance * GridParameters.MaxRenderDistance)
		{
			UGridChunkRenderComponent* comp = chunkIt.Value();
			comp->DetachFromParent();
			comp->DestroyComponent();
			chunkIt.RemoveCurrent();
		}
	}
	//新进入视距的块加进场景
	for (int x = minChunkIndex.X; x <= maxChunkIndex.X; ++x)
	{
		for (int y = minChunkIndex.Y ; y <= maxChunkIndex.Y; ++y)
		{
			FInt3 coord = FInt3(x, y, 0) * GridParameters.GridPerChunk;
			FInt3 center = coord + GridParameters.GridPerChunk / FInt3::Scalar(2);
			if (center.X * center.X + center.Y * center.Y >= GridParameters.MaxRenderDistance * GridParameters.MaxRenderDistance)
				continue;
			FChunkGridData* data = this->Coord2ChunkData.Find(coord);
			if (!data)
			{
				this->Coord2ChunkData.Add(coord, FChunkGridData(coord, this->GridParameters));
			}

			UGridChunkRenderComponent* comp = Coord2ChunkRenderComponent.FindRef(coord);
			if (!comp)
			{
				comp = NewObject<UGridChunkRenderComponent>(GetOwner());
				comp->Mgr = this;
				comp->Init(coord);
				comp->SetRelativeLocation(coord.ToFloat());
				comp->AttachTo(this);
				comp->RegisterComponent();
				Coord2ChunkRenderComponent.Add(coord, comp);

			}
			else
			{
				comp->MarkRenderStateDirty();
			}

		}
	}
}
