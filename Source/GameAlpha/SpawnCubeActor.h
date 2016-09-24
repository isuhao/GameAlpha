// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "noiseutils.h"
#include "SpawnCubeActor.generated.h"

UCLASS()
class GAMEALPHA_API ASpawnCubeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnCubeActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

protected:

	uint32 GetGridHeight(int x, int y);

	void SpawnCube(const FVector& location);

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class AFunctionalCubeActor> m_SpawnType;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	uint32 m_Width;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	uint32 m_Length;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	uint32 m_Height;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	uint32 m_SpawnNumPerTick = 1;

	noise::utils::NoiseMap m_HeightMap;

	uint32 m_curSpawnIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "SphereMesh")
	USphereComponent* m_pShape;

};
