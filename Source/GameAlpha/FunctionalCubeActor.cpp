// Fill out your copyright notice in the Description page of Project Settings.

#include "GameAlpha.h"
#include "FunctionalCubeActor.h"


// Sets default values
AFunctionalCubeActor::AFunctionalCubeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	RootComponent = m_mesh;
}

// Called when the game starts or when spawned
void AFunctionalCubeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFunctionalCubeActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

