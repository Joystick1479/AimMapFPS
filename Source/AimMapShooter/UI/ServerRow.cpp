// Bartosz Jastrzebski


#include "ServerRow.h"

#include "Components/Button.h"

#include "MyUserWidget.h"

void UServerRow::Setup(UMyUserWidget * InParent, uint32 InIndex)
{
	Parent = InParent;
	Index = InIndex;
	RowButton->OnClicked.AddDynamic(this, &UServerRow::OnClicked);
}

void UServerRow::OnClicked()
{
	Parent->SelectIndex(Index);
}
