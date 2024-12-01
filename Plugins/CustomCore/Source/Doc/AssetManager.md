// Get the global Asset Manager
UAssetManager& AssetManager = UAssetManager::Get();
// Get a list of all weapons that can be loaded AssetManager.GetPrimaryAssetIdList (WeaponItemType, WeaponIdList);
// Get tag/value data for an unloaded weapon
AssetManager.GetPrimaryAssetData (WeaponIdList[0], AssetDataToParse);
// Permanently load a single item
FPrimaryAssetId WeaponId = FPrimaryAssetId (WeaponItemType, WeaponName); AssetManager.LoadPrimaryAsset (WeaponId, CurrentLoadState[AssetBundles], DelegateFunction);
// From DelegateFunction
UWeaponItem* Weapon = AssetManager.GetPrimaryAssetObject<UWeaponItem> (WeaponId);
// Release previously loaded item
AssetManager. UnloadPrimaryAsset (WeaponId);
// Load a list of items as long as Handle is alive
Handle = AssetManager. PreloadPrimaryAssets (ListOfPrimaryAssetIds, CurrentLoadState, false);

AssetBundles
Load Related resources only when needed
AssetBundeles meta tag on soft ref inside UPrimaryDataAsset
Override UpdareAssetBundleData for complex cases
Update some With ChangeBundleStateForPrimaryAssets will async load
Update everything with ChangeBndleStateForMatchingPrimaruAssets

Loadind Best Practices
Only sync load during load screens or as fast response to input
Change bundle state and clear caches during mode switches
Use StreamableHandles to sync load or keep in memory
Load eith Delegates vs Preload plus fallback
Never use unsafe lambdas as delegates
Samples ans existing systems have room for improvement 
Set chunks 
    chunk0 :game startup needs
    chunk1: main menu
    chunk2: reste of primaryAsset

//in data asset not assetmanager
// MetaData to read off disk
UPROPERTY(EditAnywhere,BlueprintReadOnly,Category=Item,AssetRegistrySearchable)
Fname RegistryTag;


Advanced Uses
Build a startup loading state nachine
    Overrride state viruals with custon logic, use exposed utility functions
Preloid assets before map change
    Build custon bundles based on asset registry, or do manually
On-demande chunk doanloading
    Enable with bShouldAcquireMissingChunkOnLoad after setting up IPlatformChunkInstall
    Other support for manually mounted chunks
Output custum reports
    Use DumpReferencesForPackage as an example 


AssetLabel 
is a PrimaryAssetType to set up priority , chunk id cook rule ,..., is to create pak  files for remote download 
Refs
Asset Manager Explained : https://www.youtube.com/watch?v=9MGHBU5eNu0
ChunkDownloader Explained :https://www.youtube.com/watch?v=h3A8qVb2VFk

Code
AssetManagetExp.h

virtual void StarInitialLoading() override;
// Static Types for Items
static const FprimatyAssetType PotionItemType;
static const FprimatyAssetType WeaponItemType;

AssetManagetExp.cpp

 AssetManager& Am=AssetManager::Get();
 Get a list of all weapons that can be loaded
 TArray(FRimaryAssetId) List
 Am.GetPrimaryAssetIdList (WeaponItemType, 
  );
 for(auto id:WeaponList)


