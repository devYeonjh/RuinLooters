// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/RGGameInstance.h"
#include "UI/NPCStoreWidget.h"
#include "SaveGame/RGSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RGPlayerDataAsset.h"

URGGameInstance::URGGameInstance() : ContextStr(TEXT("WeaponDataTableLookup"))
{
}

void URGGameInstance::Init()
{
    UGameInstance::Init();

    if (!WeaponDataTable || !PotionDataTable || !SkillBookDataTable || !EnemyAbilityDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable or PotionDataTable are null."));
        return;
    }

    // 모든 Row 이름 조회
    WeaponRowNames = WeaponDataTable->GetRowNames();

    PotionRowNames = PotionDataTable->GetRowNames();

    SkillBookRowNames = SkillBookDataTable->GetRowNames();

    EnemyAbilityRowNames = EnemyAbilityDataTable->GetRowNames();

}

FWeaponTableRow* URGGameInstance::GetWeaponInformation(FName InWeaponName)
{
    // 이름으로 무기 정보 반환받기
    if (FWeaponTableRow* WeaponInformation = WeaponDataTable->FindRow<FWeaponTableRow>(InWeaponName, ContextStr))
    {
        return WeaponInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponInformation is null."));
    return nullptr;
}

FWeaponTableRow* URGGameInstance::GetRamdomWeapon()
{
    // 랜덤 인덱스 반환
    int32 RandomIndex = FMath::RandRange(0, WeaponRowNames.Num() - 1);

    // 랜덤하게 선택된 인덱스의 이름 가져오기
    FName RandomRowName = WeaponRowNames[RandomIndex];

    // 이름으로 데이터 테이블에서 이름으로 열 찾기
    if (FWeaponTableRow* RandomWeapon = WeaponDataTable->FindRow<FWeaponTableRow>(RandomRowName, ContextStr))
    {
        return RandomWeapon;
    }

    // 실패 시 nullptr 반환
    return nullptr;
}

FName URGGameInstance::GetWeaponName(int32 InWeaponIndex)
{
    for (FName RowName : WeaponDataTable->GetRowNames())
    {
        if (const FWeaponTableRow* Row = WeaponDataTable->FindRow<FWeaponTableRow>(RowName, ContextStr))
        {
            if (Row->WeaponIndex == InWeaponIndex)
                return RowName;
        }
    }
    return "null";
}

FPotionTableRow* URGGameInstance::GetPotionInformation(FName InPotionName)
{
    if (FPotionTableRow* PotionInformation = PotionDataTable->FindRow<FPotionTableRow>(InPotionName, ContextStr))
    {
        return PotionInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("PotionInformation is null."));
    return nullptr;
}

FPotionTableRow* URGGameInstance::GetRamdomPotion()
{
    int32 RandomIndex = FMath::RandRange(0, PotionRowNames.Num() - 1);

    FName RandomRowName = PotionRowNames[RandomIndex];

    if (FPotionTableRow* PotionInformation = PotionDataTable->FindRow<FPotionTableRow>(RandomRowName, ContextStr))
    {
        return PotionInformation;
    }

    return nullptr;
}

FName URGGameInstance::GetPotionName(int32 InPotionIndex)
{
    if (PotionRowNames.Num() == 0) return TEXT("Null Potion Row");

    FName PlayerPotionName = PotionRowNames[InPotionIndex];

    return PlayerPotionName;
}

FSkillBookTableRow* URGGameInstance::GetSkillBookInformation(FName InSkillBookName)
{
    if (FSkillBookTableRow* SkillBookInformation = SkillBookDataTable->FindRow<FSkillBookTableRow>(InSkillBookName, ContextStr))
    {
        return SkillBookInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("SkillBookInformation is null."));
    return nullptr;
}

FSkillBookTableRow* URGGameInstance::GetRamdomSkillBook()
{
    int32 RandomIndex = FMath::RandRange(0, SkillBookRowNames.Num() - 1);

    FName RandomRowName = SkillBookRowNames[RandomIndex];

    if (FSkillBookTableRow* SkillBookInformation = SkillBookDataTable->FindRow<FSkillBookTableRow>(RandomRowName, ContextStr))
    {
        return SkillBookInformation;
    }

    return nullptr;
}

FName URGGameInstance::GetSkillBookName(int32 InSkillBookIndex)
{
    if (SkillBookRowNames.Num() == 0) return TEXT("Null SkillBook Row");

    FName PlayerSkillBookName = SkillBookRowNames[InSkillBookIndex];

    return PlayerSkillBookName;
}

FEnemyAbilityTableRow* URGGameInstance::GetEnemyAbilityInformation(FName InEnemyName)
{
    if (FEnemyAbilityTableRow* EnemyAbilityInformation = EnemyAbilityDataTable->FindRow<FEnemyAbilityTableRow>(InEnemyName, ContextStr))
    {
        return EnemyAbilityInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityInformation is null."));
    return nullptr;
}

// SaveGame 저장
void URGGameInstance::SetSaveGame(URGPlayerDataAsset* NewSaveData)
{
    SaveGameInstance = Cast<URGSaveGame>(UGameplayStatics::CreateSaveGameObject(URGSaveGame::StaticClass()));
    SaveGameInstance->SetSaveGameData(NewSaveData);

    // 저장
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->PlayerIndex);
}

// SaveGame 불러오기
URGSaveGame* URGGameInstance::LoadSaveGameData()
{
    // 초기화
    LoadGameInstance = Cast<URGSaveGame>(UGameplayStatics::CreateSaveGameObject(URGSaveGame::StaticClass()));

    // 세이브 파일 찾기
    if (UGameplayStatics::DoesSaveGameExist("PlayerSaveSlot", 0))
    {
        // 있다면 로드
        LoadGameInstance = Cast<URGSaveGame>(UGameplayStatics::LoadGameFromSlot("PlayerSaveSlot", 0));

        if (LoadGameInstance)
            return LoadGameInstance;
    }

    // 세이브 파일 찾기 실패 시
    // Player Stat Asset 불러와 초기화값 저장
    URGPlayerDataAsset* NewPlayerStatAsset = LoadObject<URGPlayerDataAsset>(nullptr, TEXT("/Script/Roguelike123.RGPlayerDataAsset'/Game/Assassin/Blueprint/DA_PlayerStat.DA_PlayerStat'"));
    LoadGameInstance->SetSaveGameData(NewPlayerStatAsset);

    return LoadGameInstance;
}

