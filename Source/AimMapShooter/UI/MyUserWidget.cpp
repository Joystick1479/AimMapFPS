// Bartosz Jastrzebski


#include "MyUserWidget.h"

#include "Components/Button.h"

bool UMyUserWidget::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;

	if (!ensure(Host != nullptr)) return false;
	Host->OnClicked.AddDynamic(this, &UMyUserWidget::HostServer);

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