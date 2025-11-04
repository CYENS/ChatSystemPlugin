// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ChatMessage.h"
#include "ChatComponent.generated.h"

class UChatSubsystem;

/**
 * Component that handles chat functionality for a player
 * Should be attached to PlayerState for proper replication
 * Handles sending messages to server and receiving messages from server
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHATSYSTEM_API UChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UChatComponent();

	// Delegate for local message reception (UI binds to this)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageReceivedDelegate, const FChatMessage&, Message);
	
	/** Broadcast when this client receives a chat message */
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceivedDelegate OnChatMessageReceived;

	/**
	 * Send a chat message to the specified channel
	 * @param Content The message content
	 * @param Channel The channel to send to
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Content, EChatChannel Channel = EChatChannel::Global);

	/**
	 * Send a whisper (private message) to another player
	 * @param TargetPlayer The player to whisper to
	 * @param Content The message content
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendWhisper(APlayerState* TargetPlayer, const FString& Content);

	/**
	 * Send a proximity chat message (only players nearby will receive it)
	 * @param Content The message content
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendProximityMessage(const FString& Content);

	/**
	 * Mute a specific player (local only, doesn't affect other players)
	 * @param PlayerToMute The player to mute
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void MutePlayer(APlayerState* PlayerToMute);

	/**
	 * Unmute a previously muted player
	 * @param PlayerToUnmute The player to unmute
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void UnmutePlayer(APlayerState* PlayerToUnmute);

	/**
	 * Check if a player is muted
	 * @param Player The player to check
	 * @return True if the player is muted
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool IsPlayerMuted(APlayerState* Player) const;

	/**
	 * Get the list of muted players
	 * @return Array of muted player states
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	TArray<APlayerState*> GetMutedPlayers() const { return MutedPlayers; }

	/**
	 * Clear all muted players
	 */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ClearMutedPlayers();

	/**
	 * Client RPC to receive a message from server
	 * Public so ChatSubsystem can call it
	 * @param Message The message to receive
	 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveMessage(const FChatMessage& Message);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Server RPC to send a message
	 * @param Content The message content
	 * @param Channel The channel to send to
	 * @param WhisperTarget Optional target for whisper messages
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendMessage(const FString& Content, EChatChannel Channel, APlayerState* WhisperTarget);

	/**
	 * Client RPC to notify of message send failure
	 * @param Reason The reason the message failed
	 */
	UFUNCTION(Client, Reliable)
	void ClientNotifyMessageFailed(const FString& Reason);

private:
	/** List of players this client has muted (local only) */
	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> MutedPlayers;

	/** Timestamp of last message sent (for rate limiting) */
	float LastMessageTime;

	/** Cached reference to chat subsystem */
	UPROPERTY()
	TObjectPtr<UChatSubsystem> ChatSubsystem;

	/** Get the chat subsystem */
	UChatSubsystem* GetChatSubsystem();

	/** Get the owning PlayerState */
	APlayerState* GetOwningPlayerState() const;

	/** Validate message before sending */
	bool ValidateMessageLocally(const FString& Content, FString& OutFailureReason);
};
