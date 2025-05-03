// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/ReplicationGraphSettings.h"
#include "ReplicationGraph/BaseReplicationGraph.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ReplicationGraphSettings)


UReplicationGraphSettings::UReplicationGraphSettings()
{
	CategoryName = TEXT("Game");
	DefaultReplicationGraphClass = UBaseReplicationGraph::StaticClass();
}
