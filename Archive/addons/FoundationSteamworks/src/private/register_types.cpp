#include "register_types.h"
#include "AchievementData.h"
#include "ClanData.h"
#include "DlcData.h"
#include "ItemData.h"
#include "LeaderboardData.h"
#include "LeaderboardEntryData.h"
#include "LobbyData.h"
#include "LobbyQuery.h"
#include "LobbyType.h"
#include "SteamApi.h"
#include "SteamInitialisationResponse.h"
#include "SteamInventoryItemDetail.h"
#include "SteamResult.h"
#include "SteamTimedTrial.h"
#include "StatData.h"
#include "SteamworksSubsystem.h"
#include "UserData.h"

#include <gameframework/SubsystemManager.h>

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_foundation_steam_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
        return;

    // Subsystem singleton (instance created by AutoLoad scene)
    ClassDB::register_class<SteamApi>();

    // Artifact types
    ClassDB::register_class<UserData>();
    ClassDB::register_class<LobbyData>();
    ClassDB::register_class<DlcData>();
    ClassDB::register_class<AchievementData>();
    ClassDB::register_class<StatData>();
    ClassDB::register_class<LeaderboardData>();
    ClassDB::register_class<LeaderboardEntryData>();
    ClassDB::register_class<ItemData>();
    ClassDB::register_class<ClanData>();
    ClassDB::register_class<SteamInventoryItemDetail>();

    // Lobby helpers
    ClassDB::register_class<LobbyQuery>();
    ClassDB::register_class<LobbyType>();
    ClassDB::register_class<LobbyUseHint>();
    ClassDB::register_class<LobbyDistance>();
    ClassDB::register_class<LobbyComparison>();
    ClassDB::register_class<LobbyEnterResponse>();

    // Value types
    ClassDB::register_class<SteamInitialisationResponse>();
    ClassDB::register_class<SteamResult>();
    ClassDB::register_class<SteamTimedTrial>();
    ClassDB::register_class<SteamLeaderboardDisplay>();

    // Real gameframework::Subsystem registration — see Godot-Game-Framework's
    // README, "The linking model", and FoundationGameplayTags' register_types.cpp
    // for the reference implementation this mirrors.
    gameframework::SubsystemManager::instance().register_subsystem<SteamworksSubsystem>();
}

void uninitialize_foundation_steam_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
        return;
    // Singleton cleanup is handled by _exit_tree() when the AutoLoad node leaves the scene tree
}

extern "C"
{
    GDE_EXPORT GDExtensionBool foundation_steam_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address,
        GDExtensionClassLibraryPtr p_library,
        GDExtensionInitialization *r_initialization)
    {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_foundation_steam_module);
        init_obj.register_terminator(uninitialize_foundation_steam_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
