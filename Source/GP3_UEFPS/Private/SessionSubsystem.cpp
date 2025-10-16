// Fill out your copyright notice in the Description page of Project Settings.
#include "SessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

// UE側で一般的に使われる固定名（"GameSession"）
static const FName SESSION_NAME = NAME_GameSession;

bool USessionSubsystem::EnsureOnline()
{
    // Online Subsystem を取得（Null: LAN / Steam: Steam / EOS: Epic などプラットフォーム別に切替）
    if (!OSS) OSS = IOnlineSubsystem::Get();
    if (!OSS) { UE_LOG(LogTemp, Error, TEXT("No OnlineSubsystem")); return false; }
    if (!Session.IsValid()) Session = OSS->GetSessionInterface();
    if (!Session.IsValid()) { UE_LOG(LogTemp, Error, TEXT("No SessionInterface")); return false; }
    return true;
}

void USessionSubsystem::ClearDelegates()
{
    if (!Session.IsValid()) return;
    if (OnCreateHandle.IsValid())  Session->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateHandle);
    if (OnDestroyHandle.IsValid()) Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroyHandle);
    OnCreateHandle.Reset();
    OnDestroyHandle.Reset();
}

void USessionSubsystem::DestroyThenRecreate(int32 PublicConnections)
{
    ClearDelegates();
    OnDestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateWeakLambda(this, [this, PublicConnections](FName, bool)
            {
                CreateLanSession(PublicConnections);
            }));
    Session->DestroySession(SESSION_NAME);
}

void USessionSubsystem::OnCreateComplete(FName, bool bOk)
{
    // セッション開始（内部状態を「スタート」に）
    Session->StartSession(SESSION_NAME);

    UKismetSystemLibrary::PrintString(this, "OnCreateComplete: Success!!",
        true, true, FColor::Cyan, 4.f, TEXT("None"));
}

void USessionSubsystem::CreateLanSession(int32 PublicConnections)
{
    if (!EnsureOnline()) return;

    // 既に同名セッションが残っていたら破棄してから再作成
    if (Session->GetNamedSession(SESSION_NAME))
    {
        DestroyThenRecreate(PublicConnections);
        return;
    }

    // === セッション設定 ===
    FOnlineSessionSettings Settings;
    Settings.bIsLANMatch = true;  // LAN検索対象にする（Null Subsystem 前提）
    Settings.bShouldAdvertise = true;  // Find に出す
    Settings.bAllowJoinInProgress = true;  // 途中参加OK
    Settings.bUsesPresence = false; // Null/LANなら不要
    Settings.bUseLobbiesIfAvailable = false; // Null/LANではロビー機能は使わない
    Settings.NumPublicConnections = FMath::Max(1, PublicConnections); // 参加枠（ホスト除く枠数でOK）

    // コールバック登録
    OnCreateHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnCreateComplete));

    // 実行（UserIndex=0でOK。複数LocalPlayerがある場合は切替）
    const bool bIssued = Session->CreateSession(/*LocalUserNum=*/0, SESSION_NAME, Settings);
    if (!bIssued)
    {
        UKismetSystemLibrary::PrintString(this, "CreateSession: immediate failure",
            true, true, FColor::Red, 4.f, TEXT("None"));
    }

    void USessionSubsystem::FindLanSessions(int32 MaxResults)
    {
        if (!EnsureOnline()) return;

        // 検索条件を作る
        LastSearch = MakeShared<FOnlineSessionSearch>();
        LastSearch->bIsLanQuery = true;               // LAN に限定
        LastSearch->MaxSearchResults = FMath::Clamp(MaxResults, 1, 2000);
        LastSearch->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);

        ClearDelegates();
        OnFindHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnFindComplete));

        const bool bIssued = Session->FindSessions(/*LocalUserNum=*/0, LastSearch.ToSharedRef());
        if (!bIssued)
        {
            UE_LOG(LogTemp, Error, TEXT("FindSessions: immediate failure"));
            LastRows.Reset();
            OnFindFinished.Broadcast(LastRows);
        }
    }

    void USessionSubsystem::OnFindComplete(bool bOk)
    {
        LastRows.Reset();

        if (bOk && LastSearch.IsValid())
        {
            int32 Index = 0;
            for (const auto& R : LastSearch->SearchResults)
            {
                FFoundSessionRow Row;
                Row.DisplayName = R.Session.OwningUserName; // 表示名（UIの部屋名に使う）
                Row.PingMs = R.PingInMs;
                Row.OpenConnections = R.Session.NumOpenPublicConnections;
                Row.MaxConnections = R.Session.SessionSettings.NumPublicConnections;
                Row.SearchIndex = Index++;
                LastRows.Add(Row);
            }
        }

        OnFindFinished.Broadcast(LastRows);
        ClearDelegates();

        UKismetSystemLibrary::PrintString(this, "OnFindComplete: Success!! :" + Row.DisplayName,
            true, true, FColor::Yellow, 4.f, TEXT("None"));
    }

}