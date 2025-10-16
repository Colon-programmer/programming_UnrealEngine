// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "SessionSubsystem.generated.h"

/**
 * Online Subsystem �� IOnlineSession �𑋌��ɁACreate/Find/Join ���s��
 */
USTRUCT(BlueprintType)
struct FFoundSessionRow
{
    GENERATED_BODY();

    UPROPERTY(BlueprintReadOnly) FString DisplayName; // �����̕\�����i�I�[�i�[���Ȃǁj
    UPROPERTY(BlueprintReadOnly) int32   PingMs = 0;
    UPROPERTY(BlueprintReadOnly) int32   OpenConnections = 0;
    UPROPERTY(BlueprintReadOnly) int32   MaxConnections = 0;

    // ���ۂ� Join ����Ƃ��Ɏg�������C���f�b�N�X�iUI ����߂������j
    int32 SearchIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateFinished, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindFinished, const TArray<FFoundSessionRow>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinFinished, bool, bSuccess);

/**
 * 
 */
UCLASS()
class GP3_UEFPS_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Session")
    void CreateLanSession(int32 PublicConnections = 3);

    UFUNCTION(BlueprintCallable, Category = "Session")
    void FindLanSessions(int32 MaxResults = 50);

    UFUNCTION(BlueprintCallable, Category = "Session")
    void JoinBySearchIndex(int32 SearchIndex);

    // UI�ƌq���₷���悤�ɃC�x���g��
    UPROPERTY(BlueprintAssignable) FOnCreateFinished OnCreateFinished;
    UPROPERTY(BlueprintAssignable) FOnFindFinished   OnFindFinished;
    UPROPERTY(BlueprintAssignable) FOnJoinFinished   OnJoinFinished;

private:
    // Online Subsystem �擾
    bool EnsureOnline();

    // �f���Q�[�g�n���h���Ǘ��i�����Y��h�~�j
    void ClearDelegates();

    // Create �̑O�Ɋ����Z�b�V����������Ή�
    void DestroyThenRecreate(int32 PublicConnections);

    // Online �C���^�t�F�[�X
    IOnlineSubsystem* OSS = nullptr;
    IOnlineSessionPtr  Session;

    // �������ʁiUI�\���p�ɐ��`�j
    TSharedPtr<FOnlineSessionSearch> LastSearch;
    TArray<FFoundSessionRow>         LastRows;

    // ���ɑJ�ڂ���}�b�v���iCreate ������� ?listen �ŊJ���j
    FString NextTravelMap;

    // �f���Q�[�g
    FDelegateHandle OnCreateHandle, OnStartHandle, OnDestroyHandle, OnFindHandle, OnJoinHandle;

    // �R�[���o�b�N
    void OnCreateComplete(FName SessionName, bool bOk);
    void OnStartComplete(FName SessionName, bool bOk);
    void OnDestroyComplete(FName SessionName, bool bOk);
    void OnFindComplete(bool bOk);
    void OnJoinComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

};
