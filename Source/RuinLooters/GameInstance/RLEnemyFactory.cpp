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

    // ����, ���Ӹ��?ã��
    World = GetWorld();
    GameInstance = Cast<URLGameInstance>(UGameplayStatics::GetGameInstance(World));

    SaveGame = GameInstance->LoadSaveGameData();
    SpawnEnemy();
}

// ����
 void ARLEnemyFactory::SpawnEnemy()
{
    if (!SaveGame || !EnemyClass) return;

    FActorSpawnParameters SpawnParams;
    // �׻� �������� ����
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // �ν��Ͻ� ����
    ARLCharacterEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ARLCharacterEnemy>(EnemyClass, GetActorTransform(), SpawnParams);
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



