// Portfolio excerpt: Echo / Hand GrabThrow behavior
// 担当範囲: 召喚された手が近くの敵を掴み、召喚者方向へ投げる挙動

#pragma once

#include "CoreMinimal.h"
#include "../BaseActor/BasePawn.h"
#include "Hand.generated.h"

/**
 * 手の行動モード
 *
 * Echoの実行データで確認できる主担当範囲はGrabThrow。
 * Interceptは拡張用として残しているが、本ポートフォリオでは主説明から外している。
 */
UENUM(BlueprintType)
enum class EHandTurretMode : uint8
{
	Intercept,
	GrabThrow,
};

/**
 * GrabThrowの進行状態
 */
UENUM(BlueprintType)
enum class EHandGrabState : uint8
{
	None,
	WaitingGrab,
	Holding,
	Throwing,
	Finished,
};

/**
 * @brief 召喚された手Actor
 *
 * GrabThrowモードでは、既存のBattleMgrから近くの敵を取得し、
 * 敵の足元へ移動した後、一定時間保持して召喚者方向へ投げ飛ばす。
 *
 * BP_Hand側ではMontage再生や手のひら位置調整を行い、
 * C++側ではターゲット選択、保持、回頭、Velocity付与による投げ処理を担当する。
 */
UCLASS()
class HAZIKI_API AHand : public ABasePawn
{
	GENERATED_BODY()

public:
	AHand();

protected:
	// ==================================================
	// Engine Override
	// ==================================================

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ==================================================
	// Public Interface
	// ==================================================

	/** 召喚後にAbility側から呼ばれ、手の初期モードを設定する */
	UFUNCTION(BlueprintCallable, Category = "Turret")
	void InitHand(EHandTurretMode InMode);

	/** 既存の敵探索リストから最も近い敵を取得する */
	UFUNCTION(BlueprintCallable, Category = "Turret|Target")
	ABasePawn* FindNearestEnemy() const;

private:
	// ==================================================
	// Intercept Extension
	// ==================================================

	/** Intercept用の拡張フック。実行データではGrabThrowを主に使用するため未実装。 */
	void TryStartAttack();

	// ==================================================
	// GrabThrow
	// ==================================================

	/** GrabThrow中の保持・回頭更新 */
	void UpdateGrabThrow(float DeltaTime);

	/** ターゲットを掴み状態へ移行し、投げタイマーを開始する */
	void StartGrabEnemy();

	/** 掴んだ敵を召喚者方向へ投げ飛ばす */
	UFUNCTION(BlueprintCallable, Category = "Turret|GrabThrow")
	void ThrowGrabbedEnemy();

protected:
	// ==================================================
	// GrabThrow Parameter
	// ==================================================

	/** BP側で設定する掴み位置。手のひら位置に合わせるために使用する。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FVector m_GrabPoint = FVector::ZeroVector;

	/** Intercept拡張用。GrabThrowのみ見せる場合はfalseでもよい。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	bool m_bEnableInterceptAttack = true;

	/** 掴んでから投げるまでの保持時間 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrabThrow")
	float m_GrabHoldTime = 0.6f;

	/** 投げる強さの補正値 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrabThrow")
	float m_ThrowPower = 1.0f;

	/** 投げ方向に対する角度補正 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GrabThrow")
	float m_ThrowAngle = 0.0f;

private:
	// ==================================================
	// Timer
	// ==================================================

	FTimerHandle m_LifeTimerHandle;
	FTimerHandle m_AttackIntervalTimerHandle;
	FTimerHandle m_GrabStartTimerHandle;
	FTimerHandle m_GrabHoldTimerHandle;

	// ==================================================
	// Runtime State
	// ==================================================

	/** GrabThrow対象の敵 */
	UPROPERTY(Transient)
	TObjectPtr<AActor> m_TargetActor = nullptr;

	/** 現在の手の行動モード */
	EHandTurretMode m_TurretMode = EHandTurretMode::Intercept;

	/** GrabThrowの進行状態 */
	EHandGrabState m_GrabState = EHandGrabState::None;
};
