# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RuinLooters is an Unreal Engine 5.5 action RPG game featuring combat, character progression, and item collection mechanics. The game includes multiple character types, AI-controlled enemies, weapon systems, and a stage-based progression system.

## Development Commands

### Building the Project
- Use Unreal Engine Editor to build the project (File -> Compile or hot reload via Live Coding)
- For full rebuilds: right-click on the .uproject file and select "Generate Visual Studio Project Files"
- Build configurations: DebugGame, Development, Shipping

### Testing
- Use Unreal Engine's PIE (Play in Editor) for quick testing
- Package the game for standalone testing: File -> Package Project

## Architecture Overview

### Core Character System
The game uses a hierarchical character system with clear inheritance:

- **ARuinLootersCharacter**: Base Unreal character class
- **ARLCharacterBase**: Core game character with combat, stats, and weapon systems
  - Implements IRLCharacterAttackInterface for combat interactions
  - Manages HP, damage, defense, money, and weapon systems
  - Handles combo attacks, rolling mechanics, and invincibility frames
- **ARLCharacterPlayer**: Player character extending RLCharacterBase
  - Implements IGenericTeamAgentInterface for team-based AI
  - Adds input handling, UI management, skill systems, and glider mechanics
  - Features dual weapon forms (sword/bow) with aiming system
- **ARLCharacterEnemy**: Enemy base class with AI integration
- **ARLCharacterEnemyDragon**: Specialized dragon enemy with unique abilities

### Key Systems

#### Data Management
- **URLGameInstance**: Central data hub managing all DataTables (weapons, potions, skill books, enemy abilities)
- **URLSaveGame**: Persistent save system for player progression
- Data-driven design using UDataTable for all item and enemy configurations

#### Combat System
- Interface-based attack system (IRLCharacterAttackInterface)
- Combo attack system with data assets (URLPlayerComboAttackDataAsset)
- Dual weapon forms: sword (melee) and bow (ranged with aiming)
- Projectile system with object pooling for performance

#### AI System
Located in `/AI/` directory:
- Behavior Tree tasks for enemy actions (attack, movement, face target)
- AI services for distance checking and HP monitoring
- Specialized dragon AI with flying mechanics

#### Item and Weapon System
- DataTable-driven weapon system with dynamic weapon changing
- Item boxes and NPCs for trading (weapons, potions, skill books)
- Object pooling for arrows and projectiles

#### UI System
Located in `/UI/` directory:
- Player HUD with HP bars and skill cooldowns
- NPC store interfaces for item trading
- Settings and death screen management
- Stage progression widgets

### Code Organization

The codebase follows Unreal Engine conventions:
- **Header files (.h)**: Class declarations with UPROPERTY/UFUNCTION macros
- **Implementation files (.cpp)**: Method implementations
- **Data Assets**: Used for configurable game data (combo attacks, player stats)
- **Blueprints**: Referenced in C++ for visual scripting integration

### Key Directories
- `/Character/`: All character-related classes and components
- `/AI/`: Behavior trees, AI controllers, and AI tasks
- `/Weapon/`: Weapon base classes and implementations
- `/Item/`: Collectible items and item boxes
- `/Projectile/`: Arrow and projectile systems with pooling
- `/UI/`: User interface widgets and HUD elements
- `/GameInstance/`: Game instance and data management
- `/Pool/`: Object pooling systems for performance

### Important Design Patterns
- **Data-driven design**: Heavy use of DataTables for configuration
- **Object pooling**: Used for frequently spawned objects (arrows, projectiles)
- **Interface segregation**: Combat and team interfaces for clean interactions
- **Delegate system**: Used for character death and UI updates
- **Component-based**: Glider system implemented as a component

## Korean Language Notes
The codebase contains Korean comments and variable names. Key terms:
- 체력 (HP), 공격력 (Attack Power), 방어력 (Defense)
- 무기 (Weapon), 적 (Enemy), 플레이어 (Player)
- 스킬 (Skill), 스테이지 (Stage), 레벨 (Level)