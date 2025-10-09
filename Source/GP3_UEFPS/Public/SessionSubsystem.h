// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "SessionSubsystem.generated.h"

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
private:
    // Online Subsystem 取得
    bool EnsureOnline();
    // デリゲートハンドル管理（解除忘れ防止）
    void ClearDelegates();
    // Create の前に既存セッションがあれば壊す
    void DestroyThenRecreate(int32 PublicConnections);
    // Online インタフェース
    IOnlineSubsystem* OSS = nullptr;
    IOnlineSessionPtr  Session;
    // デリゲート
    FDelegateHandle OnCreateHandle;
    FDelegateHandle OnDestroyHandle;
    // コールバック
    void OnCreateComplete(FName SessionName, bool bOk);

};
