// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChatSubsystem.h"
#include "ChatComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UChatSubsystem::UChatSubsystem()
{
	// Initialize default settings
	ChatSettings = FChatSettings();
}

void UChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Log, TEXT("ChatSubsystem initialized"));
}

void UChatSubsystem::Deinitialize()
{
	// Clean up
	RegisteredComponents.Empty();
	MessageHistory.Empty();
	PlayerMessageTimes.Empty();
	
	Super::Deinitialize();
}

bool UChatSubsystem::BroadcastMessage(const FChatMessage& Message, FString& OutFailureReason)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OutFailureReason = TEXT("Invalid world");
		return false;
	}

	// Only server can broadcast messages
	if (!World->GetAuthGameMode())
	{
		OutFailureReason = TEXT("Only server can broadcast messages");
		return false;
	}

	// Validate the message
	if (!ValidateMessage(Message, OutFailureReason))
	{
		return false;
	}

	// Check rate limiting
	if (Message.Sender && IsPlayerRateLimited(Message.Sender, OutFailureReason))
	{
		return false;
	}

	// Add to history
	AddToHistory(Message);

	// Route the message based on channel
	RouteMessage(Message);

	return true;
}

void UChatSubsystem::BroadcastSystemMessage(const FString& Content, FLinearColor Color)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetAuthGameMode())
	{
		return; // Only server can send system messages
	}

	FChatMessage SystemMessage;
	SystemMessage.Sender = nullptr;
	SystemMessage.SenderName = TEXT("System");
	SystemMessage.Content = Content;
	SystemMessage.Channel = EChatChannel::System;
	SystemMessage.MessageColor = Color;
	SystemMessage.Timestamp = FDateTime::Now();

	// Add to history
	AddToHistory(SystemMessage);

	// Send to all players
	SendToAllPlayers(SystemMessage);
}

TArray<FChatMessage> UChatSubsystem::GetRecentMessages(int32 Count) const
{
	if (Count <= 0 || Count >= MessageHistory.Num())
	{
		return MessageHistory;
	}

	// Return the last N messages
	TArray<FChatMessage> RecentMessages;
	const int32 StartIndex = FMath::Max(0, MessageHistory.Num() - Count);
	
	for (int32 i = StartIndex; i < MessageHistory.Num(); ++i)
	{
		RecentMessages.Add(MessageHistory[i]);
	}

	return RecentMessages;
}

void UChatSubsystem::ClearMessageHistory()
{
	MessageHistory.Empty();
}

void UChatSubsystem::SetChatSettings(const FChatSettings& NewSettings)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetAuthGameMode())
	{
		return; // Only server can change settings
	}

	ChatSettings = NewSettings;
}

void UChatSubsystem::RegisterChatComponent(UChatComponent* Component)
{
	if (Component && !RegisteredComponents.Contains(Component))
	{
		RegisteredComponents.Add(Component);
		UE_LOG(LogTemp, Log, TEXT("ChatComponent registered. Total: %d"), RegisteredComponents.Num());
	}
}

void UChatSubsystem::UnregisterChatComponent(UChatComponent* Component)
{
	if (Component)
	{
		RegisteredComponents.Remove(Component);
		
		// Clean up player message times if this was their component
		APlayerState* PS = Cast<APlayerState>(Component->GetOwner());
		if (PS)
		{
			PlayerMessageTimes.Remove(PS);
		}
		
		UE_LOG(LogTemp, Log, TEXT("ChatComponent unregistered. Total: %d"), RegisteredComponents.Num());
	}
}

bool UChatSubsystem::ValidateMessage(const FChatMessage& Message, FString& OutFailureReason)
{
	// Check if message content is valid
	if (Message.Content.IsEmpty() && !ChatSettings.bAllowEmptyMessages)
	{
		OutFailureReason = TEXT("Message content is empty");
		return false;
	}

	// Check message length
	if (Message.Content.Len() > ChatSettings.MaxMessageLength)
	{
		OutFailureReason = FString::Printf(TEXT("Message too long (max %d characters)"), ChatSettings.MaxMessageLength);
		return false;
	}

	// Validate sender for non-system messages
	if (Message.Channel != EChatChannel::System && !Message.Sender)
	{
		OutFailureReason = TEXT("Invalid sender");
		return false;
	}

	// Validate whisper target
	if (Message.Channel == EChatChannel::Whisper && !Message.WhisperTarget)
	{
		OutFailureReason = TEXT("Whisper requires a target player");
		return false;
	}

	return true;
}

