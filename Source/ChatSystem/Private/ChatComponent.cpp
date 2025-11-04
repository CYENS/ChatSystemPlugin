// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChatComponent.h"
#include "ChatSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

UChatComponent::UChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	LastMessageTime = 0.0f;
}

void UChatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache the chat subsystem reference
	ChatSubsystem = GetChatSubsystem();
	
	// Register with the subsystem
	if (ChatSubsystem)
	{
		ChatSubsystem->RegisterChatComponent(this);
	}
}

void UChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// Unregister from the subsystem
	if (ChatSubsystem)
	{
		ChatSubsystem->UnregisterChatComponent(this);
	}
	
	// Clean up any references
	ChatSubsystem = nullptr;
}

void UChatComponent::SendChatMessage(const FString& Content, EChatChannel Channel)
{
	if (Content.IsEmpty())
	{
		return;
	}

	APlayerState* OwningPS = GetOwningPlayerState();
	if (!OwningPS)
	{
		return;
	}

	// Validate locally first
	FString FailureReason;
	if (!ValidateMessageLocally(Content, FailureReason))
	{
		ClientNotifyMessageFailed(FailureReason);
		return;
	}

	// Send to server
	ServerSendMessage(Content, Channel, nullptr);
}

void UChatComponent::SendWhisper(APlayerState* TargetPlayer, const FString& Content)
{
	if (!TargetPlayer || Content.IsEmpty())
	{
		return;
	}

	APlayerState* OwningPS = GetOwningPlayerState();
	if (!OwningPS)
	{
		return;
	}

	// Validate locally first
	FString FailureReason;
	if (!ValidateMessageLocally(Content, FailureReason))
	{
		ClientNotifyMessageFailed(FailureReason);
		return;
	}

	// Send to server with whisper target
	ServerSendMessage(Content, EChatChannel::Whisper, TargetPlayer);
}

void UChatComponent::SendProximityMessage(const FString& Content)
{
	if (Content.IsEmpty())
	{
		return;
	}

	const APlayerState* OwningPS = GetOwningPlayerState();
	if (!OwningPS)
	{
		return;
	}

	// Validate locally first
	FString FailureReason;
	if (!ValidateMessageLocally(Content, FailureReason))
	{
		ClientNotifyMessageFailed(FailureReason);
		return;
	}

	// Send to server
	ServerSendMessage(Content, EChatChannel::Proximity, nullptr);
}

void UChatComponent::ServerSendMessage_Implementation(const FString& Content, EChatChannel Channel, APlayerState* WhisperTarget)
{
	UChatSubsystem* Subsystem = GetChatSubsystem();
	if (!Subsystem)
	{
		ClientNotifyMessageFailed(TEXT("Chat subsystem not available"));
		return;
	}

	APlayerState* OwningPS = GetOwningPlayerState();
	if (!OwningPS)
	{
		ClientNotifyMessageFailed(TEXT("Invalid player state"));
		return;
	}

	// Create the message
	FChatMessage Message(OwningPS, Content, Channel);
	Message.WhisperTarget = WhisperTarget;

	// Let the subsystem handle validation and broadcasting
	FString FailureReason;
	if (!Subsystem->BroadcastMessage(Message, FailureReason))
	{
		ClientNotifyMessageFailed(FailureReason);
	}
}

bool UChatComponent::ServerSendMessage_Validate(const FString& Content, EChatChannel Channel, APlayerState* WhisperTarget)
{
	// Basic validation to prevent malicious clients
	return !Content.IsEmpty() && Content.Len() <= 1024;
}

void UChatComponent::ClientReceiveMessage_Implementation(const FChatMessage& Message)
{
	// Check if sender is muted
	if (Message.Sender && IsPlayerMuted(Message.Sender))
	{
		return; // Don't display messages from muted players
	}

	// Broadcast to local listeners (UI widgets)
	OnChatMessageReceived.Broadcast(Message);
}

void UChatComponent::ClientNotifyMessageFailed_Implementation(const FString& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Chat message failed: %s"), *Reason);
	
	// You could broadcast this to UI as well if needed
	// For now, just log it
}

void UChatComponent::MutePlayer(APlayerState* PlayerToMute)
{
	if (!PlayerToMute || PlayerToMute == GetOwningPlayerState())
	{
		return; // Can't mute yourself
	}

	if (!MutedPlayers.Contains(PlayerToMute))
	{
		MutedPlayers.Add(PlayerToMute);
	}
}

void UChatComponent::UnmutePlayer(APlayerState* PlayerToUnmute)
{
	if (!PlayerToUnmute)
	{
		return;
	}

	MutedPlayers.Remove(PlayerToUnmute);
}

bool UChatComponent::IsPlayerMuted(APlayerState* Player) const
{
	return Player && MutedPlayers.Contains(Player);
}

void UChatComponent::ClearMutedPlayers()
{
	MutedPlayers.Empty();
}

UChatSubsystem* UChatComponent::GetChatSubsystem()
{
	if (ChatSubsystem)
	{
		return ChatSubsystem;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	ChatSubsystem = GameInstance->GetSubsystem<UChatSubsystem>();
	return ChatSubsystem;
}

APlayerState* UChatComponent::GetOwningPlayerState() const
{
	return Cast<APlayerState>(GetOwner());
}

bool UChatComponent::ValidateMessageLocally(const FString& Content, FString& OutFailureReason)
{
	// Check if message is empty
	if (Content.IsEmpty())
	{
		OutFailureReason = TEXT("Message cannot be empty");
		return false;
	}

	// Check message length
	UChatSubsystem* Subsystem = GetChatSubsystem();
	if (Subsystem)
	{
		const FChatSettings& Settings = Subsystem->GetChatSettings();
		
		if (Content.Len() > Settings.MaxMessageLength)
		{
			OutFailureReason = FString::Printf(TEXT("Message too long (max %d characters)"), Settings.MaxMessageLength);
			return false;
		}

		// Check rate limiting
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		const float TimeSinceLastMessage = CurrentTime - LastMessageTime;
		
		if (TimeSinceLastMessage < Settings.MessageCooldown)
		{
			OutFailureReason = FString::Printf(TEXT("Please wait %.1f seconds before sending another message"), 
				Settings.MessageCooldown - TimeSinceLastMessage);
			return false;
		}

		// Update last message time
		const_cast<UChatComponent*>(this)->LastMessageTime = CurrentTime;
	}

	return true;
}
