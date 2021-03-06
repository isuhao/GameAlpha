// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "FunctionalCubeActor.generated.h"

UCLASS()
class GAMEALPHA_API AFunctionalCubeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFunctionalCubeActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FunCube", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* m_mesh;

};
