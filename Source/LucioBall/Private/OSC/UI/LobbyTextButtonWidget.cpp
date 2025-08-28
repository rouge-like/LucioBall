#include "OSC/UI/LobbyTextButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void ULobbyTextButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (MainButton)
    {
        MainButton->OnClicked.AddDynamic(this, &ULobbyTextButtonWidget::OnMainButtonClicked);
    }
}

void ULobbyTextButtonWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    
    if (MouseOverAnimation)
    {
        PlayAnimation(MouseOverAnimation);
    }
}

void ULobbyTextButtonWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    if (MouseOutAnimation)
    {
        PlayAnimation(MouseOutAnimation);
    }
}

void ULobbyTextButtonWidget::OnMainButtonClicked()
{
    if (MouseClickAnimation)
    {
        PlayAnimation(MouseClickAnimation);
    }
    
    OnClickedDelegate.Broadcast();
}
