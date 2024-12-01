// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/CoreTags.h"


namespace InitStateTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SPAWNED, "InitState.Spawned",
	                               "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DATA_AVAILABLE, "InitState.DataAvailable",
	                               "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DATA_INITIALIZED, "InitState.DataInitialized",
	                               "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay")
	;
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GAMEPLAY_READY, "InitState.GameplayReady",
	                               "4: The actor/component is fully ready for active gameplay");
}
