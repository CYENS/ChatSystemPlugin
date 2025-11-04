// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/ChatMessage.h"
#include "ChatMessageReceiver.generated.h"

/**
 * Interface for objects that want to receive chat messages
 * Primarily designed for UI widgets to implement in Blueprint
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UChatMessageReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for receiving chat-related events
 * Allows complete decoupling between chat logic and UI presentation
 */
class CHATSYSTEM_API IChatMessageReceiver
{
	GENERATED_BODY()

public:
	/**
	 * Called when a new chat message is received
	 * @param Message The chat message that was received
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void OnChatMessageReceived(const FChatMessage& Message);

	/**
	 * Called when a player joins the chat (connects to server)
	 * @param Player The PlayerState of the player who joined
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void OnPlayerJoinedChat(APlayerState* Player);

	/**
	 * Called when a player leaves the chat (disconnects from server)
	 * @param Player The PlayerState of the player who left
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void OnPlayerLeftChat(APlayerState* Player);

	/**
	 * Called when the local player is muted or unmuted
	 * @param bIsMuted Whether the player is now muted
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void OnMuteStatusChanged(bool bIsMuted);

	/**
	 * Called when a message fails to send (validation failed, rate limited, etc.)
	 * @param Reason The reason the message failed to send
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chat")
	void OnMessageSendFailed(const FString& Reason);
};
