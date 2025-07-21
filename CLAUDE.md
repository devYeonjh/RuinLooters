# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RuinLooters is a 3D action RPG game built with Unreal Engine 5.5. Key features include:
- Third-person combat with melee and ranged weapons
- Enemy AI with behavior trees
- Gliding/flying mechanics
- Save/load system
- Inventory and merchant NPCs
- Level progression through portals

## Build & Development Commands

### Opening the Project
```bash
# Open project in UE5 Editor (adjust path as needed)
"C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealEditor.exe" "D:\UE5\RuinLooters\RuinLooters.uproject"
```

### Building the Project
```bash
# Generate Visual Studio project files
"C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "D:\UE5\RuinLooters\RuinLooters.uproject"

# Build from Visual Studio
# Open RuinLooters.sln and build the RuinLooters project
```

### Packaging
```bash
# Package for Windows (from UE5 Editor)
# File -> Package Project -> Windows -> Windows (64-bit)
```

## Architecture Overview

### Module Structure
- **RuinLooters** - Main game module
  - Dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, UMG, LevelSequence, MovieScene, Niagara, ACERuntime, ACECore, A2FLocal, Http, Json, JsonUtilities, RuntimeAudioImporter

### Core Systems

1. **Character System** (`Source/RuinLooters/Character/`)
   - `RLCharacterBase` - Base class for all characters
   - `RLCharacterPlayer` - Player character with input handling
   - `RLCharacterEnemy` - Base enemy class
   - `RLCharacterEnemyDragon` - Dragon boss enemy
   - NPCs: `RLWeaponNPC`, `RLPotionNPC`, `RLSkillBookNPC`

2. **Combat System**
   - Combo attack system using `URLPlayerComboAttackDataAsset`
   - Projectile system with object pooling (`RLProjectilePool`, `RLArrowPool`)
   - Weapon switching between sword and bow
   - Roll/dodge mechanics with invincibility frames

3. **AI System** (`Source/RuinLooters/AI/`)
   - Behavior Trees: `BT_HumanEnemy`, `BT_DragonEnemyGround`, `BT_DragonEnemySky`
   - Custom BT tasks for attacking, flying, landing
   - AI Controllers: `RLEnemyAIController`, `RLHumanEnemyAIController`, `RLDragonEnemyAIController`

4. **Data Management**
   - DataTables for weapons, potions, skill books, enemy abilities
   - Save/Load system using `URLSaveGame`
   - Game instance (`URLGameInstance`) manages persistent data

5. **UI System** (`Source/RuinLooters/UI/`)
   - HUD with HP bar and skill icons
   - Store/merchant UI
   - Main menu and settings
   - Death and stage clear screens

6. **TTS System** (`Source/RuinLooters/TTSxA2F/`)
   - `RLCosyVoiceClient` - HTTP client for CosyVoice TTS server
   - `RLTTSManager` - High-level TTS management with A2F integration
   - `RLVoiceRecorder` - Voice recording for speaker presets
   - `RLA2FComponent` - Audio2Face integration for facial animation

### Plugin Dependencies

1. **NV_ACE_Reference** - NVIDIA AI Character Engine
   - Audio2Face for facial animation
   - GPT integration for AI dialogue
   - Live Link support

2. **RuntimeAudioImporter** - Dynamic audio loading

3. **NvAudio2Face[Claire/James/Mark]** - Character-specific A2F models

## Key Development Patterns

### Collision Profiles
```cpp
// Custom collision channels defined in DefaultEngine.ini
Enemy (ECC_GameTraceChannel1)
Arrow (ECC_GameTraceChannel2) 
Player (ECC_GameTraceChannel3)
```

### Character Stats Structure
```cpp
// Weapons use FWeaponTableRow
// Potions use FPotionTableRow
// Skill books use FSkillBookTableRow
// Enemies use FEnemyAbilityTableRow
```

### Input System
- Uses Enhanced Input with `IMC_Default` input mapping context
- Player controller manages input through `RLPlayerController`

### Object Pooling
- Projectiles and arrows are pooled for performance
- Pools managed by game mode

## Common Development Tasks

### Adding New Weapons
1. Add entry to `DT_Weapon` DataTable
2. Create skeletal mesh and assign to table
3. Weapon stats (damage, range, speed) configured in table

### Creating New Enemies
1. Inherit from `RLCharacterEnemy`
2. Create behavior tree and blackboard
3. Add entry to `DT_EnemyAbility` DataTable
4. Set up AI controller

### Implementing New Skills
1. Create animation montage
2. Add skill to player's skill set
3. Create UI icon and bind to input

### Level Transitions
- Use `RLLevelTransferPortal` actors
- Portals handle save game state before transition

### TTS Integration
1. **Server Setup**: Start CosyVoice server with `python run_server.py --port 50001`
2. **Basic TTS**: Use `RLTTSManager` component for high-level TTS operations
3. **Speaker Presets**: Record voice samples using `RLVoiceRecorder` component
4. **A2F Integration**: Combine TTS with facial animation using `RLA2FComponent`

#### TTS Component Usage Example
```cpp
// Get TTS manager component
URLTTSManager* TTSManager = GetComponentByClass<URLTTSManager>();

// Simple TTS
TTSManager->SpeakText(TEXT("Hello world!"));

// TTS with specific speaker and A2F
TTSManager->SpeakText(TEXT("Welcome to the game!"), TEXT("my_voice"), true);

// Queue multiple TTS requests
TTSManager->QueueTextToSpeak(TEXT("First message"));
TTSManager->QueueTextToSpeak(TEXT("Second message"));
```

#### Voice Recording Example
```cpp
// Get voice recorder
ARLVoiceRecorder* VoiceRecorder = GetWorld()->SpawnActor<ARLVoiceRecorder>();

// Record 3 seconds of audio
VoiceRecorder->StartRecording(3.0f);

// Save as voice preset when recording completes
VoiceRecorder->OnRecordingComplete.AddDynamic(this, [](const TArray<uint8>& AudioData)
{
    // Save recorded audio as voice preset
    VoiceRecorder->SaveAsVoicePreset(TEXT("player_voice"), TEXT("Hello world"));
});
```

## Important File Locations

- **Blueprints**: `Content/Blueprints/`
- **Character Assets**: `Content/Characters/`, `Content/MetaHumans/`
- **UI Widgets**: `Content/UMG/`
- **Data Tables**: `Content/DataTables/`
- **Maps**: `Content/Model/MWLandscapeAutoMaterial/Maps/`
- **Enemy AI**: `Content/Enemy/`
- **TTS System**: `Source/RuinLooters/TTSxA2F/`

## Testing & Debugging

### Console Commands
- Access console with ` (tilde) key
- Common commands:
  - `stat fps` - Show FPS
  - `stat unit` - Show frame timing
  - `show collision` - Visualize collision

### AI Debugging
- Use AI Debug mode in editor (apostrophe key)
- Behavior tree visualization available in editor

## Performance Considerations

- Object pooling used for projectiles to reduce GC
- LOD settings configured for characters and environment
- Niagara used for particle effects
- Audio streaming handled by RuntimeAudioImporter