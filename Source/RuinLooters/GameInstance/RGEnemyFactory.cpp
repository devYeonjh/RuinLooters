// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/RGEnemyFactory.h"
#include "Character/RGCharacterEnemy.h"
#include "Engine/World.h"
#include "SaveGame/RGSaveGame.h"
#include "GameInstance/RGGameInstance.h"
#include "Kismet/GameplayStatics.h"

ARGEnemyFactory::ARGEnemyFactory()
{
}

void ARGEnemyFactory::BeginPlay()
{
    Super::BeginPlay();

    // 월드, 게임모드 찾기
    World = GetWorld();
    GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));

    SaveGame = GameInstance->LoadSaveGameData();
    SpawnEnemy();
}

// 스폰
 void ARGEnemyFactory::SpawnEnemy()
{
    if (!SaveGame || !EnemyClass) return;

    FActorSpawnParameters SpawnParams;
    // 항상 스폰으로 설정
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 인스턴스 스폰
    ARGCharacterEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ARGCharacterEnemy>(EnemyClass, GetActorTransform(), SpawnParams);
    if (!SpawnedEnemy) return;

    // 스테이지에 따른 적 선택
    int32 StageIndex = SaveGame->StageIndex;
    FString BaseName = SpawnedEnemy->GetEnemyName().ToString();
    FString NewName = BaseName + FString::FromInt(StageIndex);
    SpawnedEnemy->SetEnemyName(FName(*NewName));

    // 적 스폰
    SpawnedEnemy->EnemyAbilityRow = GameInstance->GetEnemyAbilityInformation(SpawnedEnemy->GetEnemyName());
    SpawnedEnemy->GetMesh()->SetSkeletalMesh(SpawnedEnemy->EnemyAbilityRow->SkeletalMesh);

    // 적 스탯 설정
    SpawnedEnemy->SetMoney(SpawnedEnemy->EnemyAbilityRow->EnemyMoney);
    SpawnedEnemy->SetCurrentHp(SpawnedEnemy->EnemyAbilityRow->EnemyCurrentHp);
    SpawnedEnemy->SetMaxHp(SpawnedEnemy->EnemyAbilityRow->EnemyMaxHp);
    SpawnedEnemy->SetAttackDamage(SpawnedEnemy->EnemyAbilityRow->EnemyAttackDamage);
    SpawnedEnemy->SetDefence(SpawnedEnemy->EnemyAbilityRow->EnemyDefence);
    SpawnedEnemy->SetRowWeapon(GameInstance->GetWeaponInformation(SpawnedEnemy->EnemyAbilityRow->EnemyWeaponName));

    // 캐릭터 무기 장착
    if (SpawnedEnemy->GetCharacterWeaponRow())
    {
        SpawnedEnemy->ChangeWeapon(SpawnedEnemy->GetCharacterWeaponRow());

        SpawnedEnemy->GetCharacterWeaponMeshComponent()->SetSkeletalMesh(SpawnedEnemy->GetCharacterWeaponRow()->SkeletalMesh);
        UE_LOG(LogTemp, Warning, TEXT("RowWeapon :%s"), *SpawnedEnemy->GetCharacterWeaponName().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Enemy RowWeapon Null"));
    }
}
