// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "BrickRenderComponent.h"


class FBrickVertexBuffer : public FVertexBuffer
{
public:
	FBrickVertexBuffer()
	{

	}
};

class FBrickIndexBuffer : public FIndexBuffer
{
public:
	FBrickIndexBuffer()
	{

	}
};

class FBrickVertexFactory : public FVertexFactory
{
public:
	FBrickVertexFactory()
	{

	}
};

class FBrickSceneProxy: public FPrimitiveSceneProxy
{
public:
	FBrickSceneProxy(UBrickRenderComponent* pComponent)
		: FPrimitiveSceneProxy(pComponent)
	{}
	virtual ~FBrickSceneProxy()
	{}

	void BeginInitResources()
	{

	}
};


FPrimitiveSceneProxy* UBrickRenderComponent::CreateSceneProxy()
{
	return NULL;
}

FBoxSphereBounds UBrickRenderComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	return FBoxSphereBounds();
}
