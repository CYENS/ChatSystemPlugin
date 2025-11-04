# Chat System Plugin

A robust, replicated multiuser chat system for Unreal Engine 5.6+ with complete UI decoupling.

## Overview

This plugin provides a server-authoritative chat system with support for multiple chat channels, message validation, rate limiting, and flexible Blueprint UI integration. The architecture follows Unreal Engine best practices using a Subsystem-Component-Interface pattern.

## Features

- **Server-Authoritative Replication**: All messages are validated and distributed by the server
- **Multiple Chat Channels**: Global, Team, Whisper, System, Proximity, and Custom
- **Rate Limiting**: Configurable cooldown between messages to prevent spam
- **Message History**: Automatic history management for late joiners
- **Player Muting**: Client-side muting of specific players
- **UI Decoupling**: Complete separation between chat logic and UI via interfaces and delegates
- **Blueprint-Friendly**: Full Blueprint support for UI creation and customization
- **Proximity Chat**: Distance-based message delivery
- **Team Chat**: Team-based message filtering

## Architecture

### Core Components

1. **UChatSubsystem** (Game Instance Subsystem)
   - Central message broker
   - Server-side validation and routing
   - Message history management
   - Rate limiting enforcement

2. **UChatComponent** (Actor Component)
   - Attached to PlayerState
   - Handles per-player chat functionality
   - Client-to-server message sending
   - Local muting functionality

3. **IChatMessageReceiver** (Interface)
   - Blueprint-implementable interface for UI widgets
   - Event-driven message reception
   - Complete UI flexibility

4. **FChatMessage** (Struct)
   - Lightweight message data structure
   - Optimized for replication
   - Contains sender info, content, channel, timestamp

## Quick Start

### 1. Add ChatComponent to PlayerState

In your PlayerState class (C++ or Blueprint):

```cpp
// C++ Example
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chat")
TObjectPtr<UChatComponent> ChatComponent;

// In constructor
ChatComponent = CreateDefaultSubobject<UChatComponent>(TEXT("ChatComponent"));
```

Or in Blueprint:
- Open your PlayerState Blueprint
- Add Component → Chat Component

### 2. Create a Chat UI Widget

Create a new Widget Blueprint and implement the `ChatMessageReceiver` interface:

**Event Graph Setup:**
```
Event Construct
  → Get Game Instance
  → Get Subsystem (ChatSubsystem)
  → Get Player State
  → Get Component (ChatComponent)
  → Bind Event to OnChatMessageReceived
```

**Implement Interface Function:**
```
OnChatMessageReceived (Message)
  → Create Text Widget
  → Format: [Timestamp] SenderName: Content
  → Set Text Color (use Message.GetChannelColor())
  → Add to Scroll Box
```

### 3. Send Messages

**Blueprint:**
```
On Send Button Clicked
  → Get Player State
  → Get Component (ChatComponent)
  → SendChatMessage(TextBox.Text, Global)
  → Clear TextBox
```

**C++:**
```cpp
UChatComponent* ChatComp = PlayerState->FindComponentByClass<UChatComponent>();
if (ChatComp)
{
    ChatComp->SendChatMessage(TEXT("Hello World!"), EChatChannel::Global);
}
```

## Chat Channels

### EChatChannel Types

- **Global**: Visible to all players
- **Team**: Only visible to players on the same team
- **Whisper**: Private message to a specific player
- **System**: Server-generated messages (yellow by default)
- **Proximity**: Only visible to players within a certain radius
- **Custom**: For game-specific channel implementations

### Channel Usage Examples

**Global Chat:**
```cpp
ChatComponent->SendChatMessage("Hello everyone!", EChatChannel::Global);
```

**Team Chat:**
```cpp
// Note: Team chat requires custom implementation (see IMPLEMENTATION_GUIDE.md)
// By default, team messages are sent to all players
ChatComponent->SendChatMessage("Enemy spotted!", EChatChannel::Team);
```

**Whisper:**
```cpp
APlayerState* TargetPlayer = GetTargetPlayerState();
ChatComponent->SendWhisper(TargetPlayer, "Secret message");
```

**Proximity Chat:**
```cpp
ChatComponent->SendProximityMessage("Anyone nearby?");
```

**System Message (Server Only):**
```cpp
UChatSubsystem* ChatSys = GetGameInstance()->GetSubsystem<UChatSubsystem>();
ChatSys->BroadcastSystemMessage("Server restart in 5 minutes", FLinearColor::Red);
```

## Configuration

### Chat Settings

Configure chat behavior via `FChatSettings`:

```cpp
UChatSubsystem* ChatSys = GetGameInstance()->GetSubsystem<UChatSubsystem>();
FChatSettings Settings = ChatSys->GetChatSettings();

Settings.MaxMessageLength = 256;           // Maximum characters per message
Settings.MessageCooldown = 0.5f;           // Seconds between messages
Settings.MaxHistorySize = 100;             // Number of messages to keep
Settings.ProximityChatRadius = 1000.0f;    // Radius in cm for proximity chat
Settings.bEnableProfanityFilter = false;   // Enable/disable profanity filter
Settings.bAllowEmptyMessages = false;      // Allow empty messages

ChatSys->SetChatSettings(Settings);
```

