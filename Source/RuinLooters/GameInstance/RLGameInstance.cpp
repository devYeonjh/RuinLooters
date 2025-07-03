// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/RLGameInstance.h"
#include "UI/NPCStoreWidget.h"
#include "SaveGame/RLSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RLPlayerDataAsset.h"

URLGameInstance::URLGameInstance() : ContextStr(TEXT("WeaponDataTableLookup"))
{
}

void URLGameInstance::Init()
{
    UGameInstance::Init();

    if (!WeaponDataTable || !PotionDataTable || !SkillBookDataTable || !EnemyAbilityDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable or PotionDataTable are null."));
        return;
    }

    // ���?Row �̸� ��ȸ
    WeaponRowNames = WeaponDataTable->GetRowNames();

    PotionRowNames = PotionDataTable->GetRowNames();

    SkillBookRowNames = SkillBookDataTable->GetRowNames();

    EnemyAbilityRowNames = EnemyAbilityDataTable->GetRowNames();

}

FWeaponTableRow* URLGameInstance::GetWeaponInformation(FName InWeaponName)
{
    // �̸����� ���� ���� ��ȯ�ޱ�
    if (FWeaponTableRow* WeaponInformation = WeaponDataTable->FindRow<FWeaponTableRow>(InWeaponName, ContextStr))
    {
        return WeaponInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("WeaponInformation is null."));
    return nullptr;
}

FWeaponTableRow* URLGameInstance::GetRamdomWeapon()
{
    // ���� �ε��� ��ȯ
    int32 RandomIndex = FMath::RandRange(0, WeaponRowNames.Num() - 1);

    // �����ϰ� ���õ� �ε����� �̸� ��������
    FName RandomRowName = WeaponRowNames[RandomIndex];

    // �̸����� ������ ���̺����� �̸����� �� ã��
    if (FWeaponTableRow* RandomWeapon = WeaponDataTable->FindRow<FWeaponTableRow>(RandomRowName, ContextStr))
    {
        return RandomWeapon;
    }

    // ���� �� nullptr ��ȯ
    return nullptr;
}

FName URLGameInstance::GetWeaponName(int32 InWeaponIndex)
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

FPotionTableRow* URLGameInstance::GetPotionInformation(FName InPotionName)
{
    if (FPotionTableRow* PotionInformation = PotionDataTable->FindRow<FPotionTableRow>(InPotionName, ContextStr))
    {
        return PotionInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("PotionInformation is null."));
    return nullptr;
}

FPotionTableRow* URLGameInstance::GetRamdomPotion()
{
    int32 RandomIndex = FMath::RandRange(0, PotionRowNames.Num() - 1);

    FName RandomRowName = PotionRowNames[RandomIndex];

    if (FPotionTableRow* PotionInformation = PotionDataTable->FindRow<FPotionTableRow>(RandomRowName, ContextStr))
    {
        return PotionInformation;
    }

    return nullptr;
}

FName URLGameInstance::GetPotionName(int32 InPotionIndex)
{
    if (PotionRowNames.Num() == 0) return TEXT("Null Potion Row");

    FName PlayerPotionName = PotionRowNames[InPotionIndex];

    return PlayerPotionName;
}

FSkillBookTableRow* URLGameInstance::GetSkillBookInformation(FName InSkillBookName)
{
    if (FSkillBookTableRow* SkillBookInformation = SkillBookDataTable->FindRow<FSkillBookTableRow>(InSkillBookName, ContextStr))
    {
        return SkillBookInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("SkillBookInformation is null."));
    return nullptr;
}

FSkillBookTableRow* URLGameInstance::GetRamdomSkillBook()
{
    int32 RandomIndex = FMath::RandRange(0, SkillBookRowNames.Num() - 1);

    FName RandomRowName = SkillBookRowNames[RandomIndex];

    if (FSkillBookTableRow* SkillBookInformation = SkillBookDataTable->FindRow<FSkillBookTableRow>(RandomRowName, ContextStr))
    {
        return SkillBookInformation;
    }

    return nullptr;
}

FName URLGameInstance::GetSkillBookName(int32 InSkillBookIndex)
{
    if (SkillBookRowNames.Num() == 0) return TEXT("Null SkillBook Row");

    FName PlayerSkillBookName = SkillBookRowNames[InSkillBookIndex];

    return PlayerSkillBookName;
}

FEnemyAbilityTableRow* URLGameInstance::GetEnemyAbilityInformation(FName InEnemyName)
{
    if (FEnemyAbilityTableRow* EnemyAbilityInformation = EnemyAbilityDataTable->FindRow<FEnemyAbilityTableRow>(InEnemyName, ContextStr))
    {
        return EnemyAbilityInformation;
    }

    UE_LOG(LogTemp, Warning, TEXT("EnemyAbilityInformation is null."));
    return nullptr;
}

// SaveGame ����
void URLGameInstance::SetSaveGame(URLPlayerDataAsset* NewSaveData)
{
    SaveGameInstance = Cast<URLSaveGame>(UGameplayStatics::CreateSaveGameObject(URLSaveGame::StaticClass()));
    SaveGameInstance->SetSaveGameData(NewSaveData);

    // ����
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->PlayerIndex);
}

// SaveGame �ҷ�����
URLSaveGame* URLGameInstance::LoadSaveGameData()
{
    // �ʱ�ȭ
    LoadGameInstance = Cast<URLSaveGame>(UGameplayStatics::CreateSaveGameObject(URLSaveGame::StaticClass()));

    // ���̺� ���� ã��
    if (UGameplayStatics::DoesSaveGameExist("PlayerSaveSlot", 0))
    {
        // �ִٸ� �ε�
        LoadGameInstance = Cast<URLSaveGame>(UGameplayStatics::LoadGameFromSlot("PlayerSaveSlot", 0));

        if (LoadGameInstance)
            return LoadGameInstance;
    }

    // ���̺� ���� ã�� ���� ��
    // Player Stat Asset �ҷ��� �ʱ�ȭ�� ����
    URLPlayerDataAsset* NewPlayerStatAsset = LoadObject<URLPlayerDataAsset>(nullptr, TEXT("/Script/Roguelike123.RLPlayerDataAsset'/Game/Assassin/Blueprint/DA_PlayerStat.DA_PlayerStat'"));
    LoadGameInstance->SetSaveGameData(NewPlayerStatAsset);

    return LoadGameInstance;
}




