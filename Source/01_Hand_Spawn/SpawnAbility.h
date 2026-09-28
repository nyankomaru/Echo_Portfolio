// Portfolio excerpt: Echo / Hand summon ability
// 担当範囲: 手の召喚位置決定、召喚Actor生成、Owner登録

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityBase.h"
#include "SpawnAbility.generated.h"

/**
 * @brief 召喚Actorの生成位置を決定し、Montage完了時にActorを生成するAbility基底クラス
 *
 * 本クラスでは、視点方向へのLineTraceを用いた召喚位置指定と、
 * 指定に失敗した場合のプレイヤー基準フォールバック位置生成を担当する。
 *
 * Echoでは、十字キー上入力で手の召喚位置を指定し、
 * 入力解除後のMontage完了タイミングでBP_Handを生成するために使用した。
 */
UCLASS()
class HAZIKI_API USpawnAbility : public UGameplayAbilityBase
{
	GENERATED_BODY()

protected:
	// ==================================================
	// Ability Flow
	// ==================================================

	/** アビリティ実行時に召喚位置を事前計算する */
	virtual void AbilityMain() override;

	/** Montage完了時にキャッシュ済み位置へActorを生成する */
	virtual void CompleteMontage() override;

private:
	// ==================================================
	// Spawn Location
	// ==================================================

	/** 視点方向へのLineTraceから召喚Transformを作成する */
	bool MakeViewTargetSpawnTransform(FTransform& OutTransform) const;

	/** 視点指定に失敗した場合に、プレイヤー基準の相対位置から召喚Transformを作成する */
	FTransform MakeFallbackSpawnTransform() const;

private:
	// ==================================================
	// Runtime State
	// ==================================================

	/** Ability開始時に決定した召喚Transform */
	FTransform m_CachedSpawnTransform;

	/** 召喚Transformが正常にキャッシュされているか */
	bool m_bHasCachedSpawnTransform = false;

protected:
	// ==================================================
	// Spawn Parameter
	// ==================================================

	/** 生成するActorクラス */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AActor> m_SpawnActor;

	/** 視点指定に失敗した場合の相対召喚位置 */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector m_SpawnLoc = FVector(300.0f, 0.0f, 0.0f);

	/** trueの場合、視点方向へのLineTraceで召喚位置を決める */
	UPROPERTY(EditAnywhere, Category = "Spawn|ViewTarget")
	bool m_bUseViewTargetSpawn = true;

	/** 視点方向に召喚位置を探す最大距離 */
	UPROPERTY(EditAnywhere, Category = "Spawn|ViewTarget")
	float m_TraceDistance = 2500.0f;

	/** プレイヤーから召喚できる最大距離 */
	UPROPERTY(EditAnywhere, Category = "Spawn|ViewTarget")
	float m_MaxSpawnDistance = 1200.0f;

	/** 地面から浮かせたい場合の高さ補正 */
	UPROPERTY(EditAnywhere, Category = "Spawn|ViewTarget")
	float m_SpawnHeightOffset = 0.0f;

	/** LineTraceに使用するチャンネル */
	UPROPERTY(EditAnywhere, Category = "Spawn|ViewTarget")
	TEnumAsByte<ECollisionChannel> m_TraceChannel = ECC_Visibility;

	/** trueの場合、生成ActorのOwnerにAbilityの所有Actorを設定する */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool m_bRegistOwner = false;

	/** trueの場合、Ability開始時点の位置を使用して召喚する */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool m_bSpawnStartLoc = true;

protected:
	// ==================================================
	// Spawn Result
	// ==================================================

	/** Montage完了時に生成されたActor */
	UPROPERTY(Transient)
	TObjectPtr<AActor> m_SpawnedActor = nullptr;
};