void UChatSubsystem::RouteMessage(const FChatMessage& Message)
{
	switch (Message.Channel)
	{
	case EChatChannel::Global:
	case EChatChannel::System:
		SendToAllPlayers(Message);
		break;

	case EChatChannel::Team:
		SendToTeam(Message);
		break;

	case EChatChannel::Whisper:
		SendToPlayer(Message);
		break;

	case EChatChannel::Proximity:
		SendToProximity(Message);
		break;

	case EChatChannel::Custom:
		// Custom channels can be handled by game-specific logic
		SendToAllPlayers(Message);
		break;

	default:
		SendToAllPlayers(Message);
		break;
	}
}

void UChatSubsystem::SendToAllPlayers(const FChatMessage& Message)
{
	for (UChatComponent* Component : RegisteredComponents)
	{
		if (Component && Component->GetOwner())
		{
			Component->ClientReceiveMessage(Message);
		}
	}
}

void UChatSubsystem::SendToTeam(const FChatMessage& Message)
{
	if (!Message.Sender)
	{
		return;
	}

	// Team chat requires custom implementation in your PlayerState
	// For now, send to all players as a fallback
	// To implement team chat properly:
	// 1. Add a TeamId property to your PlayerState (replicated)
	// 2. Override this function in a custom ChatSubsystem subclass
	// 3. Compare team IDs and only send to matching teams
	
	UE_LOG(LogTemp, Warning, TEXT("Team chat is not fully implemented. Override SendToTeam() in a custom ChatSubsystem to add team filtering. Sending to all players as fallback."));
	SendToAllPlayers(Message);
}

void UChatSubsystem::SendToPlayer(const FChatMessage& Message)
{
	if (!Message.WhisperTarget)
	{
		return;
	}

	// Send to the target player
	UChatComponent* TargetComponent = GetChatComponentForPlayer(Message.WhisperTarget);
	if (TargetComponent)
	{
		TargetComponent->ClientReceiveMessage(Message);
	}

	// Also send to the sender so they see their own whisper
	if (Message.Sender)
	{
		UChatComponent* SenderComponent = GetChatComponentForPlayer(Message.Sender);
		if (SenderComponent)
		{
			SenderComponent->ClientReceiveMessage(Message);
		}
	}
}

void UChatSubsystem::SendToProximity(const FChatMessage& Message)
{
	if (!Message.Sender)
	{
		return;
	}

	// Get sender's pawn location
	APawn* SenderPawn = Message.Sender->GetPawn();
	if (!SenderPawn)
	{
		return;
	}

	const FVector SenderLocation = SenderPawn->GetActorLocation();
	const float RadiusSquared = ChatSettings.ProximityChatRadius * ChatSettings.ProximityChatRadius;

	for (UChatComponent* Component : RegisteredComponents)
	{
		if (!Component)
		{
			continue;
		}

		APlayerState* PS = Cast<APlayerState>(Component->GetOwner());
		if (!PS)
		{
			continue;
		}

		APawn* Pawn = PS->GetPawn();
		if (!Pawn)
		{
			continue;
		}

		// Check if within proximity range
		const float DistanceSquared = FVector::DistSquared(SenderLocation, Pawn->GetActorLocation());
		if (DistanceSquared <= RadiusSquared)
		{
			Component->ClientReceiveMessage(Message);
		}
	}
}

UChatComponent* UChatSubsystem::GetChatComponentForPlayer(APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return nullptr;
	}

	for (UChatComponent* Component : RegisteredComponents)
	{
		if (Component && Component->GetOwner() == PlayerState)
		{
			return Component;
		}
	}

	return nullptr;
}

void UChatSubsystem::AddToHistory(const FChatMessage& Message)
{
	MessageHistory.Add(Message);

	// Trim history if it exceeds max size
	while (MessageHistory.Num() > ChatSettings.MaxHistorySize)
	{
		MessageHistory.RemoveAt(0);
	}
}

bool UChatSubsystem::IsPlayerRateLimited(APlayerState* PlayerState, FString& OutFailureReason)
{
	if (!PlayerState)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float CurrentTime = World->GetTimeSeconds();
	
	if (float* LastMessageTime = PlayerMessageTimes.Find(PlayerState))
	{
		const float TimeSinceLastMessage = CurrentTime - *LastMessageTime;
		
		if (TimeSinceLastMessage < ChatSettings.MessageCooldown)
		{
			OutFailureReason = FString::Printf(TEXT("Please wait %.1f seconds before sending another message"), ChatSettings.MessageCooldown - TimeSinceLastMessage);
			return true;
		}
	}

	// Update last message time
	PlayerMessageTimes.Add(PlayerState, CurrentTime);
	return false;
}
