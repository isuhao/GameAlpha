// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "noiseutils.h"
#include "GridChunkMgrComponent.generated.h"

/** Shifts a number right with sign extension. */
inline int32 SignedShiftRight(int32 A, int32 B)
{
	// The C standard doesn't define whether shifting a signed integer right will extend the sign bit, but the VC2013 compiler does so.
	// If using a different compiler, assert that it does the same, although it is our problem if it does not.
#if defined(_MSC_VER) && _MSC_VER == 1800
	return A >> B;
#else
	const int32 Result = A >> B;
	if (A < 0)
	{
		check(~(~A >> B) == Result);
	}
	return Result;
#endif
}

/** A 3D integer vector. */
USTRUCT(BlueprintType, Atomic)
struct FInt3
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Coordinates)
		int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Coordinates)
		int32 Y;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Coordinates)
		int32 Z;

	FInt3() {}
	FInt3(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

	static FInt3 Scalar(int32 I)
	{
		return FInt3(I, I, I);
	}

	operator FIntVector() const { return FIntVector(X, Y, Z); }
	FVector ToFloat() const { return FVector(X, Y, Z); }
	int32 SumComponents() const { return X + Y + Z; }

	friend uint32 GetTypeHash(const FInt3& Coordinates)
	{
		return FCrc::MemCrc32(&Coordinates, sizeof(Coordinates));
	}
#define DEFINE_VECTOR_OPERATOR(symbol) \
		friend FInt3 operator symbol(const FInt3& A, const FInt3& B) \
		{ \
			return FInt3(A.X symbol B.X, A.Y symbol B.Y, A.Z symbol B.Z); \
		}
	DEFINE_VECTOR_OPERATOR(+);
	DEFINE_VECTOR_OPERATOR(-);
	DEFINE_VECTOR_OPERATOR(*);
	DEFINE_VECTOR_OPERATOR(/ );
	DEFINE_VECTOR_OPERATOR(&);
	DEFINE_VECTOR_OPERATOR(<< );
	DEFINE_VECTOR_OPERATOR(>> );
	DEFINE_VECTOR_OPERATOR(< );
	DEFINE_VECTOR_OPERATOR(<= );
	DEFINE_VECTOR_OPERATOR(> );
	DEFINE_VECTOR_OPERATOR(>= );
	friend bool operator==(const FInt3& A, const FInt3& B)
	{
		return A.X == B.X && A.Y == B.Y && A.Z == B.Z;
	}
	static inline FInt3 SignedShiftRight(const FInt3& A, const FInt3& B)
	{
		return FInt3(::SignedShiftRight(A.X, B.X), ::SignedShiftRight(A.Y, B.Y), ::SignedShiftRight(A.Z, B.Z));
	}
	static inline FInt3 Exp2(const FInt3& A)
	{
		return FInt3(1 << A.X, 1 << A.Y, 1 << A.Z);
	}
	static inline FInt3 CeilLog2(const FInt3& A)
	{
		return FInt3(FMath::CeilLogTwo(A.X), FMath::CeilLogTwo(A.Y), FMath::CeilLogTwo(A.Z));
	}
	static inline FInt3 Max(const FInt3& A, const FInt3& B)
	{
		return FInt3(FMath::Max(A.X, B.X), FMath::Max(A.Y, B.Y), FMath::Max(A.Z, B.Z));
	}
	static inline FInt3 Min(const FInt3& A, const FInt3& B)
	{
		return FInt3(FMath::Min(A.X, B.X), FMath::Min(A.Y, B.Y), FMath::Min(A.Z, B.Z));
	}
	static inline FInt3 Clamp(const FInt3& A, const FInt3& MinA, const FInt3& MaxA)
	{
		return Min(Max(A, MinA), MaxA);
	}
	static inline FInt3 Floor(const FVector& V)
	{
		return FInt3(FMath::FloorToInt(V.X), FMath::FloorToInt(V.Y), FMath::FloorToInt(V.Z));
	}
	static inline FInt3 Ceil(const FVector& V)
	{
		return FInt3(FMath::CeilToInt(V.X), FMath::CeilToInt(V.Y), FMath::CeilToInt(V.Z));
	}
	static inline bool Any(const FInt3& A)
	{
		return A.X || A.Y || A.Z;
	}
	static inline bool All(const FInt3& A)
	{
		return A.X && A.Y && A.Z;
	}
};

USTRUCT(BlueprintType, Atomic)
struct FGridMaterial
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		UMaterialInterface* SurfaceMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		UMaterialInterface* TopSurfaceMaterial;

	FGridMaterial() :
		SurfaceMaterial(NULL), TopSurfaceMaterial(NULL)
	{}
};

USTRUCT(BlueprintType, Atomic)
struct FGridParam
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		TArray<FGridMaterial> GridMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Material)
		int32 EmptyMaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		int MaxRenderDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		int MaxCollisionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		FInt3 GridPerChunk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		FInt3 MinCoordinate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		FInt3 MaxCoordinate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Param)
		int32 MaxHeight;

};

USTRUCT(BlueprintType)
struct FChunkGridData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		FInt3 Coordinate;

	UPROPERTY()
		TArray<uint8> GridMaterialIndex;

	FChunkGridData() {};

	FChunkGridData(const FInt3& coord, const FGridParam& param);

	noise::utils::NoiseMap m_HeightMap;

};

//根据Component当前所在位置以及视距来决定哪些块需要显示， 每个块里管理着所有要显示的3D格子

//管理所有的块的管理器
UCLASS(hidecategories = (Object, LOD, Physics), Blueprintable, editinlinenew, ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent) )
class GAMEALPHA_API UGridChunkMgrComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGridChunkMgrComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = Chunk)
		void Update(const FVector& WorldViewPosition);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GridParam)
		FGridParam GridParameters;

	TMap<FInt3, class UGridChunkRenderComponent*> Coord2ChunkRenderComponent;

	TMap<FInt3, FChunkGridData> Coord2ChunkData;


};
