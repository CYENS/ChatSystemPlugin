// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ChatMessage.generated.h"

/**
 * Enum defining different chat channels/types
 */
UENUM(BlueprintType)
enum class EChatChannel : uint8
{
	Global UMETA(DisplayName = "Global"),
	Team UMETA(DisplayName = "Team"),
	Whisper UMETA(DisplayName = "Whisper"),
	System UMETA(DisplayName = "System"),
	Proximity UMETA(DisplayName = "Proximity"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Structure representing a single chat message
 * Designed to be lightweight for replication
 */
USTRUCT(BlueprintType)
struct CHATSYSTEM_API FChatMessage
{
	GENERATED_BODY()

	/** The PlayerState of the sender (null for system messages) */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TObjectPtr<APlayerState> Sender;

	/** Cached sender name for display (in case PlayerState becomes invalid) */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString SenderName;

	/** The actual message content */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString Content;

	/** Which channel this message belongs to */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	EChatChannel Channel;

	/** When the message was sent */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FDateTime Timestamp;

	/** Optional color for UI customization */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FLinearColor MessageColor;

	/** For whispers, the target player */
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TObjectPtr<APlayerState> WhisperTarget;

	/** Default constructor */
	FChatMessage()
		: Sender(nullptr)
		, SenderName(TEXT(""))
		, Content(TEXT(""))
		, Channel(EChatChannel::Global)
		, Timestamp(FDateTime::Now())
		, MessageColor(FLinearColor::White)
		, WhisperTarget(nullptr)
	{
	}

	/** Convenience constructor for common cases */
	FChatMessage(APlayerState* InSender, const FString& InContent, EChatChannel InChannel)
		: Sender(InSender)
		, SenderName(InSender ? InSender->GetPlayerName() : TEXT("System"))
		, Content(InContent)
		, Channel(InChannel)
		, Timestamp(FDateTime::Now())
		, MessageColor(FLinearColor::White)
		, WhisperTarget(nullptr)
	{
	}

	/** Check if this is a valid message */
	bool IsValid() const
	{
		return !Content.IsEmpty() && !SenderName.IsEmpty();
	}

	/** Get formatted timestamp string */
	FString GetFormattedTimestamp() const
	{
		return Timestamp.ToString(TEXT("%H:%M:%S"));
	}

	/** Get display color based on channel */
	FLinearColor GetChannelColor() const
	{
		switch (Channel)
		{
		case EChatChannel::Global:
			return FLinearColor::White;
		case EChatChannel::Team:
			return FLinearColor(0.0f, 0.8f, 1.0f); // Cyan
		case EChatChannel::Whisper:
			return FLinearColor(1.0f, 0.5f, 1.0f); // Pink
		case EChatChannel::System:
			return FLinearColor::Yellow;
		case EChatChannel::Proximity:
			return FLinearColor(0.5f, 1.0f, 0.5f); // Light green
		default:
			return MessageColor;
		}
	}
};

/**
 * Settings for chat filtering and validation
 */
USTRUCT(BlueprintType)
struct CHATSYSTEM_API FChatSettings
{
	GENERATED_BODY()

	/** Maximum message length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	int32 MaxMessageLength = 256;

	/** Minimum time between messages (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	float MessageCooldown = 0.5f;

	/** Maximum messages to keep in history */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	int32 MaxHistorySize = 100;

	/** Enable profanity filter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	bool bEnableProfanityFilter = false;

	/** Proximity chat radius (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	float ProximityChatRadius = 1000.0f;

	/** Allow empty messages */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat Settings")
	bool bAllowEmptyMessages = false;
};