## Player Muting

Players can locally mute other players (client-side only):

**Blueprint:**
```
Mute Player
  → Get Chat Component
  → MutePlayer(PlayerState)

Unmute Player
  → Get Chat Component
  → UnmutePlayer(PlayerState)

Check if Muted
  → Get Chat Component
  → IsPlayerMuted(PlayerState) → Boolean
```

**C++:**
```cpp
// Mute a player
ChatComponent->MutePlayer(TargetPlayerState);

// Unmute a player
ChatComponent->UnmutePlayer(TargetPlayerState);

// Check if muted
bool bIsMuted = ChatComponent->IsPlayerMuted(TargetPlayerState);

// Get all muted players
TArray<APlayerState*> MutedPlayers = ChatComponent->GetMutedPlayers();
```

## Message History

The subsystem automatically maintains message history for late joiners:

```cpp
// Get recent messages (e.g., for UI initialization)
UChatSubsystem* ChatSys = GetGameInstance()->GetSubsystem<UChatSubsystem>();
TArray<FChatMessage> RecentMessages = ChatSys->GetRecentMessages(50);

// Display in UI
for (const FChatMessage& Msg : RecentMessages)
{
    DisplayMessage(Msg);
}
```

## Advanced Usage

### Custom Message Colors

```cpp
FChatMessage Message(PlayerState, "Custom colored message", EChatChannel::Custom);
Message.MessageColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
```

### Message Formatting in UI

```cpp
FString FormattedMessage = FString::Printf(
    TEXT("[%s] %s: %s"),
    *Message.GetFormattedTimestamp(),
    *Message.SenderName,
    *Message.Content
);
```

### Handling Message Send Failures

Implement error handling in your UI:

```cpp
// In ChatComponent
OnMessageSendFailed.AddDynamic(this, &UMyChatWidget::HandleMessageFailed);

void UMyChatWidget::HandleMessageFailed(const FString& Reason)
{
    // Display error to user
    ShowErrorNotification(Reason);
}
```

## Network Considerations

### Replication Flow

1. Client calls `SendChatMessage()` on their ChatComponent
2. ChatComponent validates locally (length, rate limit)
3. ChatComponent calls `ServerSendMessage` RPC
4. Server receives RPC, forwards to ChatSubsystem
5. ChatSubsystem validates message (server-side)
6. ChatSubsystem routes message based on channel
7. ChatSubsystem calls `ClientReceiveMessage` RPC on relevant clients
8. Clients receive message and broadcast to local UI

### Performance Tips

- Message history is trimmed automatically based on `MaxHistorySize`
- Proximity chat uses distance-squared checks for efficiency
- Rate limiting prevents message spam
- Muted players are filtered client-side (no network overhead)

## Troubleshooting

### Messages Not Appearing

1. Verify ChatComponent is attached to PlayerState
2. Check that UI widget is binding to OnChatMessageReceived event
3. Ensure server is running (messages require server authority)
4. Check if sender is muted locally

### Rate Limiting Issues

- Adjust `MessageCooldown` in ChatSettings
- Check server logs for validation failures
- Verify client-side validation matches server settings

### Proximity Chat Not Working

- Ensure players have valid Pawns with locations
- Check `ProximityChatRadius` setting (in cm, not meters)
- Verify players are within range

## API Reference

### UChatComponent

**Public Functions:**
- `SendChatMessage(Content, Channel)` - Send a message to a channel
- `SendWhisper(TargetPlayer, Content)` - Send private message
- `SendProximityMessage(Content)` - Send proximity-based message
- `MutePlayer(PlayerState)` - Mute a player locally
- `UnmutePlayer(PlayerState)` - Unmute a player
- `IsPlayerMuted(PlayerState)` - Check if player is muted
- `GetMutedPlayers()` - Get list of muted players
- `ClearMutedPlayers()` - Clear all muted players

**Delegates:**
- `OnChatMessageReceived` - Fired when a message is received

### UChatSubsystem

**Public Functions:**
- `BroadcastMessage(Message, OutFailureReason)` - Broadcast a message (server only)
- `BroadcastSystemMessage(Content, Color)` - Send system message (server only)
- `GetRecentMessages(Count)` - Get message history
- `ClearMessageHistory()` - Clear all history
- `GetChatSettings()` - Get current settings
- `SetChatSettings(NewSettings)` - Update settings (server only)

### IChatMessageReceiver Interface

**Blueprint Events:**
- `OnChatMessageReceived(Message)` - New message received
- `OnPlayerJoinedChat(Player)` - Player joined
- `OnPlayerLeftChat(Player)` - Player left
- `OnMuteStatusChanged(bIsMuted)` - Mute status changed
- `OnMessageSendFailed(Reason)` - Message failed to send

## License

Copyright Epic Games, Inc. All Rights Reserved.
