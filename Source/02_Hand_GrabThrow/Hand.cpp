// Portfolio excerpt: Echo / Hand GrabThrow behavior
// 担当範囲: 召喚された手が近くの敵を掴み、召喚者方向へ投げる挙動

#include "Hand.h"

#include "../MyComponents/BattleMgrComp.h"
#include "../MyComponents/MyPawnMoveComp.h"

// ==================================================
// Constructor / Initialize
// ==================================================

AHand::AHand()
{
}

void AHand::BeginPlay()
{
	Super::BeginPlay();
}

// ==================================================
// Tick / Main Update
// ==================================================

void AHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 実行データで確認できる主担当挙動。
	// GrabThrowでは、掴み位置への保持と召喚者方向への回頭を更新する。
	if (m_TurretMode == EHandTurretMode::GrabThrow)
	{
		UpdateGrabThrow(DeltaTime);
		return;
	}

	// Interceptは拡張用。今回のポートフォリオではGrabThrowに絞って説明する。
	if (!m_bEnableInterceptAttack)
	{
		return;
	}

	TryStartAttack();
}

// ==================================================
// Target Search
// ==================================================

ABasePawn* AHand::FindNearestEnemy() const
{
	if (!m_BattleMgr)
	{
		return nullptr;
	}

	ABasePawn* NearestEnemy = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();

	const FVector HandLocation = GetActorLocation();
	const TArray<ABasePawn*> Enemies = m_BattleMgr->GetSearchedActors();

	for (ABasePawn* Enemy : Enemies)
	{
		if (!IsValid(Enemy))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(
			HandLocation,
			Enemy->GetActorLocation()
		);

		if (DistanceSquared < NearestDistanceSquared)
		{
			NearestDistanceSquared = DistanceSquared;
			NearestEnemy = Enemy;
		}
	}

	return NearestEnemy;
}

// ==================================================
// Hand Init
// ==================================================

void AHand::InitHand(EHandTurretMode InMode)
{
	m_TurretMode = InMode;
	m_GrabState = EHandGrabState::None;
	m_TargetActor = nullptr;

	if (m_BattleMgr)
	{
		// 召喚時点の周辺敵情報を更新してからターゲットを決める。
		m_BattleMgr->ReSearch();
	}

	m_TargetActor = FindNearestEnemy();

	if (m_TurretMode != EHandTurretMode::GrabThrow)
	{
		return;
	}

	if (!IsValid(m_TargetActor))
	{
		m_GrabState = EHandGrabState::Finished;
		return;
	}

	// GrabThrowでは、対象の足元へ手を移動してから掴み処理を開始する。
	FHitResult HitResult;
	const FVector TargetLocation = m_TargetActor->GetActorLocation();

	const bool bHitGround = GetWorld() && GetWorld()->LineTraceSingleByProfile(
		HitResult,
		TargetLocation,
		TargetLocation + FVector(0.0f, 0.0f, -500.0f),
		FName("CheckFallRay")
	);

	if (bHitGround)
	{
		SetActorLocation(HitResult.Location);
	}
	else
	{
		// 地面取得に失敗した場合でも、ターゲット付近に手を出して挙動確認できるようにする。
		SetActorLocation(TargetLocation);
	}

	StartGrabEnemy();
}

// ==================================================
// Intercept Extension
// ==================================================

void AHand::TryStartAttack()
{
	// Intercept用の拡張フック。
	// 実行データで確認できる担当範囲はGrabThrowのため、ここでは処理を持たせていない。
}

// ==================================================
// GrabThrow
// ==================================================

void AHand::UpdateGrabThrow(float DeltaTime)
{
	// 現状では時間経過そのものはTimerで管理している。
	// Tickでは掴み中の位置保持と向き合わせのみを行う。
	(void)DeltaTime;

	if (m_GrabState != EHandGrabState::Holding)
	{
		return;
	}

	if (!IsValid(m_TargetActor))
	{
		m_GrabState = EHandGrabState::Finished;
		Destroy();
		return;
	}

	// BP側で更新した手のひら位置へ敵を保持する。
	m_TargetActor->SetActorLocation(m_GrabPoint);

	AActor* Summoner = GetOwner();
	if (IsValid(Summoner) && m_MoveComp)
	{
		m_MoveComp->RotateTo(Summoner->GetActorLocation() - GetActorLocation());
	}
}

void AHand::StartGrabEnemy()
{
	if (!IsValid(m_TargetActor))
	{
		m_GrabState = EHandGrabState::Finished;
		return;
	}

	m_GrabState = EHandGrabState::Holding;

	GetWorldTimerManager().ClearTimer(m_GrabHoldTimerHandle);
	GetWorldTimerManager().SetTimer(
		m_GrabHoldTimerHandle,
		this,
		&AHand::ThrowGrabbedEnemy,
		m_GrabHoldTime,
		false
	);
}

void AHand::ThrowGrabbedEnemy()
{
	AActor* Summoner = GetOwner();
	if (!IsValid(Summoner) || !IsValid(m_TargetActor))
	{
		m_GrabState = EHandGrabState::Finished;
		Destroy();
		return;
	}

	m_GrabState = EHandGrabState::Throwing;

	const FVector TargetLocation = Summoner->GetActorLocation();
	const FVector StartLocation = m_TargetActor->GetActorLocation();

	FVector ThrowVector = TargetLocation - StartLocation;
	if (ThrowVector.IsNearlyZero())
	{
		m_GrabState = EHandGrabState::Finished;
		Destroy();
		return;
	}

	ThrowVector *= m_ThrowPower;

	// 投げ方向に対して横軸を作り、角度補正をかける。
	const FVector ThrowLeftVector = FVector::CrossProduct(
		ThrowVector.GetSafeNormal(),
		FVector::UpVector
	);

	ThrowVector = ThrowVector.RotateAngleAxis(m_ThrowAngle, ThrowLeftVector);

	if (ABasePawn* GrabbedPawn = Cast<ABasePawn>(m_TargetActor))
	{
		if (GrabbedPawn->GetMovementComp())
		{
			GrabbedPawn->GetMovementComp()->Velocity += ThrowVector;
		}
	}

	m_GrabState = EHandGrabState::Finished;
	Destroy();
}
