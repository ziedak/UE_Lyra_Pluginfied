#include "Data/GasGameData.h"
#include "Core/GAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GasGameData)

const UGasGameData& UGasGameData::Get()
{
	return UGAssetManager::Get().GetGameData();
}
