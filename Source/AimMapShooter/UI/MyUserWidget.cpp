// Bartosz Jastrzebski


#include "MyUserWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

bool UMyUserWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (!ensure(HostButton != nullptr)) return false;
	HostButton->OnClicked.AddDynamic(this, &UMyUserWidget::HostServer);

	if (!ensure(JoinButton!= nullptr)) return false;
	JoinButton->OnClicked.AddDynamic(this, &UMyUserWidget::OpenJoinMenu);

	if (!ensure(CancelJoinMenuButton != nullptr)) return false;
	CancelJoinMenuButton->OnClicked.AddDynamic(this, &UMyUserWidget::OpenMainMenu);

	return true;
}

void UMyUserWidget::HostServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Hosting")); 
	if (MenuInterface != nullptr)
	{
		MenuInterface->Host();
	}
}

void UMyUserWidget::OpenJoinMenu()
{
	if (!ensure(MenuSwitcher != nullptr)) return;
	if (!ensure(JoinMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(JoinMenu);
}

void UMyUserWidget::OpenMainMenu()
{
	if (!ensure(MenuSwitcher != nullptr)) return;
	if (!ensure(MainMenu != nullptr)) return;

	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMyUserWidget::SetMenuInterface(IMainMenuInterface* MenuInterface)
{
	this->MenuInterface = MenuInterface;
}

void UMyUserWidget::Setup()
{
	this->AddToViewport();
	
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!ensure(PC != nullptr)) return;

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = true;
}

void UMyUserWidget::TearDown()
{
	this->RemoveFromViewport();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!ensure(PC != nullptr)) return;

	FInputModeGameOnly InputModeData;
	PC->SetInputMode(InputModeData);
	PC->bShowMouseCursor = false;

}