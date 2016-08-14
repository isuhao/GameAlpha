// Fill out your copyright notice in the Description page of Project Settings.
#include "GameAlpha.h"
#include "SpawnCubeActor.h"
#include "FunctionalCubeActor.h"


// Sets default values
ASpawnCubeActor::ASpawnCubeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_pShape = NewObject<USphereComponent>(this, TEXT("SphereMesh"));
	RootComponent = m_pShape;

}

// Called when the game starts or when spawned
void ASpawnCubeActor::BeginPlay()
{
	Super::BeginPlay();
	noise::module::Perlin myModule;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(myModule);
	heightMapBuilder.SetDestNoiseMap(m_HeightMap);
	heightMapBuilder.SetDestSize(m_Width, m_Length);
	heightMapBuilder.SetBounds(2.0, 6.0, 1.0, 5.0);
	heightMapBuilder.Build();
	m_curSpawnIndex = 0;
}

uint32 ASpawnCubeActor::GetGridHeight(int x, int y)
{
	float fNoise = m_HeightMap.GetValue(x, y);
	fNoise = fmin(fNoise, 1.0f);
	fNoise = fmax(fNoise, -1.0f);
	return uint32((fNoise + 1.0f) * 0.5 * m_Height);
}

// Called every frame
void ASpawnCubeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (uint32 cnt = 0; cnt < m_SpawnNumPerTick; cnt++)
	{
		if (m_curSpawnIndex >= m_Width * m_Length)
			return;
		FVector curLoc = m_pShape->GetComponentTransform().GetLocation();
		uint32 x = m_curSpawnIndex / m_Length;
		uint32 y = m_curSpawnIndex % m_Length;
		uint32 h = GetGridHeight(x, y);
		for (uint32 i = 0; i < h; i++)
		{
			if (GetGridHeight(x - 1, y) >= i &&
				GetGridHeight(x + 1, y) >= i &&
				GetGridHeight(x, y - 1) >= i &&
				GetGridHeight(x, y + 1) >= i)
				continue;
			FVector loc = curLoc + FVector(x * 100, y * 100, i * 100);
			this->SpawnCube(loc);
		}
		FVector loc = curLoc + FVector(x * 100, y * 100, h * 100);
		this->SpawnCube(loc);
		m_curSpawnIndex++;
	}
}

void ASpawnCubeActor::SpawnCube(const FVector& location)
{
	UWorld* pWorld = GetWorld();
	if (!pWorld)
		return;
	if (m_SpawnType == NULL)
		return;
	FActorSpawnParameters spawnParam;
	spawnParam.Owner = this;
	spawnParam.Instigator = this->Instigator;
	FRotator rotator;
	rotator.Roll = 0;
	rotator.Pitch = 0;
	rotator.Yaw = 0;
	pWorld->SpawnActor<AFunctionalCubeActor>(m_SpawnType, location, rotator, spawnParam);
}