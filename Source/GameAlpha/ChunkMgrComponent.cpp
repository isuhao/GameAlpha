// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "ChunkMgrComponent.h"
#include "ChunkRenderComponent.h"

// Sets default values for this component's properties
UChunkMgrComponent::UChunkMgrComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UChunkMgrComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UChunkMgrComponent::Update(const FVector& WorldViewPosition)
{
	FVector localViewPosition = GetComponentTransform().InverseTransformPosition(WorldViewPosition);
	FInt3 minCoordinate = FInt3::Max(GridParameters.MinCoordinate, FInt3::Floor(localViewPosition - FVector(GridParameters.MaxRenderDistance)));
	FInt3 maxCoordinate = FInt3::Min(GridParameters.MaxCoordinate, FInt3::Ceil(localViewPosition + FVector(GridParameters.MaxRenderDistance)));
	FInt3 minChunkIndex = minCoordinate / GridParameters.GridPerChunk;
	FInt3 maxChunkIndex = maxCoordinate / GridParameters.GridPerChunk;

	//删除超出视距的块
	for (auto chunkIt = Coord2ChunkRenderComponent.CreateIterator(); chunkIt; ++chunkIt)
	{
		FInt3 coord = chunkIt.Key();
		if (coord.X * coord.X + coord.Y * coord.Y >= GridParameters.MaxRenderDistance * GridParameters.MaxRenderDistance)
		{
			UChunkRenderComponent* comp = chunkIt.Value();
			comp->DetachFromParent();
			comp->DestroyComponent();
			chunkIt.RemoveCurrent();
		}
	}
	//新进入视距的块加进场景
	for (int x = minChunkIndex.X; x <= maxChunkIndex.X; x++)
	{
		for (int y = minChunkIndex.Y; y <= maxChunkIndex.Y; y++)
		{
			FInt3 coord = FInt3(x, y, 0) * GridParameters.GridPerChunk;
			if (coord.X * coord.X + coord.Y * coord.Y >= GridParameters.MaxRenderDistance * GridParameters.MaxRenderDistance)
				continue;
			UChunkRenderComponent* comp = Coord2ChunkRenderComponent.FindRef(coord);
			if (!comp)
			{
				comp = NewObject<UChunkRenderComponent>(GetOwner());
				comp->Mgr = this;
				comp->Coordinate = coord;
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

