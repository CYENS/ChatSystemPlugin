#pragma once

#include "CoreMinimal.h"
#include "ChatMessage.h"
#include "UObject/Object.h"
#include "ChatMessageDataObject.generated.h"


/**
 * A thin wrapper around a chat message to be used by ListView Entries.
 */
UCLASS(Blueprintable)
class CHATSYSTEM_API UChatMessageDataObject : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="Default", meta=(ExposeOnSpawn=true, AllowPrivateAccess=true))
	FChatMessage ChatMessage;

public:
	UFUNCTION(BlueprintCallable, Category="Default")
	FChatMessage GetChatMessage() const;
	
	UFUNCTION(BlueprintCallable, Category="Default")
	void SetChatMessage(const FChatMessage& Message);
};
