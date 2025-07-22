#include "RLDialogueTypes.h"

FString FConversationContext::GetFullContext() const
{
	FString Context;
	
	// Add NPC personality info
	Context += FString::Printf(TEXT("Character Name: %s\n"), *NPCPersonality.CharacterName);
	Context += FString::Printf(TEXT("Personality: %s\n"), *NPCPersonality.PersonalityPrompt);
	
	if (!NPCPersonality.BackgroundStory.IsEmpty())
	{
		Context += FString::Printf(TEXT("Background: %s\n"), *NPCPersonality.BackgroundStory);
	}
	
	// Add knowledge base
	if (NPCPersonality.KnowledgeBase.Num() > 0)
	{
		Context += TEXT("Known topics: ");
		for (int32 i = 0; i < NPCPersonality.KnowledgeBase.Num(); i++)
		{
			Context += NPCPersonality.KnowledgeBase[i];
			if (i < NPCPersonality.KnowledgeBase.Num() - 1)
			{
				Context += TEXT(", ");
			}
		}
		Context += TEXT("\n");
	}
	
	// Add location and time context
	Context += FString::Printf(TEXT("Location: %s\n"), *CurrentLocation);
	Context += FString::Printf(TEXT("Conversation started: %s\n"), *ConversationStartTime.ToString());
	
	return Context;
}