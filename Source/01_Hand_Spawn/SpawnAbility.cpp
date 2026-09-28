// Portfolio excerpt: Echo / Hand summon ability
// 担当範囲: 手の召喚位置決定、召喚Actor生成、Owner登録

#include "SpawnAbility.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

// ==================================================
// Ability Flow
// ==================================================

void USpawnAbility::AbilityMain()
{
	m_SpawnedActor = nullptr;
	m_bHasCachedSpawnTransform = false;

	// 入力中に決めた視点方向から召喚位置をキャッシュする。
	// 失敗した場合は、設定次第でプレイヤー基準のフォールバック位置を使用する。
	if (m_bUseViewTargetSpawn)
	{
		m_bHasCachedSpawnTransform = MakeViewTargetSpawnTransform(m_CachedSpawnTransform);
	}

	if (!m_bHasCachedSpawnTransform && m_bSpawnStartLoc)
	{
		m_CachedSpawnTransform = MakeFallbackSpawnTransform();
		m_bHasCachedSpawnTransform = true;
	}
}

void USpawnAbility::CompleteMontage()
{
	if (!m_SpawnActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Ability開始時に位置を確定できなかった場合は、Montage完了時点でフォールバック位置を使う。
	if (!m_bHasCachedSpawnTransform)
	{
		m_CachedSpawnTransform = MakeFallbackSpawnTransform();
		m_bHasCachedSpawnTransform = true;
	}

	FActorSpawnParameters SpawnParams;
	if (m_bRegistOwner)
	{
		SpawnParams.Owner = GetOwningActorFromActorInfo();
	}

	m_SpawnedActor = World->SpawnActor<AActor>(m_SpawnActor, m_CachedSpawnTransform, SpawnParams);
}

// ==================================================
// Spawn Location
// ==================================================

bool USpawnAbility::MakeViewTargetSpawnTransform(FTransform& OutTransform) const
{
	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerPawn)
	{
		return false;
	}

	AController* Controller = OwnerPawn->GetController();
	if (!Controller)
	{
		return false;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * m_TraceDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HandSpawnViewTrace), false);
	QueryParams.AddIgnoredActor(OwnerActor);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		m_TraceChannel,
		QueryParams
	);

	if (!bHit)
	{
		return false;
	}

	FVector SpawnLocation = HitResult.ImpactPoint;
	SpawnLocation.Z += m_SpawnHeightOffset;

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	FVector OwnerToSpawn = SpawnLocation - OwnerLocation;
	OwnerToSpawn.Z = 0.0f;

	const float Distance = OwnerToSpawn.Size();
	if (Distance > m_MaxSpawnDistance)
	{
		const FVector LimitedDirection = OwnerToSpawn.GetSafeNormal();
		SpawnLocation = OwnerLocation + LimitedDirection * m_MaxSpawnDistance;

		// 距離制限後も、視点Traceで取得した高さを基準にして手の出現高さを保つ。
		SpawnLocation.Z = HitResult.ImpactPoint.Z + m_SpawnHeightOffset;
	}

	FRotator SpawnRotation = OwnerActor->GetActorRotation();
	SpawnRotation.Pitch = 0.0f;
	SpawnRotation.Roll = 0.0f;

	OutTransform.SetLocation(SpawnLocation);
	OutTransform.SetRotation(SpawnRotation.Quaternion());
	OutTransform.SetScale3D(FVector::OneVector);

	return true;
}

FTransform USpawnAbility::MakeFallbackSpawnTransform() const
{
	FTransform SpawnTransform;
	SpawnTransform.SetScale3D(FVector::OneVector);

	AActor* OwnerActor = GetOwningActorFromActorInfo();
	if (!OwnerActor)
	{
		return SpawnTransform;
	}

	FVector SpawnLocation = OwnerActor->GetActorLocation();
	SpawnLocation += OwnerActor->GetActorForwardVector() * m_SpawnLoc.X;
	SpawnLocation += OwnerActor->GetActorRightVector() * m_SpawnLoc.Y;
	SpawnLocation += OwnerActor->GetActorUpVector() * m_SpawnLoc.Z;

	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(OwnerActor->GetActorQuat());

	return SpawnTransform;
}
