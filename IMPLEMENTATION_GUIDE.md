# Chat System Implementation Guide

This guide provides detailed instructions for developers to implement the Chat System plugin in their Unreal Engine project.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Step-by-Step Implementation](#step-by-step-implementation)
3. [Adding ChatComponent to PlayerState](#adding-chatcomponent-to-playerstate)
4. [Creating a Chat UI Widget](#creating-a-chat-ui-widget)
5. [Testing the Chat System](#testing-the-chat-system)
6. [Advanced Customization](#advanced-customization)
7. [Common Issues and Solutions](#common-issues-and-solutions)

---

## Prerequisites

- Unreal Engine 5.6 or later
- Basic understanding of Unreal Engine's replication system
- Familiarity with Blueprint or C++
- A multiplayer-enabled project with PlayerState

---

## Step-by-Step Implementation

### 1. Enable the Plugin

1. Open your project in Unreal Engine
2. Go to **Edit → Plugins**
3. Search for "ChatSystem"
4. Check the box to enable it
5. Restart the editor when prompted

### 2. Verify Plugin Files

Ensure the following files exist in your plugin directory:

```
Plugins/ChatSystem/
├── Source/ChatSystem/
│   ├── Public/
│   │   ├── ChatComponent.h
│   │   ├── ChatSubsystem.h
│   │   ├── Data/ChatMessage.h
│   │   └── Interfaces/ChatMessageReceiver.h
│   └── Private/
│       ├── ChatComponent.cpp
│       ├── ChatSubsystem.cpp
│       └── Interfaces/ChatMessageReceiver.cpp
```

---

## Adding ChatComponent to PlayerState

### Option A: C++ Implementation

1. **Open your PlayerState header file** (e.g., `MyPlayerState.h`)

2. **Add the include:**
```cpp
#include "ChatComponent.h"
```

3. **Declare the component:**
```cpp
UCLASS()
class MYGAME_API AMyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AMyPlayerState();

    /** Chat component for this player */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chat")
    TObjectPtr<UChatComponent> ChatComponent;
};
```

4. **In your PlayerState .cpp file, create the component:**
```cpp
AMyPlayerState::AMyPlayerState()
{
    // Create chat component
    ChatComponent = CreateDefaultSubobject<UChatComponent>(TEXT("ChatComponent"));
}
```

5. **Compile your project**

### Option B: Blueprint Implementation

1. **Open your PlayerState Blueprint** (e.g., `BP_MyPlayerState`)

2. **Add Component:**
   - Click "Add Component" button
   - Search for "Chat Component"
   - Select it to add to your PlayerState

3. **Compile and Save**

---

## Creating a Chat UI Widget

### Step 1: Create the Widget Blueprint

1. **Create a new Widget Blueprint:**
   - Right-click in Content Browser
   - User Interface → Widget Blueprint
   - Name it `WBP_ChatWindow`

### Step 2: Design the UI

Add the following widgets to your canvas:

```
Canvas Panel
└── Vertical Box (Chat Container)
    ├── Scroll Box (Message Display)
    │   └── Vertical Box (Messages Container)
    └── Horizontal Box (Input Area)
        ├── Editable Text Box (Message Input)
        └── Button (Send Button)
```

**Recommended Settings:**
- **Scroll Box**: Auto-scroll to bottom
- **Message Input**: Hint Text: "Type a message..."
- **Send Button**: Text: "Send"

### Step 3: Implement the Interface

1. **Add Interface:**
   - Class Settings → Interfaces
   - Add `ChatMessageReceiver`

2. **Implement Interface Functions:**

**Event Graph:**

```
Event Construct
├── Get Owning Player
├── Get Player State
├── Get Component by Class (ChatComponent)
├── Bind Event to OnChatMessageReceived
└── Store ChatComponent reference
```

**OnChatMessageReceived Implementation:**

```
Event OnChatMessageReceived (Message)
├── Create Widget (WBP_ChatMessageEntry)
├── Set Message Data
│   ├── Set Sender Name (Message.SenderName)
│   ├── Set Content (Message.Content)
│   ├── Set Timestamp (Message.GetFormattedTimestamp())
│   └── Set Color (Message.GetChannelColor())
└── Add to Messages Container (Scroll Box)
```

### Step 4: Create Message Entry Widget

Create `WBP_ChatMessageEntry`:

```
Horizontal Box
├── Text Block (Timestamp) - [HH:MM:SS]
├── Text Block (Sender Name) - PlayerName:
└── Text Block (Message Content) - Message text
```

### Step 5: Implement Send Functionality

**On Send Button Clicked:**

```
On Send Button Clicked
├── Get Text from Message Input
├── Is Valid String? (not empty)
│   ├── TRUE:
│   │   ├── Get ChatComponent
│   │   ├── SendChatMessage (Text, Global)
│   │   └── Clear Message Input
│   └── FALSE:
│       └── (Do nothing)
```

**Blueprint Code Example:**

```
// Get the message text
FString MessageText = MessageInputTextBox->GetText().ToString();

// Validate
if (!MessageText.IsEmpty())
{
    // Get chat component
    UChatComponent* ChatComp = PlayerState->FindComponentByClass<UChatComponent>();
    
    if (ChatComp)
    {
        // Send message
        ChatComp->SendChatMessage(MessageText, EChatChannel::Global);
        
        // Clear input
        MessageInputTextBox->SetText(FText::GetEmpty());
    }
}
```

### Step 6: Add to Viewport

In your HUD or Player Controller:

```
Event BeginPlay
├── Create Widget (WBP_ChatWindow)
├── Add to Viewport
└── Set Input Mode (UI Only or Game and UI)
```

---

## Testing the Chat System

### Local Testing (PIE - Play In Editor)

1. **Configure Multiplayer Settings:**
   - Edit → Editor Preferences → Level Editor → Play
   - Number of Players: 2 or more
   - Net Mode: Play As Listen Server

2. **Launch Test:**
   - Click Play dropdown → New Editor Window (PIE)
   - Type messages in one window
   - Verify they appear in all windows

3. **Test Different Channels:**
   - Global: Should appear in all clients
   - Team: Only same team (if implemented)
   - Whisper: Only target player
   - Proximity: Only nearby players

### Dedicated Server Testing

1. **Package your project**

2. **Launch dedicated server:**
```bash
MyGame.exe -server -log
```

3. **Launch clients:**
```bash
MyGame.exe 127.0.0.1
```

4. **Test chat functionality across clients**

---

## Advanced Customization

### Custom Chat Channels

Extend the `EChatChannel` enum in `ChatMessage.h`:

```cpp
UENUM(BlueprintType)
enum class EChatChannel : uint8
{
    Global,
    Team,
    Whisper,
    System,
    Proximity,
    Custom,
    Guild,      // Add custom channel
    Trade,      // Add custom channel
    // ... more custom channels
};
```

### Custom Message Colors

Override `GetChannelColor()` in your UI:

```cpp
FLinearColor GetCustomChannelColor(EChatChannel Channel)
{
    switch (Channel)
    {
    case EChatChannel::Guild:
        return FLinearColor::Green;
    case EChatChannel::Trade:
        return FLinearColor(1.0f, 0.65f, 0.0f); // Orange
    default:
        return Message.GetChannelColor();
    }
}
```

### Chat Commands

Implement command parsing in your UI:

```cpp
void ProcessChatInput(const FString& Input)
{
    if (Input.StartsWith("/"))
    {
        // Parse command
        TArray<FString> Parts;
        Input.ParseIntoArray(Parts, TEXT(" "));
        
        FString Command = Parts[0].ToLower();
        
        if (Command == "/mute" && Parts.Num() > 1)
        {
            // Find player by name and mute
            APlayerState* Target = FindPlayerByName(Parts[1]);
            if (Target)
            {
                ChatComponent->MutePlayer(Target);
            }
        }
        else if (Command == "/w" && Parts.Num() > 2)
        {
            // Whisper command
            APlayerState* Target = FindPlayerByName(Parts[1]);
            FString Message = Input.RightChop(Command.Len() + Parts[1].Len() + 2);
            ChatComponent->SendWhisper(Target, Message);
        }
    }
    else
    {
        // Regular message
        ChatComponent->SendChatMessage(Input, CurrentChannel);
    }
}
```

### Profanity Filter

Implement in `ChatSubsystem::ValidateMessage()`:

```cpp
bool UChatSubsystem::ValidateMessage(const FChatMessage& Message, FString& OutFailureReason)
{
    // ... existing validation ...
    
    if (ChatSettings.bEnableProfanityFilter)
    {
        if (ContainsProfanity(Message.Content))
        {
            OutFailureReason = TEXT("Message contains inappropriate language");
            return false;
        }
    }
    
    return true;
}

bool UChatSubsystem::ContainsProfanity(const FString& Text)
{
    // Implement your profanity checking logic
    // Could use a word list, regex, or external service
    return false;
}
```

### Message Timestamps

Format timestamps in your UI:

```cpp
FString FormatTimestamp(const FDateTime& Timestamp)
{
    // 12-hour format with AM/PM
    return Timestamp.ToString(TEXT("%I:%M %p"));
    
    // 24-hour format
    return Timestamp.ToString(TEXT("%H:%M:%S"));
    
    // With date
    return Timestamp.ToString(TEXT("%Y-%m-%d %H:%M"));
}
```

---

## Common Issues and Solutions

### Issue: Messages Not Appearing

**Symptoms:** Chat messages are sent but don't appear in UI

**Solutions:**
1. Verify ChatComponent is attached to PlayerState
2. Check that UI widget is binding to `OnChatMessageReceived`
3. Ensure the game is running with a server (not standalone)
4. Check console for error messages

**Debug Steps:**
```cpp
// In ChatComponent::ClientReceiveMessage_Implementation
UE_LOG(LogTemp, Log, TEXT("Received message: %s from %s"), 
    *Message.Content, *Message.SenderName);
```

### Issue: Rate Limiting Too Strict

**Symptoms:** Players can't send messages frequently enough

**Solution:** Adjust `MessageCooldown` in ChatSettings:

```cpp
UChatSubsystem* ChatSys = GetGameInstance()->GetSubsystem<UChatSubsystem>();
FChatSettings Settings = ChatSys->GetChatSettings();
Settings.MessageCooldown = 0.1f; // Reduce from default 0.5s
ChatSys->SetChatSettings(Settings);
```

### Issue: Team Chat Not Working

**Symptoms:** Team messages go to all players (this is the default behavior)

**Solution:** Team chat requires custom implementation. You have two options:

**Option 1: Override SendToTeam in a Custom ChatSubsystem**

```cpp
// MyCustomChatSubsystem.h
UCLASS()
class UMyCustomChatSubsystem : public UChatSubsystem
{
    GENERATED_BODY()

protected:
    virtual void SendToTeam(const FChatMessage& Message) override;
};

// MyCustomChatSubsystem.cpp
void UMyCustomChatSubsystem::SendToTeam(const FChatMessage& Message)
{
    if (!Message.Sender)
    {
        return;
    }

    // Assuming your PlayerState has a TeamId property
    AMyPlayerState* SenderPS = Cast<AMyPlayerState>(Message.Sender);
    if (!SenderPS)
    {
        return;
    }

    const int32 SenderTeamId = SenderPS->TeamId;

    for (UChatComponent* Component : GetRegisteredComponents())
    {
        if (!Component)
        {
            continue;
        }

        AMyPlayerState* PS = Cast<AMyPlayerState>(Component->GetOwner());
        if (PS && PS->TeamId == SenderTeamId)
        {
            Component->ClientReceiveMessage(Message);
        }
    }
}
```

**Option 2: Add Team Property to PlayerState**

```cpp
// In your PlayerState
UPROPERTY(Replicated, BlueprintReadWrite, Category = "Team")
int32 TeamId = 0;

// Then override SendToTeam as shown in Option 1
```

### Issue: Proximity Chat Range Issues

**Symptoms:** Proximity chat radius seems incorrect

**Solution:** Remember radius is in centimeters (Unreal units):

```cpp
// 10 meters = 1000 cm
Settings.ProximityChatRadius = 1000.0f;

// 50 meters = 5000 cm
Settings.ProximityChatRadius = 5000.0f;
```

### Issue: Messages Persist After Player Leaves

**Symptoms:** Disconnected players' messages still show

**Solution:** Implement cleanup in your UI:

```cpp
// When player disconnects
void OnPlayerDisconnected(APlayerState* Player)
{
    // Optionally remove their messages or mark them as offline
    UpdatePlayerStatus(Player, false);
}
```

---

## Performance Optimization

### Message History Limits

```cpp
// Limit history size to prevent memory issues
Settings.MaxHistorySize = 50; // Keep last 50 messages
```

### UI Optimization

```cpp
// Limit displayed messages in UI
const int32 MaxDisplayedMessages = 100;

void AddMessageToUI(const FChatMessage& Message)
{
    // Add new message
    MessageWidgets.Add(CreateMessageWidget(Message));
    
    // Remove old messages if limit exceeded
    while (MessageWidgets.Num() > MaxDisplayedMessages)
    {
        MessageWidgets[0]->RemoveFromParent();
        MessageWidgets.RemoveAt(0);
    }
}
```

---

## Next Steps

1. **Customize the UI** to match your game's art style
2. **Add sound effects** for message received/sent
3. **Implement chat tabs** for different channels
4. **Add emoji/emoticon support**
5. **Integrate with your game's social features**
6. **Add chat history persistence** (save to disk)
7. **Implement admin/moderator commands**

For more information, see the main [README.md](README.md) file.
