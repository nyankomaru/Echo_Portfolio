// Portfolio excerpt: Echo / Hand summon ability
// 担当範囲: 召喚された手Actorへの初期モード設定

#pragma once

#include "CoreMinimal.h"
#include "SpawnAbility.h"
#include "../Actors/Hand.h"
#include "HandSpawnAbility.generated.h"

/**
 * @brief 手専用の召喚Ability
 *
 * 共通召喚処理でBP_Handを生成した後、生成された手Actorへ初期モードを渡す。
 * Echoでは、GrabThrowモードを指定することで、近くの敵を掴んで召喚者方向へ投げる挙動につなげた。
 */
UCLASS()
class HAZIKI_API UHandSpawnAbility : public USpawnAbility
{
	GENERATED_BODY()

protected:
	// ==================================================
	// Ability Flow
	// ==================================================

	/** Montage完了時に手Actorを生成し、初期モードを設定する */
	virtual void CompleteMontage() override;

protected:
	// ==================================================
	// Hand Init Parameter
	// ==================================================

	/** 召喚時に手へ渡す初期モード */
	UPROPERTY(EditAnywhere, Category = "Init")
	EHandTurretMode m_InitMode = EHandTurretMode::GrabThrow;
};
