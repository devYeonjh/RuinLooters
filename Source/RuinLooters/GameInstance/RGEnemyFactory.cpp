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

    // ����, ���Ӹ�� ã��
    World = GetWorld();
    GameInstance = Cast<URGGameInstance>(UGameplayStatics::GetGameInstance(World));

    SaveGame = GameInstance->LoadSaveGameData();
    SpawnEnemy();
}

// ����
 void ARGEnemyFactory::SpawnEnemy()
{
    if (!SaveGame || !EnemyClass) return;

    FActorSpawnParameters SpawnParams;
    // �׻� �������� ����
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // �ν��Ͻ� ����
    ARGCharacterEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ARGCharacterEnemy>(EnemyClass, GetActorTransform(), SpawnParams);
    if (!SpawnedEnemy) return;

    // ���������� ���� �� ����
    int32 StageIndex = SaveGame->StageIndex;
    FString BaseName = SpawnedEnemy->GetEnemyName().ToString();
    FString NewName = BaseName + FString::FromInt(StageIndex);
    SpawnedEnemy->SetEnemyName(FName(*NewName));

    // �� ����
    SpawnedEnemy->EnemyAbilityRow = GameInstance->GetEnemyAbilityInformation(SpawnedEnemy->GetEnemyName());
    SpawnedEnemy->GetMesh()->SetSkeletalMesh(SpawnedEnemy->EnemyAbilityRow->SkeletalMesh);

    // �� ���� ����
    SpawnedEnemy->SetMoney(SpawnedEnemy->EnemyAbilityRow->EnemyMoney);
    SpawnedEnemy->SetCurrentHp(SpawnedEnemy->EnemyAbilityRow->EnemyCurrentHp);
    SpawnedEnemy->SetMaxHp(SpawnedEnemy->EnemyAbilityRow->EnemyMaxHp);
    SpawnedEnemy->SetAttackDamage(SpawnedEnemy->EnemyAbilityRow->EnemyAttackDamage);
    SpawnedEnemy->SetDefence(SpawnedEnemy->EnemyAbilityRow->EnemyDefence);
    SpawnedEnemy->SetRowWeapon(GameInstance->GetWeaponInformation(SpawnedEnemy->EnemyAbilityRow->EnemyWeaponName));

    // ĳ���� ���� ����
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
