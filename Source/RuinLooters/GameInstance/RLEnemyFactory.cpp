// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/RLEnemyFactory.h"
#include "Character/RLCharacterEnemy.h"
#include "Engine/World.h"
#include "SaveGame/RLSaveGame.h"
#include "GameInstance/RLGameInstance.h"
#include "Kismet/GameplayStatics.h"

ARLEnemyFactory::ARLEnemyFactory()
{
}

void ARLEnemyFactory::BeginPlay()
{
    Super::BeginPlay();

    // 월드, 게임인스턴스 찾기
    World = GetWorld();
    GameInstance = Cast<URLGameInstance>(UGameplayStatics::GetGameInstance(World));

    SaveGame = GameInstance->LoadSaveGameData();
    SpawnEnemy();
}

// 생성
 void ARLEnemyFactory::SpawnEnemy()
{
    if (!SaveGame || !EnemyClass) return;

    FActorSpawnParameters SpawnParams;
    // 항상 생성되게 설정
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 인스턴스 생성
    ARLCharacterEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ARLCharacterEnemy>(EnemyClass, GetActorTransform(), SpawnParams);
    if (!SpawnedEnemy) return;

    // 스테이지에 따른 적 설정
    int32 StageIndex = SaveGame->StageIndex;
    FString BaseName = SpawnedEnemy->GetEnemyName().ToString();
    FString NewName = BaseName + FString::FromInt(StageIndex);
    SpawnedEnemy->SetEnemyName(FName(*NewName));

    // 적 설정
    SpawnedEnemy->EnemyAbilityRow = GameInstance->GetEnemyAbilityInformation(SpawnedEnemy->GetEnemyName());
    SpawnedEnemy->GetMesh()->SetSkeletalMesh(SpawnedEnemy->EnemyAbilityRow->SkeletalMesh);

    // 적 스탯 설정
    SpawnedEnemy->SetMoney(SpawnedEnemy->EnemyAbilityRow->EnemyMoney);
    SpawnedEnemy->SetCurrentHp(SpawnedEnemy->EnemyAbilityRow->EnemyCurrentHp);
    SpawnedEnemy->SetMaxHp(SpawnedEnemy->EnemyAbilityRow->EnemyMaxHp);
    SpawnedEnemy->SetAttackDamage(SpawnedEnemy->EnemyAbilityRow->EnemyAttackDamage);
    SpawnedEnemy->SetDefence(SpawnedEnemy->EnemyAbilityRow->EnemyDefence);
    SpawnedEnemy->SetRowWeapon(GameInstance->GetWeaponInformation(SpawnedEnemy->EnemyAbilityRow->EnemyWeaponName));

    // 캐릭터 무기 설정
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



