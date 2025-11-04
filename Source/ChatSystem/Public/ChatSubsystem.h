// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/ChatMessage.h"
#include "ChatSubsystem.generated.h"

class UChatComponent;
class APlayerState;

/**
 * Game Instance Subsystem that manages the chat system
 * Handles message broadcasting, validation, and history
 * Server-authoritative: all messages go through the server
 */
UCLASS()
class CHATSYSTEM_API UChatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UChatSubsystem();

	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Broadcast a message to relevant players
	 * Should only be called on the server
	 * @param Message The message to broadcast
	 * @param OutFailureReason If validation fails, this will contain the reason
	 * @return True if message was successfully broadcast
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool BroadcastMessage(const FChatMessage& Message, FString& OutFailureReason);

	/**
	 * Send a system message to all players
	 * @param Content The message content
	 * @param Color Optional color for the message
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void BroadcastSystemMessage(const FString& Content, FLinearColor Color = FLinearColor::Yellow);

	/**
	 * Get recent chat messages (for late joiners or UI history)
	 * @param Count Number of recent messages to retrieve (0 = all)
	 * @return Array of recent messages
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	TArray<FChatMessage> GetRecentMessages(int32 Count = 50) const;

	/**
	 * Clear all message history
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ClearMessageHistory();

	/**
	 * Get the current chat settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	const FChatSettings& GetChatSettings() const { return ChatSettings; }

	/**
	 * Update chat settings (server only)
	 * @param NewSettings The new settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetChatSettings(const FChatSettings& NewSettings);

	/**
	 * Register a chat component (called automatically by components)
	 * @param Component The component to register
	 */
	void RegisterChatComponent(UChatComponent* Component);

	/**
	 * Unregister a chat component (called automatically by components)
	 * @param Component The component to unregister
	 */
	void UnregisterChatComponent(UChatComponent* Component);

	/**
	 * Get all registered chat components
	 */
	const TArray<UChatComponent*>& GetRegisteredComponents() const { return RegisteredComponents; }

protected:
	/**
	 * Validate a message before broadcasting
	 * @param Message The message to validate
	 * @param OutFailureReason If validation fails, this will contain the reason
	 * @return True if message is valid
	 */
	bool ValidateMessage(const FChatMessage& Message, FString& OutFailureReason);

	/**
	 * Send message to specific players based on channel type
	 * @param Message The message to send
	 */
	void RouteMessage(const FChatMessage& Message);

	/**
	 * Send message to all players
	 * @param Message The message to send
	 */
	void SendToAllPlayers(const FChatMessage& Message);

	/**
	 * Send message to players on the same team
	 * @param Message The message to send
	 */
	void SendToTeam(const FChatMessage& Message);

	/**
	 * Send message to a specific player (whisper)
	 * @param Message The message to send
	 */
	void SendToPlayer(const FChatMessage& Message);

	/**
	 * Send message to players within proximity range
	 * @param Message The message to send
	 */
	void SendToProximity(const FChatMessage& Message);

	/**
	 * Get the chat component for a player state
	 * @param PlayerState The player state to get the component from
	 * @return The chat component, or nullptr if not found
	 */
	UChatComponent* GetChatComponentForPlayer(APlayerState* PlayerState) const;

	/**
	 * Add message to history
	 * @param Message The message to add
	 */
	void AddToHistory(const FChatMessage& Message);

private:
	/** Chat configuration settings */
	UPROPERTY(EditAnywhere, Category = "Chat Settings")
	FChatSettings ChatSettings;

	/** Message history for late joiners */
	UPROPERTY()
	TArray<FChatMessage> MessageHistory;

	/** All registered chat components */
	UPROPERTY()
	TArray<TObjectPtr<UChatComponent>> RegisteredComponents;

	/** Track last message time per player for rate limiting */
	TMap<APlayerState*, float> PlayerMessageTimes;

	/** Check if a player is rate limited */
	bool IsPlayerRateLimited(APlayerState* PlayerState, FString& OutFailureReason);
};
