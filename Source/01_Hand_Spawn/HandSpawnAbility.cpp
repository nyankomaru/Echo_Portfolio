// Portfolio excerpt: Echo / Hand summon ability
// 担当範囲: 召喚された手Actorへの初期モード設定

#include "HandSpawnAbility.h"

// ==================================================
// Ability Flow
// ==================================================

void UHandSpawnAbility::CompleteMontage()
{
	Super::CompleteMontage();

	AHand* SpawnedHand = Cast<AHand>(m_SpawnedActor);
	if (!IsValid(SpawnedHand))
	{
		return;
	}

	// 生成されたBP_Handに、GrabThrowなどの初期挙動モードを渡す。
	SpawnedHand->InitHand(m_InitMode);
}
