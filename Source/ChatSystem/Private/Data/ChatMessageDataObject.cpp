
#include "Data/ChatMessageDataObject.h"

#include "Data/ChatMessage.h"

FChatMessage UChatMessageDataObject::GetChatMessage() const
{
	return ChatMessage;
}

void UChatMessageDataObject::SetChatMessage(const FChatMessage& Message)
{
	ChatMessage = Message;
}
