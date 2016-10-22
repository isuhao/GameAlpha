// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "GridChunkMgrComponent.h"
#include "GridChunkRenderComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMEALPHA_API UGridChunkRenderComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:
	void Init(const FInt3& cood);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	uint16 GetMaterialIndex(FInt3 coord);

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
	class UGridChunkMgrComponent* Mgr;

	//以这个块的左上角为坐标，而不是中心
	FInt3 Coordinate;
};
