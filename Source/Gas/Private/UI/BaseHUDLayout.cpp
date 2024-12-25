#include "UI/BaseHUDLayout.h"
#include "CommonUIExtensions.h"
#include "Tags/BaseGameplayTags.h"
#include "Input/CommonUIInputTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseHUDLayout)


void UBaseHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(UITags::ACTION_ESCAPE), false,
	                                          FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
}

void UBaseHUDLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(GetOwningLocalPlayer(),
		                                                          UITags::LAYER_MENU,
		                                                          EscapeMenuClass);
	}
}
