#include "DialogueOptionEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

bool UDialogueOptionEntry::Initialize()
{
    const bool bResult = Super::Initialize();

    if (OptionButton)
    {
        OptionButton->OnClicked.AddDynamic(this, &UDialogueOptionEntry::HandleButtonClicked);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OptionButton is null in UDialogueOptionEntry::Initialize"));
    }

    return bResult;
}

void UDialogueOptionEntry::SetupOption(const FText& InOptionText, int32 InOptionIndex)
{
    OptionIndex = InOptionIndex;

    if (OptionText)
    {
        OptionText->SetText(InOptionText);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OptionText is null in UDialogueOptionEntry::SetupOption"));
    }
}

void UDialogueOptionEntry::HandleButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Dialogue option entry clicked. Index: %d"), OptionIndex);
    OnDialogueOptionClicked.Broadcast(OptionIndex);
}