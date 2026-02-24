#include "DunGenOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UDunGenOverlay::SetHealthBarPercent(float Percent)
{
    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(Percent);
    }
}
void UDunGenOverlay::SetStaminaBarPercent(float Percent)
{
    if (StaminaProgressBar)
    {
        StaminaProgressBar->SetPercent(Percent);
    }
}


void UDunGenOverlay::SetGold(int32 Gold)
{
    if (GoldText)
    {
        const FString String = FString::Printf(TEXT("%d"), Gold);
        const FText Text = FText::FromString(String);
        GoldText->SetText(Text);
    }
}

void UDunGenOverlay::SetWisdom(int32 Wisdom)
{
    if (WisdomText)
    {
        const FString String = FString::Printf(TEXT("%d"), Wisdom);
        const FText Text = FText::FromString(String);
        WisdomText->SetText(Text);
    }
}

void UDunGenOverlay::ShowInteractButton()
{
    FLinearColor WhiteHalfOpacity = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
    if (InteractBorder)
    {
        InteractBorder->SetContentColorAndOpacity(WhiteHalfOpacity);
    }
}

void UDunGenOverlay::HideInteractButton()
{
    FLinearColor WhiteNoOpacity = FLinearColor(1.f, 1.f, 1.f, 0.f); 
    if (InteractBorder)
    {
        InteractBorder->SetContentColorAndOpacity(WhiteNoOpacity);
    }
}