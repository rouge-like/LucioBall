#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyTextButtonWidget.generated.h"

class UButton;
class UTextBlock;
class UWidgetAnimation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyButtonClicked);

UCLASS()
class LUCIOBALL_API ULobbyTextButtonWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnLobbyButtonClicked OnClickedDelegate;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> MainButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ButtonText;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* MouseOverAnimation;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* MouseClickAnimation;

private:
    UFUNCTION()
    void OnMainButtonClicked();
};