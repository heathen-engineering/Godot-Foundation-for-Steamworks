#include "SteamApi.h"
#include "AchievementData.h"
#include "DlcData.h"
#include "LobbyData.h"
#include "SteamInitialisationResponse.h"
#include "SteamResult.h"
#include "SteamTimedTrial.h"
#include "UserData.h"
#include "LeaderboardEntryData.h"
#include "SteamInventoryItemDetail.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

SteamApi *SteamApi::singleton = nullptr;

namespace
{
/// Editor-only RAII guard telling the Steamworks SDK which App ID to init against via
/// the SteamAppId environment variable, instead of steam_appid.txt. Avoids a file on
/// disk that can go stale, get forgotten, or drift out of sync with AppId — ported
/// from O3DE's FoundationSteamworksSystemComponent.cpp ScopedSteamAppIdEnv. No-ops
/// outside the editor (packaged builds are launched through Steam, which already
/// knows the App ID) and restores whatever value was previously set on scope exit.
struct ScopedSteamAppIdEnv
{
    bool active = false;
    bool had_previous = false;
    String previous;

    explicit ScopedSteamAppIdEnv(int app_id, bool in_editor)
    {
        if (app_id <= 0 || !in_editor)
            return;

        OS *os = OS::get_singleton();
        had_previous = os->has_environment("SteamAppId");
        if (had_previous)
            previous = os->get_environment("SteamAppId");
        os->set_environment("SteamAppId", String::num_int64(app_id));
        active = true;
    }

    ~ScopedSteamAppIdEnv()
    {
        if (!active)
            return;
        OS *os = OS::get_singleton();
        if (had_previous)
            os->set_environment("SteamAppId", previous);
        else
            os->unset_environment("SteamAppId");
    }
};
} // namespace

void SteamApi::ClientArtifactLoad()
{
    if (singleton->debug) UtilityFunctions::print("[SteamApi] Loading client artifacts");

    const int DlcCount = SteamApps()->GetDLCCount();
    if (singleton->debug) UtilityFunctions::print("[SteamApi] DLC entries: ", DlcCount);
    for (int i = 0; i < DlcCount; ++i)
    {
        AppId_t app_id = 0;
        bool available = false;
        char name[128];
        SteamApps()->BGetDLCDataByIndex(i, &app_id, &available, name, 128);
        singleton->DlcCollection.append(DlcData::GetDlcData(app_id, available, name));
    }

    if (singleton->is_client && !singleton->leaderboardIds.is_empty())
    {
        for (int i = 0; i < singleton->leaderboardIds.size(); ++i)
        {
            const String leaderboardId = singleton->leaderboardIds[i];
            if (leaderboardId.is_empty())
                continue;

            LeaderboardFindRequest request;
            request.leaderboard = leaderboardId;
            request.createIfMissing = false;
            request.lowestScoreIsTopRank = false;
            request.displayType = SteamLeaderboardDisplay::Type::Numeric;
            singleton->LeaderboardFindRequests.push(request);
        }

        if (!singleton->LeaderboardFindRequests.empty())
        {
            if (singleton->debug)
                UtilityFunctions::print("[SteamApi] Resolving ", (int)singleton->LeaderboardFindRequests.size(), " leaderboard(s)");
            singleton->m_LeaderboardPending = singleton->LeaderboardFindRequests.front();
            singleton->LeaderboardFindRequests.pop();
            const auto handle = SteamUserStats()->FindLeaderboard(singleton->m_LeaderboardPending.leaderboard.utf8().get_data());
            singleton->m_LeaderboardFindResult_t.Set(handle, singleton, &SteamApi::OnLeaderboardFindResult);
        }
        else
        {
            if (singleton->debug) UtilityFunctions::print("[SteamApi] Ready");
            singleton->IsReady = true;
            singleton->emit_signal("OnReady");
        }
    }
    else
    {
        if (singleton->debug) UtilityFunctions::print("[SteamApi] Ready");
        singleton->IsReady = true;
        singleton->emit_signal("OnReady");
    }
}

Dictionary SteamApi::GameServerItemToDictionary(gameserveritem_t *serverItem)
{
    Dictionary d;
    if (serverItem)
    {
        d["name"] = SteamAPI_gameserveritem_t_GetName(serverItem);
        d["app_id"] = serverItem->m_nAppID;
        d["players"] = serverItem->m_nPlayers;
        d["max_players"] = serverItem->m_nMaxPlayers;
        d["ping"] = serverItem->m_nPing;
        d["secure"] = serverItem->m_bSecure;
        d["map"] = serverItem->m_szMap;
        d["game_tags"] = serverItem->m_szGameTags;
        d["steam_id"] = (uint64_t)serverItem->m_steamID.ConvertToUint64();
    }
    return d;
}

SteamApi::SteamApi()
{
    singleton = this;
    SelfInitialised = false;
    IsReady = false;
    InventorySnapshotDirty = false;
    PendingInventoryNotificationCallable = false;
    PendingInventoryFullRefresh = false;
}

SteamApi::~SteamApi()
{
    SteamAPI_Shutdown();
}

void SteamApi::_init()
{
    AvatarCache.clear();
    AvatarRequests.clear();
    MemberOfLobbies.clear();
    DlcCollection.clear();
    IsReady = false;
    singleton = this;
}

SteamApi *SteamApi::GetSingleton() { return singleton; }
PackedInt64Array SteamApi::GetMemberOfLobbies() { return singleton->MemberOfLobbies; }
bool SteamApi::GetIsReady() { return singleton && singleton->IsReady; }
void SteamApi::SetSingleton(SteamApi *p_singleton) { singleton = p_singleton; }

void SteamApi::SetFullConfiguration(int app_id, const String &ip_address, int game_port, int query_port, bool bVACEnabled, const String &game_version)
{
    singleton->appId = app_id;
    singleton->ipAddress = ip_address;
    singleton->gamePort = game_port;
    singleton->queryPort = query_port;
    singleton->bVACEnabled = bVACEnabled;
    singleton->gameVersion = game_version;
}

void SteamApi::SetMinimalConfiguration(int app_id)
{
    singleton->appId = app_id;
    singleton->ipAddress = "0.0.0.0";
    singleton->gamePort = 27015;
    singleton->queryPort = 27016;
    singleton->bVACEnabled = false;
    singleton->gameVersion = "1.0.0.0";
}

Ref<SteamInitialisationResponse> SteamApi::InitialiseClient()
{
    singleton->is_client = true;
    singleton->initialisationResponse = memnew(SteamInitialisationResponse);

    if (singleton->debug) UtilityFunctions::print("[SteamApi] Beginning client initialisation (App ID: ", singleton->appId, ")");

    if (!SteamAPI_IsSteamRunning())
    {
        UtilityFunctions::printerr("[SteamApi] Aborting: Steam is not running");
        singleton->initialisationResponse->bSuccess = false;
        singleton->initialisationResponse->message = "Steam must be running before you can initialise";
        return singleton->initialisationResponse;
    }
    if (singleton->debug) UtilityFunctions::print("[SteamApi] Steam is running");

    const bool in_editor = Engine::get_singleton()->has_singleton("EditorInterface");
    ScopedSteamAppIdEnv appIdGuard(singleton->appId, in_editor);
    if (in_editor)
    {
        if (singleton->debug) UtilityFunctions::print("[SteamApi] Editor detected; SteamAppId env var set to ", singleton->appId);
    }

    bool requiresRestart = false;
    if (!in_editor)
    {
        requiresRestart = SteamAPI_RestartAppIfNecessary(static_cast<AppId_t>(singleton->appId));
        singleton->initialisationResponse->bShouldRestart = requiresRestart;
    }

    if (!requiresRestart)
    {
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
        SteamErrMsg errMsg;
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK)
        {
            UtilityFunctions::printerr("[SteamApi] SteamAPI_Init failed: ", errMsg);
            singleton->initialisationResponse->bSuccess = false;
            singleton->initialisationResponse->message = errMsg;
        }
        else
        {
            if (singleton->debug) UtilityFunctions::print("[SteamApi] SteamAPI_Init succeeded");
            singleton->initialisationResponse->bSuccess = true;
        }
#else
        singleton->initialisationResponse->bSuccess = SteamAPI_Init();
        if (singleton->debug)
        {
            if (singleton->initialisationResponse->bSuccess) UtilityFunctions::print("[SteamApi] SteamAPI_Init succeeded");
            else UtilityFunctions::printerr("[SteamApi] SteamAPI_Init failed");
        }
#endif
    }
    else
    {
        UtilityFunctions::printerr("[SteamApi] Aborting: Steam requires the app to restart");
        singleton->initialisationResponse->bSuccess = false;
        singleton->initialisationResponse->message = "Steam restart required";
    }

    if (singleton->initialisationResponse->bSuccess)
        ClientArtifactLoad();

    return singleton->initialisationResponse;
}

Ref<SteamInitialisationResponse> SteamApi::InitialiseServer()
{
    singleton->is_client = false;
    singleton->initialisationResponse = memnew(SteamInitialisationResponse);

    if (singleton->debug) UtilityFunctions::print("[SteamApi] Beginning server initialisation (App ID: ", singleton->appId, ", port: ", singleton->gamePort, ")");

    const bool in_editor = Engine::get_singleton()->is_editor_hint();
    ScopedSteamAppIdEnv appIdGuard(singleton->appId, in_editor);
    if (in_editor)
    {
        if (singleton->debug) UtilityFunctions::print("[SteamApi] Editor detected; SteamAppId env var set to ", singleton->appId);
    }

    uint16_t safe_game_port = static_cast<uint16_t>(Math::clamp(singleton->gamePort, 0, 65535));
    uint16_t safe_query_port = static_cast<uint16_t>(Math::clamp(singleton->queryPort, 0, 65535));
    const EServerMode mode = singleton->bVACEnabled ? eServerModeAuthenticationAndSecure : eServerModeAuthentication;
    const char *version_cstr = singleton->gameVersion.utf8().get_data();

#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    SteamErrMsg errMsg;
    if (SteamGameServer_InitEx(0, safe_game_port, safe_query_port, mode, version_cstr, &errMsg) != k_ESteamAPIInitResult_OK)
    {
        UtilityFunctions::printerr("[SteamApi] SteamGameServer_Init failed: ", errMsg);
        singleton->initialisationResponse->bSuccess = false;
        singleton->initialisationResponse->message = errMsg;
    }
    else
    {
        if (singleton->debug) UtilityFunctions::print("[SteamApi] SteamGameServer_Init succeeded");
        singleton->initialisationResponse->bSuccess = true;
    }
#else
    singleton->initialisationResponse->bSuccess = SteamGameServer_Init(0, safe_game_port, safe_query_port, mode, version_cstr);
#endif

    singleton->initialisationResponse->bShouldRestart = false;
    singleton->IsReady = true;
    if (singleton->debug) UtilityFunctions::print("[SteamApi] Ready");

    if (singleton->initialisationResponse->bSuccess && singleton->autoLogOn)
    {
        if (singleton->debug) UtilityFunctions::print("[SteamApi] Logging on anonymously");
        SteamGameServer()->LogOnAnonymous();
    }

    return singleton->initialisationResponse;
}

// ISteamMatchmakingServerListResponse
void SteamApi::ServerResponded(HServerListRequest h, int s) { emit_signal("OnServerRequestResponded", reinterpret_cast<uint64_t>(h), s); }
void SteamApi::ServerFailedToRespond(HServerListRequest h, int s) { emit_signal("OnServerRequestFailedToRespond", reinterpret_cast<uint64_t>(h), s); }
void SteamApi::RefreshComplete(HServerListRequest h, EMatchMakingServerResponse r) { emit_signal("OnRefreshRequestComplete", reinterpret_cast<uint64_t>(h), r); }
void SteamApi::ServerResponded(gameserveritem_t &server) { emit_signal("OnPingServerResponded", GameServerItemToDictionary(&server)); }
void SteamApi::ServerFailedToRespond() { emit_signal("OnPingServerFailedToRespond"); }
void SteamApi::AddPlayerToList(const char *name, int score, float time) { emit_signal("OnAddPlayerDetailsToList", String(name), score, time); }
void SteamApi::PlayersFailedToRespond() { emit_signal("OnPlayerDetailsFailedToRespond"); }
void SteamApi::PlayersRefreshComplete() { emit_signal("OnPlayerDetailsRefreshComplete"); }
void SteamApi::RulesResponded(const char *rule, const char *value) { emit_signal("OnServerRulesResponded", String(rule), String(value)); }
void SteamApi::RulesFailedToRespond() { emit_signal("OnServerRulesFailedToRespond"); }
void SteamApi::RulesRefreshComplete() { emit_signal("OnServerRulesRefreshComplete"); }

// Properties
void SteamApi::set_debug(bool v) { debug = v; }
bool SteamApi::get_debug() const { return debug; }
void SteamApi::set_appId(int v) { appId = v; }
void SteamApi::set_ipAddress(const String &v) { ipAddress = v; }
void SteamApi::set_gamePort(int v) { gamePort = v; }
void SteamApi::set_queryPort(int v) { queryPort = v; }
void SteamApi::set_bVACEnabled(bool v) { bVACEnabled = v; }
void SteamApi::set_gameVersion(const String &v) { gameVersion = v; }
void SteamApi::set_autoInitialise(bool v) { autoInitialise = v; }
void SteamApi::set_autoLogOn(bool v) { autoLogOn = v; }
void SteamApi::set_leaderboardIds(const PackedStringArray &v) { leaderboardIds = v; }
int SteamApi::get_appId() const { return appId; }
String SteamApi::get_ipAddress() const { return ipAddress; }
int SteamApi::get_gamePort() const { return gamePort; }
int SteamApi::get_queryPort() const { return queryPort; }
bool SteamApi::get_bVACEnabled() const { return bVACEnabled; }
String SteamApi::get_gameVersion() const { return gameVersion; }
bool SteamApi::get_autoInitialise() const { return autoInitialise; }
bool SteamApi::get_autoLogOn() const { return autoLogOn; }
PackedStringArray SteamApi::get_leaderboardIds() const { return leaderboardIds; }

// --- App ---
int SteamApi::GetAppBuildId() { return GetIsReady() ? SteamApps()->GetAppBuildId() : 0; }

String SteamApi::GetAppInstallDir(int appId)
{
    if (!GetIsReady()) return "";
    char path[512];
    return SteamApps()->GetAppInstallDir(static_cast<uint32_t>(appId), path, 512) ? String(path) : "";
}

Ref<UserData> SteamApi::GetAppOwner()
{
    return GetIsReady() ? Ref<UserData>(memnew(UserData(SteamApps()->GetAppOwner()))) : Ref<UserData>(memnew(UserData()));
}

String SteamApi::GetAvailableGameLanguages() { return GetIsReady() ? String(SteamApps()->GetAvailableGameLanguages()) : ""; }

String SteamApi::GetCurrentBetaName()
{
    if (!GetIsReady()) return "";
    char buf[512];
    return SteamApps()->GetCurrentBetaName(buf, 512) ? String(buf) : "";
}

String SteamApi::GetCurrentGameLanguage() { return GetIsReady() ? String(SteamApps()->GetCurrentGameLanguage()) : ""; }
int SteamApi::GetDLCCount() { return GetIsReady() ? SteamApps()->GetDLCCount() : 0; }
TypedArray<DlcData> SteamApi::GetDLC() { return singleton ? singleton->DlcCollection : TypedArray<DlcData>(); }

int SteamApi::GetEarliestPurchaseUnixTime(int appId)
{
    return GetIsReady() ? SteamApps()->GetEarliestPurchaseUnixTime(static_cast<AppId_t>(appId)) : 0;
}

String SteamApi::GetLaunchCommandLine()
{
    if (!GetIsReady()) return "";
    char buf[256]{};
    SteamApps()->GetLaunchCommandLine(buf, 256);
    return String(buf);
}

String SteamApi::GetLaunchQueryParam(const String &key)
{
    return GetIsReady() ? String(SteamApps()->GetLaunchQueryParam(key.utf8().get_data())) : "";
}

bool SteamApi::IsAppInstalled(int appId) { return GetIsReady() && SteamApps()->BIsAppInstalled(static_cast<AppId_t>(appId)); }
bool SteamApi::IsCybercafe() { return GetIsReady() && SteamApps()->BIsCybercafe(); }
bool SteamApi::IsLowViolence() { return GetIsReady() && SteamApps()->BIsLowViolence(); }
bool SteamApi::IsSubscribed() { return GetIsReady() && SteamApps()->BIsSubscribed(); }
bool SteamApi::IsSubscribedApp(int appId) { return GetIsReady() && SteamApps()->BIsSubscribedApp(static_cast<AppId_t>(appId)); }
bool SteamApi::IsSubscribedFromFamilySharing() { return GetIsReady() && SteamApps()->BIsSubscribedFromFamilySharing(); }
bool SteamApi::IsSubscribedFromFreeWeekend() { return GetIsReady() && SteamApps()->BIsSubscribedFromFreeWeekend(); }

Ref<SteamTimedTrial> SteamApi::IsTimedTrial()
{
    if (!GetIsReady()) return Ref<SteamTimedTrial>(memnew(SteamTimedTrial()));
    uint32 secondsAllowed = 0, secondsPlayed = 0;
    SteamApps()->BIsTimedTrial(&secondsAllowed, &secondsPlayed);
    return SteamTimedTrial::GetSteamTimedTrial(secondsAllowed, secondsPlayed);
}

bool SteamApi::IsVACBanned() { return GetIsReady() && SteamApps()->BIsVACBanned(); }
bool SteamApi::MarkContentCorrupt(bool missingFilesOnly) { return GetIsReady() && SteamApps()->MarkContentCorrupt(missingFilesOnly); }

// --- Overlay ---
void SteamApi::ActivateGameOverlay(const String &type)
{
    if (GetIsReady()) SteamFriends()->ActivateGameOverlay(type.utf8().get_data());
}
void SteamApi::ActivateGameOverlayInviteDialogConnectString(const String &s)
{
    if (GetIsReady()) SteamFriends()->ActivateGameOverlayInviteDialogConnectString(s.utf8().get_data());
}
void SteamApi::ActivateGameOverlayToStore(int app_id)
{
    if (GetIsReady()) SteamFriends()->ActivateGameOverlayToStore(static_cast<AppId_t>(app_id), k_EOverlayToStoreFlag_None);
}
void SteamApi::ActivateGameOverlayToWebPage(const String &url, bool modal)
{
    if (GetIsReady()) SteamFriends()->ActivateGameOverlayToWebPage(url.utf8().get_data(), modal ? k_EActivateGameOverlayToWebPageMode_Modal : k_EActivateGameOverlayToWebPageMode_Default);
}

// --- Leaderboards ---
void SteamApi::FindLeaderboard(const String &leaderboard)
{
    if (!GetIsReady()) return;
    LeaderboardFindRequest request;
    request.leaderboard = leaderboard;
    request.createIfMissing = false;
    singleton->LeaderboardFindRequests.push(request);
}

void SteamApi::FindOrCreateLeaderboard(const String &leaderboard, bool lowestScoreIsTopRank, SteamLeaderboardDisplay::Type displayType)
{
    if (!GetIsReady()) return;
    LeaderboardFindRequest request;
    request.leaderboard = leaderboard;
    request.createIfMissing = true;
    request.lowestScoreIsTopRank = lowestScoreIsTopRank;
    request.displayType = displayType;
    singleton->LeaderboardFindRequests.push(request);
}

uint64_t SteamApi::GetLeaderboardNativeId(const String &leaderboard)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return 0;
    return singleton->LeaderboardMap.get(leaderboard);
}

SteamLeaderboardDisplay::Type SteamApi::GetLeaderboardDisplayType(const String &leaderboard)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return SteamLeaderboardDisplay::Type::Numeric;
    return static_cast<SteamLeaderboardDisplay::Type>(SteamUserStats()->GetLeaderboardDisplayType(singleton->LeaderboardMap.get(leaderboard)));
}

int SteamApi::GetLeaderboardEntryCount(const String &leaderboard)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return 0;
    return SteamUserStats()->GetLeaderboardEntryCount(singleton->LeaderboardMap.get(leaderboard));
}

String SteamApi::GetLeaderboardName(const String &leaderboard)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return "";
    return String(SteamUserStats()->GetLeaderboardName(singleton->LeaderboardMap.get(leaderboard)));
}

bool SteamApi::IsLeaderboardTopRankLowestScore(const String &leaderboard)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return false;
    return SteamUserStats()->GetLeaderboardSortMethod(singleton->LeaderboardMap.get(leaderboard)) == k_ELeaderboardSortMethodAscending;
}

void SteamApi::DownloadLeaderboardGlobalEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    LeaderboardDownloadRequest req;
    req.leaderboard = singleton->LeaderboardMap.get(leaderboard);
    req.dataRequest = k_ELeaderboardDataRequestGlobal;
    req.start = start; req.end = end; req.detailCount = detailCount;
    req.onComplete = callback;
    singleton->LeaderboardDownloadRequests.push(req);
}

void SteamApi::DownloadLeaderboardAroundUserEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    LeaderboardDownloadRequest req;
    req.leaderboard = singleton->LeaderboardMap.get(leaderboard);
    req.dataRequest = k_ELeaderboardDataRequestGlobalAroundUser;
    req.start = start; req.end = end; req.detailCount = detailCount;
    req.onComplete = callback;
    singleton->LeaderboardDownloadRequests.push(req);
}

void SteamApi::DownloadLeaderboardFriendsEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    LeaderboardDownloadRequest req;
    req.leaderboard = singleton->LeaderboardMap.get(leaderboard);
    req.dataRequest = k_ELeaderboardDataRequestFriends;
    req.start = start; req.end = end; req.detailCount = detailCount;
    req.onComplete = callback;
    singleton->LeaderboardDownloadRequests.push(req);
}

void SteamApi::DownloadLeaderboardEntriesForUsers(TypedArray<UserData> users, int detailCount, const String &leaderboard, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    LeaderboardDownloadRequest req;
    req.leaderboard = singleton->LeaderboardMap.get(leaderboard);
    req.dataRequest = k_ELeaderboardDataRequestUsers;
    req.detailCount = detailCount;
    for (int i = 0; i < users.size(); i++)
        req.userReferences.push_back(users[i]);
    req.onComplete = callback;
    singleton->LeaderboardDownloadRequests.push(req);
}

void SteamApi::DownloadLeaderboardEntriesForUserIds(PackedInt64Array users, int detailCount, const String &leaderboard, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    LeaderboardDownloadRequest req;
    req.leaderboard = singleton->LeaderboardMap.get(leaderboard);
    req.dataRequest = k_ELeaderboardDataRequestUsers;
    req.detailCount = detailCount;
    req.userIds = users;
    req.onComplete = callback;
    singleton->LeaderboardDownloadRequests.push(req);
}

void SteamApi::UploadLeaderboardScore(const String &leaderboard, int score, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    singleton->m_LeaderboardScoreUploaded_callback = callback;
    const SteamLeaderboard_t handle = singleton->LeaderboardMap.get(leaderboard);
    const auto apiCall = SteamUserStats()->UploadLeaderboardScore(handle, k_ELeaderboardUploadScoreMethodKeepBest, score, nullptr, 0);
    singleton->m_LeaderboardScoreUploaded_t.Set(apiCall, singleton, &SteamApi::OnLeaderboardScoreUploaded);
}

void SteamApi::UploadLeaderboardScoreWithDetails(const String &leaderboard, int score, const PackedInt32Array &details, const Callable &callback)
{
    if (!GetIsReady() || !singleton->LeaderboardMap.has(leaderboard)) return;
    singleton->m_LeaderboardScoreUploaded_callback = callback;
    const SteamLeaderboard_t handle = singleton->LeaderboardMap.get(leaderboard);
    std::vector<int32> detailVec;
    for (int i = 0; i < details.size(); i++) detailVec.push_back(details[i]);
    const auto apiCall = SteamUserStats()->UploadLeaderboardScore(handle, k_ELeaderboardUploadScoreMethodKeepBest, score, detailVec.data(), static_cast<int>(detailVec.size()));
    singleton->m_LeaderboardScoreUploaded_t.Set(apiCall, singleton, &SteamApi::OnLeaderboardScoreUploaded);
}

void SteamApi::AttachLeaderboardFile(const String &file, const String &leaderboard, const Callable &callback)
{
    // Steam has no "attach a raw file to a leaderboard entry" call — it's a two-step
    // process: share the Cloud file to get a UGC handle (RemoteStorage::FileShare),
    // then attach that handle to the leaderboard (UserStats::AttachLeaderboardUGC).
    if (!GetIsReady() || !callback.is_valid())
        return;

    if (!singleton->LeaderboardMap.has(leaderboard))
    {
        callback.call(false);
        return;
    }

    singleton->m_RemoteStorageFileShareResult_leaderboard = leaderboard;
    singleton->m_RemoteStorageFileShareResult_callback = callback;
    const auto handle = SteamRemoteStorage()->FileShare(file.utf8().get_data());
    singleton->m_RemoteStorageFileShareResult_t.Set(handle, singleton, &SteamApi::OnRemoteStorageFileShareResult);
}

// --- Stats & Achievements ---
void SteamApi::GetNumberOfCurrentPlayers(Callable callback)
{
    if (!GetIsReady()) return;
    singleton->m_NumberOfCurrentPlayers_callback = callback;
    const auto handle = SteamUserStats()->GetNumberOfCurrentPlayers();
    singleton->m_NumberOfCurrentPlayers_t.Set(handle, singleton, &SteamApi::OnNumberOfCurrentPlayers);
}

void SteamApi::RequestUserRefStats(const Ref<UserData> &user)
{
    if (GetIsReady()) SteamUserStats()->RequestUserStats(user->ToCSteamID());
}

void SteamApi::RequestUserIdStats(uint64_t steamId)
{
    if (GetIsReady()) SteamUserStats()->RequestUserStats(CSteamID(static_cast<uint64>(steamId)));
}

bool SteamApi::ResetAllStats(bool achievementsToo) { return GetIsReady() && SteamUserStats()->ResetAllStats(achievementsToo); }
bool SteamApi::StoreStats() { return GetIsReady() && SteamUserStats()->StoreStats(); }

bool SteamApi::SetAchievement(const String &achievement) { return GetIsReady() && SteamUserStats()->SetAchievement(achievement.utf8().get_data()); }

bool SteamApi::IsAchievementUnlocked(const String &achievement)
{
    if (!GetIsReady()) return false;
    bool achieved = false;
    SteamUserStats()->GetAchievement(achievement.utf8().get_data(), &achieved);
    return achieved;
}

bool SteamApi::IsUserAchievementUnlocked(const Ref<UserData> &user, const String &achievement)
{
    if (!GetIsReady()) return false;
    bool achieved = false;
    SteamUserStats()->GetUserAchievement(user->ToCSteamID(), achievement.utf8().get_data(), &achieved);
    return achieved;
}

int SteamApi::GetAchievementUnlockTime(const String &achievement)
{
    if (!GetIsReady()) return 0;
    bool achieved = false;
    uint32 time = 0;
    SteamUserStats()->GetAchievementAndUnlockTime(achievement.utf8().get_data(), &achieved, &time);
    return achieved ? static_cast<int>(time) : 0;
}

int SteamApi::GetUserAchievementUnlockTime(const Ref<UserData> &user, const String &achievement)
{
    if (!GetIsReady()) return 0;
    bool achieved = false;
    uint32 time = 0;
    SteamUserStats()->GetUserAchievementAndUnlockTime(user->ToCSteamID(), achievement.utf8().get_data(), &achieved, &time);
    return achieved ? static_cast<int>(time) : 0;
}

float SteamApi::GetAchievementAchievedPercent(const String &achievement)
{
    if (!GetIsReady()) return 0.f;
    float percent = 0.f;
    SteamUserStats()->GetAchievementAchievedPercent(achievement.utf8().get_data(), &percent);
    return percent;
}

String SteamApi::GetAchievementName(const String &achievement)
{
    return GetIsReady() ? String(SteamUserStats()->GetAchievementDisplayAttribute(achievement.utf8().get_data(), "name")) : "";
}

String SteamApi::GetAchievementDescription(const String &achievement)
{
    return GetIsReady() ? String(SteamUserStats()->GetAchievementDisplayAttribute(achievement.utf8().get_data(), "desc")) : "";
}

bool SteamApi::GetAchievementIsHidden(const String &achievement)
{
    return GetIsReady() && String(SteamUserStats()->GetAchievementDisplayAttribute(achievement.utf8().get_data(), "hidden")) == "1";
}

void SteamApi::GetAchievementIcon(const String &achievement, const Callable &callback)
{
    if (!GetIsReady()) return;
    const int icon = SteamUserStats()->GetAchievementIcon(achievement.utf8().get_data());
    if (icon > 0)
        LoadAvatar(CSteamID());
}

PackedStringArray SteamApi::GetAchievements()
{
    PackedStringArray result;
    if (!GetIsReady()) return result;
    const uint32 count = SteamUserStats()->GetNumAchievements();
    for (uint32 i = 0; i < count; i++)
        result.append(String(SteamUserStats()->GetAchievementName(i)));
    return result;
}

TypedArray<Dictionary> SteamApi::GetMostAchievedInfo()
{
    TypedArray<Dictionary> result;
    if (!GetIsReady()) return result;
    uint32 iterator = 0;
    char name[256];
    float percent = 0.f;
    bool achieved = false;
    while (SteamUserStats()->GetMostAchievedAchievementInfo(name, sizeof(name), &percent, &achieved) != -1)
    {
        Dictionary d;
        d["name"] = String(name);
        d["percent"] = percent;
        d["achieved"] = achieved;
        result.append(d);
        iterator++;
        if (iterator > 100) break;
    }
    return result;
}

bool SteamApi::IndicateAchievementProgress(const String &achievement, int current, int max)
{
    return GetIsReady() && SteamUserStats()->IndicateAchievementProgress(achievement.utf8().get_data(), current, max);
}

bool SteamApi::ClearAchievement(const String &achievement)
{
    return GetIsReady() && SteamUserStats()->ClearAchievement(achievement.utf8().get_data());
}

bool SteamApi::SetStatFloat(const String &stat, float value) { return GetIsReady() && SteamUserStats()->SetStat(stat.utf8().get_data(), value); }
bool SteamApi::SetStatInt(const String &stat, int value) { return GetIsReady() && SteamUserStats()->SetStat(stat.utf8().get_data(), static_cast<int32>(value)); }
bool SteamApi::UpdateAvgRateStat(const String &stat, float thisSession, double sessionLength) { return GetIsReady() && SteamUserStats()->UpdateAvgRateStat(stat.utf8().get_data(), thisSession, sessionLength); }

void SteamApi::RequestGlobalAchievementPercentages(const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_GlobalAchievementPercentagesReady_callback = callback;
    const auto handle = SteamUserStats()->RequestGlobalAchievementPercentages();
    singleton->m_GlobalAchievementPercentagesReady_t.Set(handle, singleton, &SteamApi::OnGlobalAchievementPercentagesReady);
}

void SteamApi::RequestGlobalStats(int historyDays, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_GlobalStatsReceived_callback = callback;
    const auto handle = SteamUserStats()->RequestGlobalStats(historyDays);
    singleton->m_GlobalStatsReceived_t.Set(handle, singleton, &SteamApi::OnGlobalStatsReceived);
}

uint64_t SteamApi::GetGlobalStatInt(const String &stat)
{
    if (!GetIsReady()) return 0;
    int64 val = 0;
    SteamUserStats()->GetGlobalStat(stat.utf8().get_data(), &val);
    return static_cast<uint64_t>(val);
}

double SteamApi::GetGlobalStatFloat(const String &stat)
{
    if (!GetIsReady()) return 0.0;
    double val = 0.0;
    SteamUserStats()->GetGlobalStat(stat.utf8().get_data(), &val);
    return val;
}

float SteamApi::GetStatFloat(const String &stat)
{
    if (!GetIsReady()) return 0.f;
    float val = 0.f;
    SteamUserStats()->GetStat(stat.utf8().get_data(), &val);
    return val;
}

int SteamApi::GetStatInt(const String &stat)
{
    if (!GetIsReady()) return 0;
    int32 val = 0;
    SteamUserStats()->GetStat(stat.utf8().get_data(), &val);
    return val;
}

float SteamApi::GetUserStatFloat(const Ref<UserData> &user, const String &stat)
{
    if (!GetIsReady()) return 0.f;
    float val = 0.f;
    SteamUserStats()->GetUserStat(user->ToCSteamID(), stat.utf8().get_data(), &val);
    return val;
}

int SteamApi::GetUserStatInt(const Ref<UserData> &user, const String &stat)
{
    if (!GetIsReady()) return 0;
    int32 val = 0;
    SteamUserStats()->GetUserStat(user->ToCSteamID(), stat.utf8().get_data(), &val);
    return val;
}

PackedInt64Array SteamApi::GetGlobalStatIntHistory(const String &stat, int historyDays)
{
    PackedInt64Array result;
    if (!GetIsReady()) return result;
    std::vector<int64> history(historyDays);
    int count = SteamUserStats()->GetGlobalStatHistory(stat.utf8().get_data(), history.data(), historyDays * sizeof(int64));
    for (int i = 0; i < count; i++) result.append(history[i]);
    return result;
}

PackedFloat64Array SteamApi::GetGlobalStatFloatHistory(const String &stat, int historyDays)
{
    PackedFloat64Array result;
    if (!GetIsReady()) return result;
    std::vector<double> history(historyDays);
    int count = SteamUserStats()->GetGlobalStatHistory(stat.utf8().get_data(), history.data(), historyDays * sizeof(double));
    for (int i = 0; i < count; i++) result.append(history[i]);
    return result;
}

// --- Friends / User ---
void SteamApi::GetFriendAvatar(const Ref<UserData> &user, const Callable &callback)
{
    if (!GetIsReady()) return;
    const CSteamID id = user->ToCSteamID();
    const uint64 rawId = id.ConvertToUint64();

    if (singleton->AvatarCache.count(rawId))
    {
        callback.call(singleton->AvatarCache.at(rawId));
        return;
    }
    singleton->AvatarRequests[rawId] = callback;
    SteamFriends()->GetLargeFriendAvatar(id);
}

void SteamApi::LoadAvatar(CSteamID steam_id)
{
    const int handle = SteamFriends()->GetLargeFriendAvatar(steam_id);
    if (handle <= 0) return;

    uint32 width = 0, height = 0;
    if (!SteamUtils()->GetImageSize(handle, &width, &height)) return;

    std::vector<uint8_t> pixels(width * height * 4);
    if (!SteamUtils()->GetImageRGBA(handle, pixels.data(), pixels.size())) return;

    PackedByteArray pba;
    pba.resize(static_cast<int>(pixels.size()));
    memcpy(pba.ptrw(), pixels.data(), pixels.size());
    Ref<Image> img = Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, pba);
    Ref<ImageTexture> tex = ImageTexture::create_from_image(img);

    const uint64 rawId = steam_id.ConvertToUint64();
    singleton->AvatarCache[rawId] = tex;

    if (singleton->AvatarRequests.count(rawId))
    {
        singleton->AvatarRequests.at(rawId).call(tex);
        singleton->AvatarRequests.erase(rawId);
    }
}

// --- Matchmaking ---
void SteamApi::LeaveLobby(const Ref<LobbyData> &lobby)
{
    if (!GetIsReady()) return;
    CSteamID id = lobby->ToCSteamID();
    SteamMatchmaking()->LeaveLobby(id);
    int64 rawId = static_cast<int64>(id.ConvertToUint64());
    int idx = singleton->MemberOfLobbies.find(rawId);
    if (idx >= 0) singleton->MemberOfLobbies.remove_at(idx);
}

void SteamApi::CreateLobby(LobbyType::Type type, LobbyUseHint::Hint hint, int maxMembers, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_LobbyCreated_callback = callback;
    singleton->m_LobbyCreatedHint = hint;
    const auto handle = SteamMatchmaking()->CreateLobby(static_cast<ELobbyType>(type), maxMembers);
    singleton->m_LobbyCreate_t.Set(handle, singleton, &SteamApi::OnLobbyCreated);
}

void SteamApi::JoinLobby(const Ref<LobbyData> &lobby, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_LobbyEnter_callback = callback;
    const auto handle = SteamMatchmaking()->JoinLobby(lobby->ToCSteamID());
    singleton->m_LobbyEnter_t.Set(handle, singleton, &SteamApi::OnLobbyEnter);
}

void SteamApi::JoinLobbyById(int64_t lobby_id, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_LobbyEnter_callback = callback;
    const auto handle = SteamMatchmaking()->JoinLobby(CSteamID(static_cast<uint64>(lobby_id)));
    singleton->m_LobbyEnter_t.Set(handle, singleton, &SteamApi::OnLobbyEnter);
}

void SteamApi::JoinLobbyByHex(const String &hexId, const Callable &callback)
{
    uint32_t accountId = static_cast<uint32_t>(hexId.hex_to_int());
    CSteamID steamId(accountId, 393216, k_EUniversePublic, k_EAccountTypeChat);
    JoinLobbyById(static_cast<int64_t>(steamId.ConvertToUint64()), callback);
}

void SteamApi::LobbyMatchList(const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_LobbyMatchList_callback = callback;
    const auto handle = SteamMatchmaking()->RequestLobbyList();
    singleton->m_LobbyMatchList_t.Set(handle, singleton, &SteamApi::OnLobbyMatchList);
}

// --- Matchmaking: lobby metadata/property plumbing ---
// Raw SDK operations only — ported from the Toolkit for Steamworks scaffold, which
// originally duplicated this natively; it now calls these instead of reimplementing them.
namespace
{
    static constexpr const char *kLobbyDataName = "name";
    static constexpr const char *kLobbyDataMode = "z_heathenMode";
    static constexpr const char *kLobbyDataType = "z_heathenType";
} // namespace

void SteamApi::InviteUserToLobby(const Ref<LobbyData> &lobby, const Ref<UserData> &user)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->InviteUserToLobby(lobby->ToCSteamID(), user->ToCSteamID());
}

void SteamApi::SetLobbyData(const Ref<LobbyData> &lobby, const String &key, const String &value)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyData(lobby->ToCSteamID(), key.utf8().get_data(), value.utf8().get_data());
}

String SteamApi::GetLobbyData(const Ref<LobbyData> &lobby, const String &key)
{
    if (SteamMatchmaking())
        return String(SteamMatchmaking()->GetLobbyData(lobby->ToCSteamID(), key.utf8().get_data()));
    return "";
}

void SteamApi::SetLobbyMemberData(const Ref<LobbyData> &lobby, const String &key, const String &value)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyMemberData(lobby->ToCSteamID(), key.utf8().get_data(), value.utf8().get_data());
}

String SteamApi::GetLobbyMemberData(const Ref<LobbyData> &lobby, const Ref<UserData> &user, const String &key)
{
    if (SteamMatchmaking())
        return String(SteamMatchmaking()->GetLobbyMemberData(lobby->ToCSteamID(), user->ToCSteamID(), key.utf8().get_data()));
    return "";
}

String SteamApi::GetLobbyName(const Ref<LobbyData> &lobby)
{
    if (SteamMatchmaking())
        return String(SteamMatchmaking()->GetLobbyData(lobby->ToCSteamID(), kLobbyDataName));
    return "";
}

void SteamApi::SetLobbyName(const Ref<LobbyData> &lobby, const String &name)
{
    SetLobbyData(lobby, kLobbyDataName, name);
}

int SteamApi::GetLobbyMemberCount(const Ref<LobbyData> &lobby)
{
    if (SteamMatchmaking())
        return SteamMatchmaking()->GetNumLobbyMembers(lobby->ToCSteamID());
    return 0;
}

int SteamApi::GetLobbyMaxMembers(const Ref<LobbyData> &lobby)
{
    if (SteamMatchmaking())
        return SteamMatchmaking()->GetLobbyMemberLimit(lobby->ToCSteamID());
    return 0;
}

void SteamApi::SetLobbyMaxMembers(const Ref<LobbyData> &lobby, int max_members)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyMemberLimit(lobby->ToCSteamID(), max_members);
}

LobbyUseHint::Hint SteamApi::GetLobbyUseHint(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking())
        return LobbyUseHint::Hint::General;
    const char *value = SteamMatchmaking()->GetLobbyData(lobby->ToCSteamID(), kLobbyDataMode);
    if (value == nullptr || strcmp(value, "") == 0 || strcmp(value, "General") == 0) return LobbyUseHint::Hint::General;
    if (strcmp(value, "Session") == 0) return LobbyUseHint::Hint::Session;
    if (strcmp(value, "Party") == 0) return LobbyUseHint::Hint::Party;
    return LobbyUseHint::Hint::General;
}

void SteamApi::SetLobbyUseHint(const Ref<LobbyData> &lobby, LobbyUseHint::Hint hint)
{
    if (hint == LobbyUseHint::Hint::Session) SetLobbyData(lobby, kLobbyDataMode, "Session");
    else if (hint == LobbyUseHint::Hint::Party) SetLobbyData(lobby, kLobbyDataMode, "Party");
    else SetLobbyData(lobby, kLobbyDataMode, "General");
}

LobbyType::Type SteamApi::GetLobbyType(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking())
        return LobbyType::Type::Public;
    const char *value = SteamMatchmaking()->GetLobbyData(lobby->ToCSteamID(), kLobbyDataType);
    if (value == nullptr || strcmp(value, "") == 0) return LobbyType::Type::Public;
    if (strcmp(value, "Private") == 0) return LobbyType::Type::Private;
    if (strcmp(value, "FriendOnly") == 0) return LobbyType::Type::FriendsOnly;
    if (strcmp(value, "Public") == 0) return LobbyType::Type::Public;
    if (strcmp(value, "Invisible") == 0) return LobbyType::Type::Invisible;
    return LobbyType::Type::Public;
}

void SteamApi::SetLobbyType(const Ref<LobbyData> &lobby, LobbyType::Type type)
{
    if (!SteamMatchmaking()) return;
    SteamMatchmaking()->SetLobbyType(lobby->ToCSteamID(), static_cast<ELobbyType>(type));
    if (type == LobbyType::Type::Private) SetLobbyData(lobby, kLobbyDataType, "Private");
    else if (type == LobbyType::Type::FriendsOnly) SetLobbyData(lobby, kLobbyDataType, "FriendOnly");
    else if (type == LobbyType::Type::Public) SetLobbyData(lobby, kLobbyDataType, "Public");
    else if (type == LobbyType::Type::Invisible) SetLobbyData(lobby, kLobbyDataType, "Invisible");
}

bool SteamApi::IsLobbyFull(const Ref<LobbyData> &lobby)
{
    return GetLobbyMemberCount(lobby) >= GetLobbyMaxMembers(lobby);
}

bool SteamApi::IsLobbyOwner(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking() || !SteamUser()) return false;
    return SteamMatchmaking()->GetLobbyOwner(lobby->ToCSteamID()) == SteamUser()->GetSteamID();
}

void SteamApi::SetLobbyOwner(const Ref<LobbyData> &lobby, const Ref<UserData> &user)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyOwner(lobby->ToCSteamID(), user->ToCSteamID());
}

bool SteamApi::IsLobbyMember(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking() || !SteamUser()) return false;
    const CSteamID lobbyId = lobby->ToCSteamID();
    const int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
    const CSteamID localUser = SteamUser()->GetSteamID();
    for (int i = 0; i < count; i++)
        if (SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i) == localUser)
            return true;
    return false;
}

void SteamApi::SetLobbyJoinable(const Ref<LobbyData> &lobby, bool joinable)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyJoinable(lobby->ToCSteamID(), joinable);
}

void SteamApi::SetLobbyListenServer(const Ref<LobbyData> &lobby)
{
    if (SteamMatchmaking())
        SteamMatchmaking()->SetLobbyGameServer(lobby->ToCSteamID(), 0, 0, SteamMatchmaking()->GetLobbyOwner(lobby->ToCSteamID()));
}

void SteamApi::SetLobbyDedicatedServer(const Ref<LobbyData> &lobby, uint64_t serverId, const String &ip, uint16_t port)
{
    if (!SteamMatchmaking()) return;
    const uint32 unIP = IpStringToUint(ip);
    SteamMatchmaking()->SetLobbyGameServer(lobby->ToCSteamID(), unIP, port, CSteamID(static_cast<uint64>(serverId)));
}

bool SteamApi::LobbyHasGameServer(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking()) return false;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    return SteamMatchmaking()->GetLobbyGameServer(lobby->ToCSteamID(), &unIP, &usPort, &steamServerID);
}

uint64_t SteamApi::GetLobbyServerId(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking()) return 0;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(lobby->ToCSteamID(), &unIP, &usPort, &steamServerID);
    return steamServerID.ConvertToUint64();
}

String SteamApi::GetLobbyServerIp(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking()) return "";
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(lobby->ToCSteamID(), &unIP, &usPort, &steamServerID);
    return IpUintToString(unIP);
}

uint16_t SteamApi::GetLobbyServerPort(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking()) return 0;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(lobby->ToCSteamID(), &unIP, &usPort, &steamServerID);
    return usPort;
}

TypedArray<UserData> SteamApi::GetLobbyMemberList(const Ref<LobbyData> &lobby)
{
    TypedArray<UserData> members;
    if (!SteamMatchmaking()) return members;
    const CSteamID lobbyId = lobby->ToCSteamID();
    const int count = SteamMatchmaking()->GetNumLobbyMembers(lobbyId);
    for (int i = 0; i < count; i++)
        members.append(memnew(UserData(SteamMatchmaking()->GetLobbyMemberByIndex(lobbyId, i))));
    return members;
}

Ref<UserData> SteamApi::GetLobbyOwner(const Ref<LobbyData> &lobby)
{
    if (!SteamMatchmaking()) return {};
    return Ref<UserData>(memnew(UserData(SteamMatchmaking()->GetLobbyOwner(lobby->ToCSteamID()))));
}

bool SteamApi::SendLobbyChatMessage(const Ref<LobbyData> &lobby, const String &message)
{
    if (!SteamMatchmaking()) return false;
    const std::string utf8 = message.utf8().get_data();
    return SteamMatchmaking()->SendLobbyChatMsg(lobby->ToCSteamID(), utf8.c_str(), static_cast<int>(utf8.size()));
}

void SteamApi::ActivateLobbyInviteDialog(const Ref<LobbyData> &lobby)
{
    if (SteamFriends())
        SteamFriends()->ActivateGameOverlayInviteDialog(lobby->ToCSteamID());
}

void SteamApi::ActivateLobbyRemotePlayTogetherInviteDialog(const Ref<LobbyData> &lobby)
{
    if (SteamFriends())
        SteamFriends()->ActivateGameOverlayRemotePlayTogetherInviteDialog(lobby->ToCSteamID());
}

// --- Utilities ---
uint32_t SteamApi::IpStringToUint(const String &ip)
{
    const auto parts = ip.split(".");
    if (parts.size() != 4) return 0;
    uint32_t result = 0;
    for (int i = 0; i < 4; i++)
        result = (result << 8) | static_cast<uint32_t>(parts[i].to_int());
    return result;
}

String SteamApi::IpUintToString(uint32_t ip)
{
    return String::num_int64((ip >> 24) & 0xFF) + "." +
           String::num_int64((ip >> 16) & 0xFF) + "." +
           String::num_int64((ip >> 8) & 0xFF) + "." +
           String::num_int64(ip & 0xFF);
}

// --- Remote Storage ---
bool SteamApi::FileWrite(const String &fileName, const PackedByteArray &data)
{
    if (!GetIsReady()) return false;
    return SteamRemoteStorage()->FileWrite(fileName.utf8().get_data(), data.ptr(), data.size());
}

PackedByteArray SteamApi::FileRead(const String &fileName)
{
    if (!GetIsReady()) return {};
    const int size = SteamRemoteStorage()->GetFileSize(fileName.utf8().get_data());
    if (size <= 0) return {};
    PackedByteArray data;
    data.resize(size);
    SteamRemoteStorage()->FileRead(fileName.utf8().get_data(), data.ptrw(), size);
    return data;
}

bool SteamApi::FileDelete(const String &fileName)
{
    return GetIsReady() && SteamRemoteStorage()->FileDelete(fileName.utf8().get_data());
}

bool SteamApi::FileExists(const String &fileName)
{
    return GetIsReady() && SteamRemoteStorage()->FileExists(fileName.utf8().get_data());
}

int SteamApi::GetFileSize(const String &fileName)
{
    return GetIsReady() ? SteamRemoteStorage()->GetFileSize(fileName.utf8().get_data()) : 0;
}

int SteamApi::GetFileCount()
{
    return GetIsReady() ? SteamRemoteStorage()->GetFileCount() : 0;
}

Array SteamApi::GetFileNameAndSize(int index)
{
    Array result;
    if (!GetIsReady()) return result;
    int32 fileSize = 0;
    const char *fileName = SteamRemoteStorage()->GetFileNameAndSize(index, &fileSize);
    result.append(String(fileName));
    result.append(static_cast<int>(fileSize));
    return result;
}

void SteamApi::GetQuota(const Callable &callback)
{
    if (!GetIsReady()) return;
    uint64 total = 0, available = 0;
    SteamRemoteStorage()->GetQuota(&total, &available);
    callback.call(static_cast<int64_t>(total), static_cast<int64_t>(available));
}

bool SteamApi::IsCloudEnabledForApp()
{
    return GetIsReady() && SteamRemoteStorage()->IsCloudEnabledForApp();
}

bool SteamApi::IsCloudEnabledForAccount()
{
    return GetIsReady() && SteamRemoteStorage()->IsCloudEnabledForAccount();
}

// --- Remote Play ---
int SteamApi::GetRemotePlaySessionCount()
{
    return GetIsReady() ? static_cast<int>(SteamRemotePlay()->GetSessionCount()) : 0;
}

uint32_t SteamApi::GetRemotePlaySessionID(int index)
{
    return GetIsReady() ? SteamRemotePlay()->GetSessionID(index) : 0;
}

Ref<UserData> SteamApi::GetRemotePlaySessionSteamID(uint64_t sessionId)
{
    if (!GetIsReady()) return {};
    return Ref<UserData>(memnew(UserData(SteamRemotePlay()->GetSessionSteamID(static_cast<RemotePlaySessionID_t>(sessionId)))));
}

String SteamApi::GetRemotePlaySessionClientName(uint64_t sessionId)
{
    if (!GetIsReady()) return "";
    const char *name = SteamRemotePlay()->GetSessionClientName(static_cast<RemotePlaySessionID_t>(sessionId));
    return name ? String(name) : "";
}

void SteamApi::SendRemotePlayTogetherInvite(const Ref<UserData> &user, const Callable &callback)
{
    if (!GetIsReady()) { callback.call(false); return; }
    const bool success = SteamRemotePlay()->BSendRemotePlayTogetherInvite(user->ToCSteamID());
    callback.call(success);
}

// --- Screenshots ---
void SteamApi::TriggerScreenshot()
{
    if (GetIsReady()) SteamScreenshots()->TriggerScreenshot();
}

void SteamApi::HookScreenshots(bool hook)
{
    if (GetIsReady()) SteamScreenshots()->HookScreenshots(hook);
}

uint32_t SteamApi::AddScreenshotToLibrary(const String &fileName, const String &thumbnailFileName, int width, int height)
{
    if (!GetIsReady()) return 0;
    return SteamScreenshots()->AddScreenshotToLibrary(
        fileName.utf8().get_data(), thumbnailFileName.utf8().get_data(), width, height);
}

void SteamApi::SetScreenshotLocation(uint32_t screenshotHandle, const String &location)
{
    if (GetIsReady())
        SteamScreenshots()->SetLocation(static_cast<ScreenshotHandle>(screenshotHandle), location.utf8().get_data());
}

void SteamApi::TagScreenshotUser(uint32_t screenshotHandle, const Ref<UserData> &user)
{
    if (GetIsReady())
        SteamScreenshots()->TagUser(static_cast<ScreenshotHandle>(screenshotHandle), user->ToCSteamID());
}

void SteamApi::TagPublishedFile(int64_t screenshotHandle, int64_t publishedFileId)
{
    if (GetIsReady())
        SteamScreenshots()->TagPublishedFile(
            static_cast<ScreenshotHandle>(screenshotHandle),
            static_cast<PublishedFileId_t>(publishedFileId));
}

// --- Voice ---
void SteamApi::StartVoiceRecording()
{
    if (GetIsReady()) SteamUser()->StartVoiceRecording();
}

void SteamApi::StopVoiceRecording()
{
    if (GetIsReady()) SteamUser()->StopVoiceRecording();
}

int SteamApi::GetAvailableVoice()
{
    if (!GetIsReady()) return 0;
    uint32 compressed = 0;
    SteamUser()->GetAvailableVoice(&compressed);
    return static_cast<int>(compressed);
}

PackedByteArray SteamApi::GetVoice()
{
    if (!GetIsReady()) return {};
    uint32 available = 0;
    SteamUser()->GetAvailableVoice(&available);
    if (available == 0) return {};
    PackedByteArray buf;
    buf.resize(static_cast<int>(available));
    uint32 written = 0;
    const EVoiceResult r = SteamUser()->GetVoice(true, buf.ptrw(), available, &written, false, nullptr, 0, nullptr, 0);
    if (r != k_EVoiceResultOK) return {};
    buf.resize(static_cast<int>(written));
    return buf;
}

PackedByteArray SteamApi::DecompressVoice(const PackedByteArray &compressed, int sampleRate)
{
    if (!GetIsReady()) return {};
    const uint32 outSize = 22050 * 2 * 2;
    PackedByteArray out;
    out.resize(static_cast<int>(outSize));
    uint32 written = 0;
    const EVoiceResult r = SteamUser()->DecompressVoice(
        compressed.ptr(), compressed.size(),
        out.ptrw(), outSize, &written,
        static_cast<uint32>(sampleRate));
    if (r != k_EVoiceResultOK) return {};
    out.resize(static_cast<int>(written));
    return out;
}

int SteamApi::GetVoiceOptimalSampleRate()
{
    return GetIsReady() ? static_cast<int>(SteamUser()->GetVoiceOptimalSampleRate()) : 0;
}

// --- Timeline ---
void SteamApi::SetTimelineGamePhase(int mode)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady()) SteamTimeline()->SetTimelineGameMode(static_cast<ETimelineGameMode>(mode));
#endif
}

void SteamApi::AddTimelineEvent(const String &title, const String &description, const String &icon, int priority, float startOffset, float duration)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady())
        SteamTimeline()->AddRangeTimelineEvent(
            title.utf8().get_data(), description.utf8().get_data(), icon.utf8().get_data(),
            static_cast<uint32>(priority), startOffset, duration, k_ETimelineEventClipPriority_None);
#endif
}

void SteamApi::SetTimelineStateDescription(const String &description, float timeDelta)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady()) SteamTimeline()->SetTimelineTooltip(description.utf8().get_data(), timeDelta);
#endif
}

void SteamApi::ClearTimelineStateDescription(float timeDelta)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady()) SteamTimeline()->ClearTimelineTooltip(timeDelta);
#endif
}

void SteamApi::SetTimelineTooltip(const String &description, float timeDelta)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady()) SteamTimeline()->SetTimelineTooltip(description.utf8().get_data(), timeDelta);
#endif
}

void SteamApi::ClearTimelineTooltip(float timeDelta)
{
#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 58)
    if (GetIsReady()) SteamTimeline()->ClearTimelineTooltip(timeDelta);
#endif
}

// --- Input ---
void SteamApi::InputInit(bool explicitlyCallRunFrame)
{
    if (GetIsReady()) SteamInput()->Init(explicitlyCallRunFrame);
}

void SteamApi::InputShutdown()
{
    if (GetIsReady()) SteamInput()->Shutdown();
}

void SteamApi::InputRunFrame()
{
    if (GetIsReady()) SteamInput()->RunFrame();
}

PackedInt64Array SteamApi::GetConnectedControllers()
{
    PackedInt64Array result;
    if (!GetIsReady()) return result;
    InputHandle_t handles[STEAM_INPUT_MAX_COUNT];
    const int count = SteamInput()->GetConnectedControllers(handles);
    for (int i = 0; i < count; i++)
        result.append(static_cast<int64_t>(handles[i]));
    return result;
}

uint64_t SteamApi::GetActionSetHandle(const String &actionSetName)
{
    return GetIsReady() ? SteamInput()->GetActionSetHandle(actionSetName.utf8().get_data()) : 0;
}

void SteamApi::ActivateActionSet(int64_t inputHandle, int64_t actionSetHandle)
{
    if (GetIsReady())
        SteamInput()->ActivateActionSet(
            static_cast<InputHandle_t>(inputHandle),
            static_cast<InputActionSetHandle_t>(actionSetHandle));
}

uint64_t SteamApi::GetCurrentActionSet(int64_t inputHandle)
{
    return GetIsReady() ? SteamInput()->GetCurrentActionSet(static_cast<InputHandle_t>(inputHandle)) : 0;
}

uint64_t SteamApi::GetDigitalActionHandle(const String &actionName)
{
    return GetIsReady() ? SteamInput()->GetDigitalActionHandle(actionName.utf8().get_data()) : 0;
}

bool SteamApi::GetDigitalActionData(int64_t inputHandle, int64_t actionHandle)
{
    if (!GetIsReady()) return false;
    const InputDigitalActionData_t data = SteamInput()->GetDigitalActionData(
        static_cast<InputHandle_t>(inputHandle),
        static_cast<InputDigitalActionHandle_t>(actionHandle));
    return data.bState;
}

uint64_t SteamApi::GetAnalogActionHandle(const String &actionName)
{
    return GetIsReady() ? SteamInput()->GetAnalogActionHandle(actionName.utf8().get_data()) : 0;
}

Vector2 SteamApi::GetAnalogActionData(int64_t inputHandle, int64_t actionHandle)
{
    if (!GetIsReady()) return Vector2();
    const InputAnalogActionData_t data = SteamInput()->GetAnalogActionData(
        static_cast<InputHandle_t>(inputHandle),
        static_cast<InputAnalogActionHandle_t>(actionHandle));
    return Vector2(data.x, data.y);
}

void SteamApi::TriggerVibration(int64_t inputHandle, int leftSpeed, int rightSpeed)
{
    if (GetIsReady())
        SteamInput()->TriggerVibration(
            static_cast<InputHandle_t>(inputHandle),
            static_cast<unsigned short>(leftSpeed),
            static_cast<unsigned short>(rightSpeed));
}

// --- Parties ---
int SteamApi::GetNumActiveBeacons()
{
    return GetIsReady() ? static_cast<int>(SteamParties()->GetNumActiveBeacons()) : 0;
}

uint64_t SteamApi::GetBeaconByIndex(int index)
{
    return GetIsReady() ? static_cast<uint64_t>(SteamParties()->GetBeaconByIndex(static_cast<uint32>(index))) : 0;
}

void SteamApi::CreateBeacon(int openSlots, const Ref<LobbyData> &lobby, const String &connectString, const String &metadata, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_CreateBeacon_callback = callback;
    SteamPartyBeaconLocation_t loc;
    loc.m_eType = k_ESteamPartyBeaconLocationType_ChatGroup;
    loc.m_ulLocationID = lobby->ToCSteamID().ConvertToUint64();
    const auto handle = SteamParties()->CreateBeacon(
        static_cast<uint32>(openSlots),
        &loc,
        connectString.utf8().get_data(),
        metadata.utf8().get_data());
    singleton->m_CreateBeacon_t.Set(handle, singleton, &SteamApi::OnCreateBeacon);
}

void SteamApi::DestroyBeacon(int64_t beaconHandle)
{
    if (GetIsReady())
        SteamParties()->DestroyBeacon(static_cast<PartyBeaconID_t>(beaconHandle));
}

void SteamApi::ChangeNumOpenSlots(int64_t beaconHandle, int openSlots, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_ChangeNumOpenSlots_callback = callback;
    const auto handle = SteamParties()->ChangeNumOpenSlots(
        static_cast<PartyBeaconID_t>(beaconHandle),
        static_cast<uint32>(openSlots));
    singleton->m_ChangeNumOpenSlots_t.Set(handle, singleton, &SteamApi::OnChangeNumOpenSlots);
}

void SteamApi::OnCreateBeacon(CreateBeaconCallback_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    const int64_t beaconId = (!bIOFailure && pResult->m_eResult == k_EResultOK)
        ? static_cast<int64_t>(pResult->m_ulBeaconID) : 0;
    m_CreateBeacon_callback.call(result, beaconId);
}

void SteamApi::OnChangeNumOpenSlots(ChangeNumOpenSlotsCallback_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    m_ChangeNumOpenSlots_callback.call(result);
}

// --- UGC / Workshop ---
void SteamApi::UgcCreateItem(int appId, int fileType, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_UgcCreateItem_callback = callback;
    const auto handle = SteamUGC()->CreateItem(
        static_cast<AppId_t>(appId),
        static_cast<EWorkshopFileType>(fileType));
    singleton->m_UgcCreateItem_t.Set(handle, singleton, &SteamApi::OnUgcCreateItem);
}

int64_t SteamApi::UgcStartItemUpdate(int appId, int64_t publishedFileId)
{
    if (!GetIsReady()) return 0;
    return static_cast<int64_t>(SteamUGC()->StartItemUpdate(
        static_cast<AppId_t>(appId),
        static_cast<PublishedFileId_t>(publishedFileId)));
}

bool SteamApi::UgcSetItemTitle(int64_t updateHandle, const String &title)
{
    return GetIsReady() && SteamUGC()->SetItemTitle(static_cast<UGCUpdateHandle_t>(updateHandle), title.utf8().get_data());
}

bool SteamApi::UgcSetItemDescription(int64_t updateHandle, const String &description)
{
    return GetIsReady() && SteamUGC()->SetItemDescription(static_cast<UGCUpdateHandle_t>(updateHandle), description.utf8().get_data());
}

bool SteamApi::UgcSetItemContent(int64_t updateHandle, const String &contentFolder)
{
    return GetIsReady() && SteamUGC()->SetItemContent(static_cast<UGCUpdateHandle_t>(updateHandle), contentFolder.utf8().get_data());
}

bool SteamApi::UgcSetItemPreview(int64_t updateHandle, const String &previewFile)
{
    return GetIsReady() && SteamUGC()->SetItemPreview(static_cast<UGCUpdateHandle_t>(updateHandle), previewFile.utf8().get_data());
}

bool SteamApi::UgcSetItemVisibility(int64_t updateHandle, int visibility)
{
    return GetIsReady() && SteamUGC()->SetItemVisibility(
        static_cast<UGCUpdateHandle_t>(updateHandle),
        static_cast<ERemoteStoragePublishedFileVisibility>(visibility));
}

bool SteamApi::UgcAddItemTag(int64_t updateHandle, const String &tag)
{
    if (!GetIsReady()) return false;
    SteamParamStringArray_t tags;
    const std::string s = tag.utf8().get_data();
    const char *ptr = s.c_str();
    tags.m_ppStrings = &ptr;
    tags.m_nNumStrings = 1;
    return SteamUGC()->SetItemTags(static_cast<UGCUpdateHandle_t>(updateHandle), &tags);
}

void SteamApi::UgcSubmitItemUpdate(int64_t updateHandle, const String &changeNotes, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_UgcSubmitItemUpdate_callback = callback;
    const auto handle = SteamUGC()->SubmitItemUpdate(
        static_cast<UGCUpdateHandle_t>(updateHandle),
        changeNotes.utf8().get_data());
    singleton->m_UgcSubmitItemUpdate_t.Set(handle, singleton, &SteamApi::OnUgcSubmitItemUpdate);
}

void SteamApi::UgcSubscribeItem(int64_t publishedFileId, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_UgcSubscribeItem_callback = callback;
    const auto handle = SteamUGC()->SubscribeItem(static_cast<PublishedFileId_t>(publishedFileId));
    singleton->m_UgcSubscribeItem_t.Set(handle, singleton, &SteamApi::OnUgcSubscribeItem);
}

void SteamApi::UgcUnsubscribeItem(int64_t publishedFileId, const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_UgcUnsubscribeItem_callback = callback;
    const auto handle = SteamUGC()->UnsubscribeItem(static_cast<PublishedFileId_t>(publishedFileId));
    singleton->m_UgcUnsubscribeItem_t.Set(handle, singleton, &SteamApi::OnUgcUnsubscribeItem);
}

uint32_t SteamApi::UgcGetNumSubscribedItems()
{
    return GetIsReady() ? SteamUGC()->GetNumSubscribedItems() : 0;
}

PackedInt64Array SteamApi::UgcGetSubscribedItems()
{
    PackedInt64Array result;
    if (!GetIsReady()) return result;
    const uint32 count = SteamUGC()->GetNumSubscribedItems();
    if (count == 0) return result;
    std::vector<PublishedFileId_t> ids(count);
    SteamUGC()->GetSubscribedItems(ids.data(), count);
    for (uint32 i = 0; i < count; i++)
        result.append(static_cast<int64_t>(ids[i]));
    return result;
}

bool SteamApi::UgcDownloadItem(int64_t publishedFileId, bool highPriority)
{
    return GetIsReady() && SteamUGC()->DownloadItem(static_cast<PublishedFileId_t>(publishedFileId), highPriority);
}

Array SteamApi::UgcGetItemInstallInfo(int64_t publishedFileId)
{
    Array result;
    if (!GetIsReady()) return result;
    uint64 sizeOnDisk = 0;
    char path[512];
    uint32 timestamp = 0;
    SteamUGC()->GetItemInstallInfo(static_cast<PublishedFileId_t>(publishedFileId), &sizeOnDisk, path, 512, &timestamp);
    result.append(static_cast<int64_t>(sizeOnDisk));
    result.append(static_cast<int64_t>(timestamp));
    result.append(String(path));
    return result;
}

Array SteamApi::UgcGetItemDownloadInfo(int64_t publishedFileId)
{
    Array result;
    if (!GetIsReady()) return result;
    uint64 downloaded = 0, total = 0;
    SteamUGC()->GetItemDownloadInfo(static_cast<PublishedFileId_t>(publishedFileId), &downloaded, &total);
    result.append(static_cast<int64_t>(downloaded));
    result.append(static_cast<int64_t>(total));
    return result;
}

void SteamApi::OnUgcCreateItem(CreateItemResult_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    const int64_t fileId = (!bIOFailure) ? static_cast<int64_t>(pResult->m_nPublishedFileId) : 0;
    const bool needsLegalAgreement = (!bIOFailure) && pResult->m_bUserNeedsToAcceptWorkshopLegalAgreement;
    m_UgcCreateItem_callback.call(result, fileId, needsLegalAgreement);
}

void SteamApi::OnUgcSubmitItemUpdate(SubmitItemUpdateResult_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    const bool needsLegalAgreement = (!bIOFailure) && pResult->m_bUserNeedsToAcceptWorkshopLegalAgreement;
    m_UgcSubmitItemUpdate_callback.call(result, needsLegalAgreement);
}

void SteamApi::OnUgcSubscribeItem(RemoteStorageSubscribePublishedFileResult_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    m_UgcSubscribeItem_callback.call(result);
}

void SteamApi::OnUgcUnsubscribeItem(RemoteStorageUnsubscribePublishedFileResult_t *pResult, bool bIOFailure)
{
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_eResult);
    m_UgcUnsubscribeItem_callback.call(result);
}

// --- Inventory ---
void SteamApi::GetAllItems(const Callable &callback)
{
    if (!GetIsReady()) return;
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->GetAllItems(&handle);
    if (handle != k_SteamInventoryResultInvalid)
        singleton->InventoryResultCallbacks.insert(static_cast<int>(handle), callback);
}

void SteamApi::GetItemsByDefinition(int definitionId, const Callable &callback)
{
    if (!GetIsReady()) return;
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->GetAllItems(&handle);
    if (handle != k_SteamInventoryResultInvalid)
    {
        singleton->InventoryResultCallbacks.insert(static_cast<int>(handle), callback);
        singleton->InventoryResultFilters.insert(static_cast<int>(handle), definitionId);
    }
}

void SteamApi::AddPromoItem(int definitionId)
{
    if (!GetIsReady()) return;
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->AddPromoItem(&handle, static_cast<SteamItemDef_t>(definitionId));
}

void SteamApi::AddPromoItems(const Array &definitionIds)
{
    if (!GetIsReady()) return;
    std::vector<SteamItemDef_t> defs;
    for (int i = 0; i < definitionIds.size(); i++)
        defs.push_back(static_cast<SteamItemDef_t>((int)definitionIds[i]));
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->AddPromoItems(&handle, defs.data(), static_cast<uint32>(defs.size()));
}

void SteamApi::ConsumeItem(int itemId, int quantity)
{
    if (!GetIsReady()) return;
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->ConsumeItem(&handle, static_cast<SteamItemInstanceID_t>(itemId), static_cast<uint32>(quantity));
}

void SteamApi::TriggerItemDrop(int definitionId)
{
    if (!GetIsReady()) return;
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->TriggerItemDrop(&handle, static_cast<SteamItemDef_t>(definitionId));
}

void SteamApi::ExchangeItems(const Array &genDefIds, const Array &genQty, const Array &destItemIds, const Array &destQty)
{
    if (!GetIsReady()) return;
    std::vector<SteamItemDef_t> defs;
    std::vector<uint32> defQtys;
    for (int i = 0; i < genDefIds.size(); i++)
    {
        defs.push_back(static_cast<SteamItemDef_t>((int)genDefIds[i]));
        defQtys.push_back(static_cast<uint32>((int)genQty[i]));
    }
    std::vector<SteamItemInstanceID_t> ids;
    std::vector<uint32> idQtys;
    for (int i = 0; i < destItemIds.size(); i++)
    {
        ids.push_back(static_cast<SteamItemInstanceID_t>((int)destItemIds[i]));
        idQtys.push_back(static_cast<uint32>((int)destQty[i]));
    }
    SteamInventoryResult_t handle = k_SteamInventoryResultInvalid;
    SteamInventory()->ExchangeItems(&handle,
        defs.data(), defQtys.data(), static_cast<uint32>(defs.size()),
        ids.data(), idQtys.data(), static_cast<uint32>(ids.size()));
}

void SteamApi::RequestPrices(const Callable &callback)
{
    if (!GetIsReady()) return;
    singleton->m_InventoryRequestPrices_callback = callback;
    const auto handle = SteamInventory()->RequestPrices();
    singleton->m_InventoryRequestPrices_t.Set(handle, singleton, &SteamApi::OnInventoryRequestPricesResult);
}

void SteamApi::OnInventoryRequestPricesResult(SteamInventoryRequestPricesResult_t *pResult, bool bIOFailure)
{
    const String currency = bIOFailure ? "" : String(pResult->m_rgchCurrency);
    const int result = bIOFailure ? static_cast<int>(k_EResultIOFailure) : static_cast<int>(pResult->m_result);
    m_InventoryRequestPrices_callback.call(result, currency);
}

void SteamApi::_enter_tree()
{
    SetSingleton(this);
    Engine::get_singleton()->register_singleton("SteamApi", this);
    if (debug) UtilityFunctions::print("[SteamApi] Registered as Engine singleton");
}

void SteamApi::_exit_tree()
{
    if (debug) UtilityFunctions::print("[SteamApi] Shutting down and unregistering singleton");
    Engine::get_singleton()->unregister_singleton("SteamApi");
    SetSingleton(nullptr);
}

void SteamApi::_ready()
{
    if (autoInitialise)
    {
        if (debug) UtilityFunctions::print("[SteamApi] AutoInitialise enabled; initialising client");
        SelfInitialised = true;
        InitialiseClient();
    }
    else if (debug)
        UtilityFunctions::print("[SteamApi] AutoInitialise disabled; call InitialiseClient() or InitialiseServer() manually");
}

void SteamApi::pump_callbacks(double delta)
{
    if (!IsReady) return;
    if (is_client)
        SteamAPI_RunCallbacks();
    else
        SteamGameServer_RunCallbacks();

    // Process queued leaderboard find requests
    if (!LeaderboardFindRequests.empty() && !m_LeaderboardFindResult_t.IsActive())
    {
        m_LeaderboardPending = LeaderboardFindRequests.front();
        LeaderboardFindRequests.pop();
        SteamAPICall_t handle;
        if (m_LeaderboardPending.createIfMissing)
            handle = SteamUserStats()->FindOrCreateLeaderboard(m_LeaderboardPending.leaderboard.utf8().get_data(),
                m_LeaderboardPending.lowestScoreIsTopRank ? k_ELeaderboardSortMethodAscending : k_ELeaderboardSortMethodDescending,
                static_cast<ELeaderboardDisplayType>(m_LeaderboardPending.displayType));
        else
            handle = SteamUserStats()->FindLeaderboard(m_LeaderboardPending.leaderboard.utf8().get_data());
        m_LeaderboardFindResult_t.Set(handle, this, &SteamApi::OnLeaderboardFindResult);
    }

    // Process queued leaderboard download requests
    if (!LeaderboardDownloadRequests.empty() && !m_LeaderboardScoresDownloaded_t.IsActive())
    {
        m_LeaderboardScoresPending = LeaderboardDownloadRequests.front();
        LeaderboardDownloadRequests.pop();
        SteamAPICall_t handle;
        if (m_LeaderboardScoresPending.dataRequest == k_ELeaderboardDataRequestUsers)
        {
            std::vector<CSteamID> ids;
            for (int i = 0; i < m_LeaderboardScoresPending.userReferences.size(); i++)
            {
                Ref<UserData> u = m_LeaderboardScoresPending.userReferences[i];
                ids.push_back(u->ToCSteamID());
            }
            for (int i = 0; i < m_LeaderboardScoresPending.userIds.size(); i++)
                ids.push_back(CSteamID(static_cast<uint64>(m_LeaderboardScoresPending.userIds[i])));
            handle = SteamUserStats()->DownloadLeaderboardEntriesForUsers(m_LeaderboardScoresPending.leaderboard, ids.data(), ids.size());
        }
        else
        {
            handle = SteamUserStats()->DownloadLeaderboardEntries(m_LeaderboardScoresPending.leaderboard, m_LeaderboardScoresPending.dataRequest, m_LeaderboardScoresPending.start, m_LeaderboardScoresPending.end);
        }
        m_LeaderboardScoresDownloaded_t.Set(handle, this, &SteamApi::OnLeaderboardScoresDownloaded);
    }
}

// =========================================================
// Steam Callbacks
// =========================================================

void SteamApi::EventUserStatsReceived(UserStatsReceived_t *pCallback)
{
    emit_signal("OnUserStatsReceived", static_cast<uint64_t>(pCallback->m_nGameID), static_cast<int>(pCallback->m_eResult), static_cast<uint64_t>(pCallback->m_steamIDUser.ConvertToUint64()));
}

void SteamApi::EventUserStatsUnloaded(UserStatsUnloaded_t *pCallback)
{
    emit_signal("OnUserStatsUnloaded", static_cast<uint64_t>(pCallback->m_steamIDUser.ConvertToUint64()));
}

void SteamApi::EventUserStatsStored(UserStatsStored_t *pCallback)
{
    emit_signal("OnUserStatsStored", static_cast<uint64_t>(pCallback->m_nGameID), static_cast<int>(pCallback->m_eResult));
}

void SteamApi::EventUserAchievementStored(UserAchievementStored_t *pCallback)
{
    emit_signal("OnUserAchievementStored", static_cast<uint64_t>(pCallback->m_nGameID), pCallback->m_bGroupAchievement, String(pCallback->m_rgchAchievementName), pCallback->m_nCurProgress, pCallback->m_nMaxProgress);
}

void SteamApi::EventDlcInstalled(DlcInstalled_t *pCallback)
{
    emit_signal("OnDlcInstalled", static_cast<int>(pCallback->m_nAppID));
}

void SteamApi::EventSteamServerConnectFailure(SteamServerConnectFailure_t *pCallback)
{
    emit_signal("OnSteamServerConnectFailure", static_cast<int>(pCallback->m_eResult), pCallback->m_bStillRetrying);
}

void SteamApi::EventSteamServersConnected(SteamServersConnected_t *pCallback)
{
    emit_signal("OnSteamServersConnected");
}

void SteamApi::EventSteamServersDisconnected(SteamServersDisconnected_t *pCallback)
{
    emit_signal("OnSteamServersDisconnected", static_cast<int>(pCallback->m_eResult));
}

void SteamApi::EventFriendRichPresenceUpdate(FriendRichPresenceUpdate_t *pCallback)
{
    emit_signal("OnFriendRichPresenceUpdate", static_cast<uint64_t>(pCallback->m_steamIDFriend.ConvertToUint64()), static_cast<int>(pCallback->m_nAppID));
}

void SteamApi::EventPersonaStateChange(PersonaStateChange_t *pCallback)
{
    emit_signal("OnPersonaStateChange", static_cast<uint64_t>(pCallback->m_ulSteamID), static_cast<int>(pCallback->m_nChangeFlags));
}

void SteamApi::EventAvatarImageLoaded(AvatarImageLoaded_t *pCallback)
{
    LoadAvatar(pCallback->m_steamID);
}

void SteamApi::EventGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t *pCallback)
{
    emit_signal("OnGameRichPresenceJoinRequested", static_cast<uint64_t>(pCallback->m_steamIDFriend.ConvertToUint64()), String(pCallback->m_rgchConnect));
}

void SteamApi::EventGameLobbyJoinRequested(GameLobbyJoinRequested_t *pCallback)
{
    emit_signal("OnGameLobbyJoinRequested", static_cast<uint64_t>(pCallback->m_steamIDLobby.ConvertToUint64()), static_cast<uint64_t>(pCallback->m_steamIDFriend.ConvertToUint64()));
}

void SteamApi::EventGameOverlayActivated(GameOverlayActivated_t *pCallback)
{
    emit_signal("OnGameOverlayActivated", static_cast<bool>(pCallback->m_bActive));
}

void SteamApi::PopulateItemDefinitionProperties(SteamItemDef_t definitionId, Dictionary &out_properties, Dictionary &out_tags)
{
    // Passing nullptr for pchPropertyName is documented Steamworks behaviour: it returns
    // the full list of defined property names for this item definition as a comma
    // delimited string, using the SDK's standard two-call size-then-fetch idiom.
    uint32 nameBufSize = 0;
    SteamInventory()->GetItemDefinitionProperty(definitionId, nullptr, nullptr, &nameBufSize);
    if (nameBufSize == 0)
        return;

    std::vector<char> nameBuf(nameBufSize);
    if (!SteamInventory()->GetItemDefinitionProperty(definitionId, nullptr, nameBuf.data(), &nameBufSize))
        return;

    const PackedStringArray names = String(nameBuf.data()).split(",");
    for (int i = 0; i < names.size(); i++)
    {
        const String key = names[i].strip_edges();
        if (key.is_empty())
            continue;

        const CharString keyUtf8 = key.utf8();
        uint32 valueBufSize = 0;
        SteamInventory()->GetItemDefinitionProperty(definitionId, keyUtf8.get_data(), nullptr, &valueBufSize);
        if (valueBufSize == 0)
            continue;

        std::vector<char> valueBuf(valueBufSize);
        if (!SteamInventory()->GetItemDefinitionProperty(definitionId, keyUtf8.get_data(), valueBuf.data(), &valueBufSize))
            continue;

        const String value = String(valueBuf.data());
        out_properties[key] = value;

        // Convention: item definitions carry their tags in a "tags" property as a
        // comma delimited list of tag values. Stored as a set-like Dictionary (tag -> true).
        if (key == "tags")
        {
            const PackedStringArray tagParts = value.split(",");
            for (int t = 0; t < tagParts.size(); t++)
            {
                const String tag = tagParts[t].strip_edges();
                if (!tag.is_empty())
                    out_tags[tag] = true;
            }
        }
    }
}

void SteamApi::EventSteamInventoryResultReady(SteamInventoryResultReady_t *pCallback)
{
    const int handle = static_cast<int>(pCallback->m_handle);
    const int result = static_cast<int>(pCallback->m_result);
    emit_signal("OnSteamInventoryResultReady", handle, result);

    if (!singleton->InventoryResultCallbacks.has(handle))
        return;

    Callable callback = singleton->InventoryResultCallbacks.get(handle);
    singleton->InventoryResultCallbacks.erase(handle);

    const int defFilter = singleton->InventoryResultFilters.has(handle)
        ? singleton->InventoryResultFilters.get(handle) : -1;
    singleton->InventoryResultFilters.erase(handle);

    TypedArray<SteamInventoryItemDetail> items;
    if (pCallback->m_result == k_EResultOK)
    {
        uint32 count = 0;
        SteamInventory()->GetResultItems(pCallback->m_handle, nullptr, &count);
        if (count > 0)
        {
            std::vector<SteamItemDetails_t> details(count);
            SteamInventory()->GetResultItems(pCallback->m_handle, details.data(), &count);
            for (uint32 i = 0; i < count; i++)
            {
                if (defFilter >= 0 && static_cast<int>(details[i].m_iDefinition) != defFilter)
                    continue;
                Ref<SteamInventoryItemDetail> item = memnew(SteamInventoryItemDetail);
                item->setItemId(static_cast<int>(details[i].m_itemId));
                item->setDefinitionId(static_cast<int>(details[i].m_iDefinition));
                item->setQuantity(static_cast<int>(details[i].m_unQuantity));
                item->setFlags(static_cast<int>(details[i].m_unFlags));

                Dictionary properties;
                Dictionary tags;
                PopulateItemDefinitionProperties(details[i].m_iDefinition, properties, tags);
                item->setProperties(properties);
                item->setTags(tags);
                // dynamicProperties (per-instance JSON via GetResultItemProperty) is not
                // yet populated — left as a known gap pending SDK-version verification.

                items.append(item);
            }
        }
    }
    SteamInventory()->DestroyResult(pCallback->m_handle);
    callback.call(items, result);
}

void SteamApi::EventSteamInventoryDefinitionUpdate(SteamInventoryDefinitionUpdate_t *pCallback)
{
    emit_signal("OnSteamInventoryDefinitionUpdate");
}

void SteamApi::EventMicroTxnAuthorizationResponse(MicroTxnAuthorizationResponse_t *pCallback)
{
    emit_signal("OnMicroTxnAuthorizationResponse", static_cast<int>(pCallback->m_unAppID), static_cast<uint64_t>(pCallback->m_ulOrderID), static_cast<bool>(pCallback->m_bAuthorized));
}

void SteamApi::EventFavoritesListAccountsUpdated(FavoritesListAccountsUpdated_t *pCallback)
{
    emit_signal("OnFavoritesListAccountsUpdated", static_cast<int>(pCallback->m_eResult));
}

void SteamApi::EventFavoritesListChanged(FavoritesListChanged_t *pCallback)
{
    emit_signal("OnFavoritesListChanged");
}

void SteamApi::EventLobbyChatMsg(LobbyChatMsg_t *pCallback)
{
    CSteamID lobby(pCallback->m_ulSteamIDLobby);
    CSteamID user(pCallback->m_ulSteamIDUser);
    char buf[4096];
    EChatEntryType entryType;
    int size = SteamMatchmaking()->GetLobbyChatEntry(lobby, pCallback->m_iChatID, nullptr, buf, sizeof(buf), &entryType);
    emit_signal("OnLobbyChatMsg", static_cast<uint64_t>(lobby.ConvertToUint64()), static_cast<uint64_t>(user.ConvertToUint64()), String(buf).substr(0, size));
}

void SteamApi::EventLobbyChatUpdate(LobbyChatUpdate_t *pCallback)
{
    emit_signal("OnLobbyChatUpdate", static_cast<uint64_t>(pCallback->m_ulSteamIDLobby), static_cast<uint64_t>(pCallback->m_ulSteamIDUserChanged), static_cast<uint64_t>(pCallback->m_ulSteamIDMakingChange), static_cast<int>(pCallback->m_rgfChatMemberStateChange));
}

void SteamApi::EventLobbyDataUpdate(LobbyDataUpdate_t *pCallback)
{
    emit_signal("OnLobbyDataUpdate", static_cast<uint64_t>(pCallback->m_ulSteamIDLobby), static_cast<uint64_t>(pCallback->m_ulSteamIDMember), static_cast<bool>(pCallback->m_bSuccess));
}

void SteamApi::EventLobbyGameCreated(LobbyGameCreated_t *pCallback)
{
    emit_signal("OnLobbyGameCreated", static_cast<uint64_t>(pCallback->m_ulSteamIDLobby), static_cast<uint64_t>(pCallback->m_ulSteamIDGameServer), pCallback->m_unIP, pCallback->m_usPort);
}

void SteamApi::EventLobbyInvite(LobbyInvite_t *pCallback)
{
    emit_signal("OnLobbyInvite", static_cast<uint64_t>(pCallback->m_ulSteamIDUser), static_cast<uint64_t>(pCallback->m_ulSteamIDLobby));
}

void SteamApi::EventRemotePlaySessionConnected(SteamRemotePlaySessionConnected_t *pCallback)
{
    emit_signal("OnRemotePlaySessionConnected", static_cast<int>(pCallback->m_unSessionID));
}

void SteamApi::EventRemotePlaySessionDisconnected(SteamRemotePlaySessionDisconnected_t *pCallback)
{
    emit_signal("OnRemotePlaySessionDisconnected", static_cast<int>(pCallback->m_unSessionID));
}

void SteamApi::EventRemotePlaySessionGuestInvite(SteamRemotePlayTogetherGuestInvite_t *pCallback)
{
    emit_signal("OnRemotePlaySessionGuestInvite", String(pCallback->m_szConnectURL));
}

void SteamApi::EventValidateAuthTicket(ValidateAuthTicketResponse_t *pCallback)
{
    emit_signal("OnValidateAuthTicket", static_cast<uint64_t>(pCallback->m_SteamID.ConvertToUint64()), static_cast<int>(pCallback->m_eAuthSessionResponse));
}

#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 57)
void SteamApi::EventGetTicketForWebApiResponse(GetTicketForWebApiResponse_t *pCallback)
{
    emit_signal("OnGetTicketForWebApiResponse", static_cast<int>(pCallback->m_hAuthTicket), static_cast<int>(pCallback->m_eResult));
}
#endif

void SteamApi::EventSteamGameServerConnectFailure(SteamServerConnectFailure_t *pCallback)
{
    emit_signal("OnSteamGameServerConnectFailure", static_cast<int>(pCallback->m_eResult), pCallback->m_bStillRetrying);
}

void SteamApi::EventSteamGameServersConnected(SteamServersConnected_t *pCallback)
{
    emit_signal("OnSteamGameServersConnected");
}

void SteamApi::EventSteamGameServersDisconnected(SteamServersDisconnected_t *pCallback)
{
    emit_signal("OnSteamGameServersDisconnected", static_cast<int>(pCallback->m_eResult));
}

// =========================================================
// CCallResult Handlers
// =========================================================

void SteamApi::OnLobbyCreated(LobbyCreated_t *pResult, bool bIOFailure)
{
    if (bIOFailure || pResult->m_eResult != k_EResultOK)
    {
        m_LobbyCreated_callback.call(Ref<LobbyData>());
        return;
    }
    Ref<LobbyData> lobby = memnew(LobbyData(pResult->m_ulSteamIDLobby));
    if (m_LobbyCreatedHint == LobbyUseHint::Hint::Session)
        SetLobbyUseHint(lobby, LobbyUseHint::Hint::Session);
    else if (m_LobbyCreatedHint == LobbyUseHint::Hint::Party)
        SetLobbyUseHint(lobby, LobbyUseHint::Hint::Party);
    MemberOfLobbies.append(static_cast<int64_t>(pResult->m_ulSteamIDLobby));
    emit_signal("OnLobbyCreated", lobby);
    m_LobbyCreated_callback.call(lobby);
}

void SteamApi::OnLobbyEnter(LobbyEnter_t *pResult, bool bIOFailure)
{
    if (bIOFailure)
    {
        m_LobbyEnter_callback.call(Ref<LobbyData>(), static_cast<int>(LobbyEnterResponse::EnterResponse::Error));
        return;
    }
    Ref<LobbyData> lobby = memnew(LobbyData(pResult->m_ulSteamIDLobby));
    MemberOfLobbies.append(static_cast<int64_t>(pResult->m_ulSteamIDLobby));
    emit_signal("OnLobbyEntered", lobby, static_cast<int>(pResult->m_EChatRoomEnterResponse));
    m_LobbyEnter_callback.call(lobby, static_cast<int>(pResult->m_EChatRoomEnterResponse));
}

void SteamApi::OnLobbyMatchList(LobbyMatchList_t *pResult, bool bIOFailure)
{
    TypedArray<LobbyData> lobbies;
    if (!bIOFailure)
    {
        for (uint32 i = 0; i < pResult->m_nLobbiesMatching; i++)
            lobbies.append(memnew(LobbyData(SteamMatchmaking()->GetLobbyByIndex(i))));
    }
    emit_signal("OnLobbyMatchList", lobbies);
    m_LobbyMatchList_callback.call(lobbies);
}

void SteamApi::OnLeaderboardFindResult(LeaderboardFindResult_t *pResult, bool bIOFailure)
{
    if (!bIOFailure && pResult->m_bLeaderboardFound)
    {
        const String name = String(SteamUserStats()->GetLeaderboardName(pResult->m_hSteamLeaderboard));
        LeaderboardMap.insert(name, pResult->m_hSteamLeaderboard);
        emit_signal("OnLeaderboardFound", name);
        if (m_LeaderboardPending.onComplete)
            m_LeaderboardPending.onComplete();
    }

    // Process next queued leaderboard
    if (!LeaderboardFindRequests.empty())
    {
        m_LeaderboardPending = LeaderboardFindRequests.front();
        LeaderboardFindRequests.pop();
        const auto handle = SteamUserStats()->FindLeaderboard(m_LeaderboardPending.leaderboard.utf8().get_data());
        m_LeaderboardFindResult_t.Set(handle, this, &SteamApi::OnLeaderboardFindResult);
    }
    else if (!IsReady)
    {
        IsReady = true;
        emit_signal("OnReady");
    }
}

void SteamApi::OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure)
{
    TypedArray<LeaderboardEntryData> entries;
    if (!bIOFailure && pResult->m_cEntryCount > 0)
    {
        const int detailCount = m_LeaderboardScoresPending.detailCount;
        std::vector<int32> details(detailCount > 0 ? detailCount : 1);
        for (int i = 0; i < pResult->m_cEntryCount; i++)
        {
            LeaderboardEntry_t entry;
            SteamUserStats()->GetDownloadedLeaderboardEntry(pResult->m_hSteamLeaderboardEntries, i, &entry, details.data(), detailCount);
            PackedInt32Array detailArray;
            for (int d = 0; d < detailCount; d++) detailArray.append(details[d]);
            entries.append(LeaderboardEntryData::create(entry.m_steamIDUser, entry.m_nGlobalRank, entry.m_nScore, detailArray, entry.m_hUGC));
        }
    }
    m_LeaderboardScoresPending.onComplete.call(entries);
}

void SteamApi::OnNumberOfCurrentPlayers(NumberOfCurrentPlayers_t *pResult, bool bIOFailure)
{
    m_NumberOfCurrentPlayers_callback.call(bIOFailure ? 0 : pResult->m_cPlayers);
}

void SteamApi::OnGlobalAchievementPercentagesReady(GlobalAchievementPercentagesReady_t *pResult, bool bIOFailure)
{
    m_GlobalAchievementPercentagesReady_callback.call(static_cast<int>(pResult->m_eResult));
}

void SteamApi::OnGlobalStatsReceived(GlobalStatsReceived_t *pResult, bool bIOFailure)
{
    m_GlobalStatsReceived_callback.call(static_cast<int>(pResult->m_eResult));
}

void SteamApi::OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *pResult, bool bIOFailure)
{
    Callable callback = m_RemoteStorageFileShareResult_callback;
    const String leaderboard = m_RemoteStorageFileShareResult_leaderboard;
    m_RemoteStorageFileShareResult_callback = Callable();
    m_RemoteStorageFileShareResult_leaderboard = String();

    if (bIOFailure || pResult->m_eResult != k_EResultOK || !LeaderboardMap.has(leaderboard))
    {
        callback.call(false);
        return;
    }

    m_LeaderboardUgcSet_callback = callback;
    const auto apiCall = SteamUserStats()->AttachLeaderboardUGC(LeaderboardMap.get(leaderboard), pResult->m_hFile);
    m_LeaderboardUgcSet_t.Set(apiCall, this, &SteamApi::OnLeaderboardUgcSet);
}

void SteamApi::OnLeaderboardUgcSet(LeaderboardUGCSet_t *pResult, bool bIOFailure)
{
    Callable callback = m_LeaderboardUgcSet_callback;
    m_LeaderboardUgcSet_callback = Callable();
    callback.call(!bIOFailure && pResult->m_eResult == k_EResultOK);
}

void SteamApi::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *pResult, bool bIOFailure)
{
    if (bIOFailure || !pResult->m_bSuccess)
    {
        m_LeaderboardScoreUploaded_callback.call(Ref<LeaderboardEntryData>(), true);
        return;
    }
    Ref<LeaderboardEntryData> entry = LeaderboardEntryData::create(
        SteamUser()->GetSteamID(),
        pResult->m_nGlobalRankNew,
        pResult->m_nScore,
        PackedInt32Array(),
        0
    );
    emit_signal("OnLeaderboardScoreUploaded", entry, false);
    m_LeaderboardScoreUploaded_callback.call(entry, false);
}

// =========================================================
// _bind_methods
// =========================================================

void SteamApi::_bind_methods()
{
    // Signals
    ADD_SIGNAL(MethodInfo("OnReady"));
    ADD_SIGNAL(MethodInfo("OnUserStatsReceived", PropertyInfo(Variant::INT, "gameId"), PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "userId")));
    ADD_SIGNAL(MethodInfo("OnUserStatsUnloaded", PropertyInfo(Variant::INT, "userId")));
    ADD_SIGNAL(MethodInfo("OnUserStatsStored", PropertyInfo(Variant::INT, "gameId"), PropertyInfo(Variant::INT, "result")));
    ADD_SIGNAL(MethodInfo("OnUserAchievementStored", PropertyInfo(Variant::INT, "gameId"), PropertyInfo(Variant::BOOL, "groupAchievement"), PropertyInfo(Variant::STRING, "achievementName"), PropertyInfo(Variant::INT, "curProgress"), PropertyInfo(Variant::INT, "maxProgress")));
    ADD_SIGNAL(MethodInfo("OnDlcInstalled", PropertyInfo(Variant::INT, "appId")));
    ADD_SIGNAL(MethodInfo("OnSteamServerConnectFailure", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "stillRetrying")));
    ADD_SIGNAL(MethodInfo("OnSteamServersConnected"));
    ADD_SIGNAL(MethodInfo("OnSteamServersDisconnected", PropertyInfo(Variant::INT, "result")));
    ADD_SIGNAL(MethodInfo("OnFriendRichPresenceUpdate", PropertyInfo(Variant::INT, "steamId"), PropertyInfo(Variant::INT, "appId")));
    ADD_SIGNAL(MethodInfo("OnPersonaStateChange", PropertyInfo(Variant::INT, "steamId"), PropertyInfo(Variant::INT, "changeFlags")));
    ADD_SIGNAL(MethodInfo("OnGameRichPresenceJoinRequested", PropertyInfo(Variant::INT, "friendId"), PropertyInfo(Variant::STRING, "connectString")));
    ADD_SIGNAL(MethodInfo("OnGameLobbyJoinRequested", PropertyInfo(Variant::INT, "lobbyId"), PropertyInfo(Variant::INT, "friendId")));
    ADD_SIGNAL(MethodInfo("OnGameOverlayActivated", PropertyInfo(Variant::BOOL, "active")));
    ADD_SIGNAL(MethodInfo("OnSteamInventoryResultReady", PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::INT, "result")));
    ADD_SIGNAL(MethodInfo("OnSteamInventoryDefinitionUpdate"));
    ADD_SIGNAL(MethodInfo("OnMicroTxnAuthorizationResponse", PropertyInfo(Variant::INT, "appId"), PropertyInfo(Variant::INT, "orderId"), PropertyInfo(Variant::BOOL, "authorized")));
    ADD_SIGNAL(MethodInfo("OnFavoritesListAccountsUpdated", PropertyInfo(Variant::INT, "result")));
    ADD_SIGNAL(MethodInfo("OnFavoritesListChanged"));
    ADD_SIGNAL(MethodInfo("OnLobbyChatMsg", PropertyInfo(Variant::INT, "lobbyId"), PropertyInfo(Variant::INT, "userId"), PropertyInfo(Variant::STRING, "message")));
    ADD_SIGNAL(MethodInfo("OnLobbyChatUpdate", PropertyInfo(Variant::INT, "lobbyId"), PropertyInfo(Variant::INT, "userId"), PropertyInfo(Variant::INT, "makingChangeId"), PropertyInfo(Variant::INT, "changeFlags")));
    ADD_SIGNAL(MethodInfo("OnLobbyDataUpdate", PropertyInfo(Variant::INT, "lobbyId"), PropertyInfo(Variant::INT, "memberId"), PropertyInfo(Variant::BOOL, "success")));
    ADD_SIGNAL(MethodInfo("OnLobbyGameCreated", PropertyInfo(Variant::INT, "lobbyId"), PropertyInfo(Variant::INT, "gameServerId"), PropertyInfo(Variant::INT, "ip"), PropertyInfo(Variant::INT, "port")));
    ADD_SIGNAL(MethodInfo("OnLobbyInvite", PropertyInfo(Variant::INT, "userId"), PropertyInfo(Variant::INT, "lobbyId")));
    ADD_SIGNAL(MethodInfo("OnLobbyCreated", PropertyInfo(Variant::OBJECT, "lobby")));
    ADD_SIGNAL(MethodInfo("OnLobbyEntered", PropertyInfo(Variant::OBJECT, "lobby"), PropertyInfo(Variant::INT, "response")));
    ADD_SIGNAL(MethodInfo("OnLobbyMatchList", PropertyInfo(Variant::ARRAY, "lobbies")));
    ADD_SIGNAL(MethodInfo("OnLeaderboardFound", PropertyInfo(Variant::STRING, "name")));
    ADD_SIGNAL(MethodInfo("OnLeaderboardScoreUploaded", PropertyInfo(Variant::OBJECT, "entry"), PropertyInfo(Variant::BOOL, "ioFailure")));
    ADD_SIGNAL(MethodInfo("OnRemotePlaySessionConnected", PropertyInfo(Variant::INT, "sessionId")));
    ADD_SIGNAL(MethodInfo("OnRemotePlaySessionDisconnected", PropertyInfo(Variant::INT, "sessionId")));
    ADD_SIGNAL(MethodInfo("OnRemotePlaySessionGuestInvite", PropertyInfo(Variant::STRING, "url")));
    ADD_SIGNAL(MethodInfo("OnValidateAuthTicket", PropertyInfo(Variant::INT, "steamId"), PropertyInfo(Variant::INT, "response")));
    ADD_SIGNAL(MethodInfo("OnSteamGameServerConnectFailure", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::BOOL, "stillRetrying")));
    ADD_SIGNAL(MethodInfo("OnSteamGameServersConnected"));
    ADD_SIGNAL(MethodInfo("OnSteamGameServersDisconnected", PropertyInfo(Variant::INT, "result")));
    ADD_SIGNAL(MethodInfo("OnServerRequestResponded", PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::INT, "server")));
    ADD_SIGNAL(MethodInfo("OnServerRequestFailedToRespond", PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::INT, "server")));
    ADD_SIGNAL(MethodInfo("OnRefreshRequestComplete", PropertyInfo(Variant::INT, "handle"), PropertyInfo(Variant::INT, "response")));
    ADD_SIGNAL(MethodInfo("OnPingServerResponded", PropertyInfo(Variant::DICTIONARY, "server")));
    ADD_SIGNAL(MethodInfo("OnPingServerFailedToRespond"));
    ADD_SIGNAL(MethodInfo("OnAddPlayerDetailsToList", PropertyInfo(Variant::STRING, "playerName"), PropertyInfo(Variant::INT, "score"), PropertyInfo(Variant::FLOAT, "timePlayed")));
    ADD_SIGNAL(MethodInfo("OnPlayerDetailsFailedToRespond"));
    ADD_SIGNAL(MethodInfo("OnPlayerDetailsRefreshComplete"));
    ADD_SIGNAL(MethodInfo("OnServerRulesResponded", PropertyInfo(Variant::STRING, "rule"), PropertyInfo(Variant::STRING, "value")));
    ADD_SIGNAL(MethodInfo("OnServerRulesFailedToRespond"));
    ADD_SIGNAL(MethodInfo("OnServerRulesRefreshComplete"));

    // Properties
    ClassDB::bind_method(D_METHOD("get_debug"), &SteamApi::get_debug);
    ClassDB::bind_method(D_METHOD("set_debug", "v"), &SteamApi::set_debug);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Debug"), "set_debug", "get_debug");

    ClassDB::bind_method(D_METHOD("get_appId"), &SteamApi::get_appId);
    ClassDB::bind_method(D_METHOD("set_appId", "v"), &SteamApi::set_appId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "AppId"), "set_appId", "get_appId");

    ClassDB::bind_method(D_METHOD("get_autoInitialise"), &SteamApi::get_autoInitialise);
    ClassDB::bind_method(D_METHOD("set_autoInitialise", "v"), &SteamApi::set_autoInitialise);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "AutoInitialise"), "set_autoInitialise", "get_autoInitialise");

    ClassDB::bind_method(D_METHOD("get_autoLogOn"), &SteamApi::get_autoLogOn);
    ClassDB::bind_method(D_METHOD("set_autoLogOn", "v"), &SteamApi::set_autoLogOn);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "AutoLogOn"), "set_autoLogOn", "get_autoLogOn");

    ClassDB::bind_method(D_METHOD("get_leaderboardIds"), &SteamApi::get_leaderboardIds);
    ClassDB::bind_method(D_METHOD("set_leaderboardIds", "v"), &SteamApi::set_leaderboardIds);
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "LeaderboardIds"), "set_leaderboardIds", "get_leaderboardIds");

    // Static methods
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetIsReady"), &SteamApi::GetIsReady);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetMemberOfLobbies"), &SteamApi::GetMemberOfLobbies);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetFullConfiguration", "appId", "ipAddress", "gamePort", "queryPort", "vacEnabled", "gameVersion"), &SteamApi::SetFullConfiguration);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetMinimalConfiguration", "appId"), &SteamApi::SetMinimalConfiguration);
    ClassDB::bind_static_method("SteamApi", D_METHOD("InitialiseClient"), &SteamApi::InitialiseClient);
    ClassDB::bind_static_method("SteamApi", D_METHOD("InitialiseServer"), &SteamApi::InitialiseServer);

    // App
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAppBuildId"), &SteamApi::GetAppBuildId);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAppInstallDir", "appId"), &SteamApi::GetAppInstallDir);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAppOwner"), &SteamApi::GetAppOwner);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAvailableGameLanguages"), &SteamApi::GetAvailableGameLanguages);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetCurrentBetaName"), &SteamApi::GetCurrentBetaName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetCurrentGameLanguage"), &SteamApi::GetCurrentGameLanguage);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetDLCCount"), &SteamApi::GetDLCCount);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetDLC"), &SteamApi::GetDLC);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetEarliestPurchaseUnixTime", "appId"), &SteamApi::GetEarliestPurchaseUnixTime);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLaunchCommandLine"), &SteamApi::GetLaunchCommandLine);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLaunchQueryParam", "key"), &SteamApi::GetLaunchQueryParam);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsAppInstalled", "appId"), &SteamApi::IsAppInstalled);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsCybercafe"), &SteamApi::IsCybercafe);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsLowViolence"), &SteamApi::IsLowViolence);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsSubscribed"), &SteamApi::IsSubscribed);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsSubscribedApp", "appId"), &SteamApi::IsSubscribedApp);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsSubscribedFromFamilySharing"), &SteamApi::IsSubscribedFromFamilySharing);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsSubscribedFromFreeWeekend"), &SteamApi::IsSubscribedFromFreeWeekend);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsTimedTrial"), &SteamApi::IsTimedTrial);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsVACBanned"), &SteamApi::IsVACBanned);
    ClassDB::bind_static_method("SteamApi", D_METHOD("MarkContentCorrupt", "missingFilesOnly"), &SteamApi::MarkContentCorrupt);

    // Overlay
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateGameOverlay", "type"), &SteamApi::ActivateGameOverlay);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateGameOverlayInviteDialogConnectString", "connectString"), &SteamApi::ActivateGameOverlayInviteDialogConnectString);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateGameOverlayToStore", "appId"), &SteamApi::ActivateGameOverlayToStore);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateGameOverlayToWebPage", "url", "modal"), &SteamApi::ActivateGameOverlayToWebPage);

    // Leaderboards
    ClassDB::bind_static_method("SteamApi", D_METHOD("FindLeaderboard", "leaderboard"), &SteamApi::FindLeaderboard);
    ClassDB::bind_static_method("SteamApi", D_METHOD("FindOrCreateLeaderboard", "leaderboard", "lowestScoreIsTopRank", "displayType"), &SteamApi::FindOrCreateLeaderboard);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLeaderboardNativeId", "leaderboard"), &SteamApi::GetLeaderboardNativeId);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLeaderboardDisplayType", "leaderboard"), &SteamApi::GetLeaderboardDisplayType);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLeaderboardEntryCount", "leaderboard"), &SteamApi::GetLeaderboardEntryCount);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLeaderboardName", "leaderboard"), &SteamApi::GetLeaderboardName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsLeaderboardTopRankLowestScore", "leaderboard"), &SteamApi::IsLeaderboardTopRankLowestScore);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DownloadLeaderboardGlobalEntries", "start", "end", "detailCount", "leaderboard", "callback"), &SteamApi::DownloadLeaderboardGlobalEntries);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DownloadLeaderboardAroundUserEntries", "start", "end", "detailCount", "leaderboard", "callback"), &SteamApi::DownloadLeaderboardAroundUserEntries);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DownloadLeaderboardFriendsEntries", "start", "end", "detailCount", "leaderboard", "callback"), &SteamApi::DownloadLeaderboardFriendsEntries);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DownloadLeaderboardEntriesForUsers", "users", "detailCount", "leaderboard", "callback"), &SteamApi::DownloadLeaderboardEntriesForUsers);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DownloadLeaderboardEntriesForUserIds", "users", "detailCount", "leaderboard", "callback"), &SteamApi::DownloadLeaderboardEntriesForUserIds);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UploadLeaderboardScore", "leaderboard", "score", "callback"), &SteamApi::UploadLeaderboardScore);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UploadLeaderboardScoreWithDetails", "leaderboard", "score", "details", "callback"), &SteamApi::UploadLeaderboardScoreWithDetails);
    ClassDB::bind_static_method("SteamApi", D_METHOD("AttachLeaderboardFile", "file", "leaderboard", "callback"), &SteamApi::AttachLeaderboardFile);

    // Stats & Achievements
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetNumberOfCurrentPlayers", "callback"), &SteamApi::GetNumberOfCurrentPlayers);
    ClassDB::bind_static_method("SteamApi", D_METHOD("RequestUserRefStats", "user"), &SteamApi::RequestUserRefStats);
    ClassDB::bind_static_method("SteamApi", D_METHOD("RequestUserIdStats", "steamId"), &SteamApi::RequestUserIdStats);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ResetAllStats", "achievementsToo"), &SteamApi::ResetAllStats);
    ClassDB::bind_static_method("SteamApi", D_METHOD("StoreStats"), &SteamApi::StoreStats);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetAchievement", "achievement"), &SteamApi::SetAchievement);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsAchievementUnlocked", "achievement"), &SteamApi::IsAchievementUnlocked);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsUserAchievementUnlocked", "user", "achievement"), &SteamApi::IsUserAchievementUnlocked);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementUnlockTime", "achievement"), &SteamApi::GetAchievementUnlockTime);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetUserAchievementUnlockTime", "user", "achievement"), &SteamApi::GetUserAchievementUnlockTime);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementAchievedPercent", "achievement"), &SteamApi::GetAchievementAchievedPercent);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementName", "achievement"), &SteamApi::GetAchievementName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementDescription", "achievement"), &SteamApi::GetAchievementDescription);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementIsHidden", "achievement"), &SteamApi::GetAchievementIsHidden);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievementIcon", "achievement", "callback"), &SteamApi::GetAchievementIcon);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAchievements"), &SteamApi::GetAchievements);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetMostAchievedInfo"), &SteamApi::GetMostAchievedInfo);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IndicateAchievementProgress", "achievement", "current", "max"), &SteamApi::IndicateAchievementProgress);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ClearAchievement", "achievement"), &SteamApi::ClearAchievement);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetStatFloat", "stat", "value"), &SteamApi::SetStatFloat);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetStatInt", "stat", "value"), &SteamApi::SetStatInt);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UpdateAvgRateStat", "stat", "thisSession", "sessionLength"), &SteamApi::UpdateAvgRateStat);
    ClassDB::bind_static_method("SteamApi", D_METHOD("RequestGlobalAchievementPercentages", "callback"), &SteamApi::RequestGlobalAchievementPercentages);
    ClassDB::bind_static_method("SteamApi", D_METHOD("RequestGlobalStats", "historyDays", "callback"), &SteamApi::RequestGlobalStats);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetGlobalStatInt", "stat"), &SteamApi::GetGlobalStatInt);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetGlobalStatFloat", "stat"), &SteamApi::GetGlobalStatFloat);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetStatFloat", "stat"), &SteamApi::GetStatFloat);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetStatInt", "stat"), &SteamApi::GetStatInt);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetUserStatFloat", "user", "stat"), &SteamApi::GetUserStatFloat);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetUserStatInt", "user", "stat"), &SteamApi::GetUserStatInt);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetGlobalStatIntHistory", "stat", "historyDays"), &SteamApi::GetGlobalStatIntHistory);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetGlobalStatFloatHistory", "stat", "historyDays"), &SteamApi::GetGlobalStatFloatHistory);

    // Friends / User
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetFriendAvatar", "user", "callback"), &SteamApi::GetFriendAvatar);

    // Matchmaking
    ClassDB::bind_static_method("SteamApi", D_METHOD("LeaveLobby", "lobby"), &SteamApi::LeaveLobby);
    ClassDB::bind_static_method("SteamApi", D_METHOD("CreateLobby", "type", "hint", "maxMembers", "callback"), &SteamApi::CreateLobby);
    ClassDB::bind_static_method("SteamApi", D_METHOD("JoinLobby", "lobby", "callback"), &SteamApi::JoinLobby);
    ClassDB::bind_static_method("SteamApi", D_METHOD("JoinLobbyById", "lobbyId", "callback"), &SteamApi::JoinLobbyById);
    ClassDB::bind_static_method("SteamApi", D_METHOD("JoinLobbyByHex", "hexId", "callback"), &SteamApi::JoinLobbyByHex);
    ClassDB::bind_static_method("SteamApi", D_METHOD("LobbyMatchList", "callback"), &SteamApi::LobbyMatchList);

    ClassDB::bind_static_method("SteamApi", D_METHOD("InviteUserToLobby", "lobby", "user"), &SteamApi::InviteUserToLobby);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyData", "lobby", "key", "value"), &SteamApi::SetLobbyData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyData", "lobby", "key"), &SteamApi::GetLobbyData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyMemberData", "lobby", "key", "value"), &SteamApi::SetLobbyMemberData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyMemberData", "lobby", "user", "key"), &SteamApi::GetLobbyMemberData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyName", "lobby"), &SteamApi::GetLobbyName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyName", "lobby", "name"), &SteamApi::SetLobbyName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyMemberCount", "lobby"), &SteamApi::GetLobbyMemberCount);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyMaxMembers", "lobby"), &SteamApi::GetLobbyMaxMembers);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyMaxMembers", "lobby", "maxMembers"), &SteamApi::SetLobbyMaxMembers);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyUseHint", "lobby"), &SteamApi::GetLobbyUseHint);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyUseHint", "lobby", "hint"), &SteamApi::SetLobbyUseHint);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyType", "lobby"), &SteamApi::GetLobbyType);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyType", "lobby", "type"), &SteamApi::SetLobbyType);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsLobbyFull", "lobby"), &SteamApi::IsLobbyFull);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsLobbyOwner", "lobby"), &SteamApi::IsLobbyOwner);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyOwner", "lobby", "user"), &SteamApi::SetLobbyOwner);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsLobbyMember", "lobby"), &SteamApi::IsLobbyMember);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyJoinable", "lobby", "joinable"), &SteamApi::SetLobbyJoinable);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyListenServer", "lobby"), &SteamApi::SetLobbyListenServer);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetLobbyDedicatedServer", "lobby", "serverId", "ip", "port"), &SteamApi::SetLobbyDedicatedServer);
    ClassDB::bind_static_method("SteamApi", D_METHOD("LobbyHasGameServer", "lobby"), &SteamApi::LobbyHasGameServer);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyServerId", "lobby"), &SteamApi::GetLobbyServerId);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyServerIp", "lobby"), &SteamApi::GetLobbyServerIp);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyServerPort", "lobby"), &SteamApi::GetLobbyServerPort);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyMemberList", "lobby"), &SteamApi::GetLobbyMemberList);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetLobbyOwner", "lobby"), &SteamApi::GetLobbyOwner);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SendLobbyChatMessage", "lobby", "message"), &SteamApi::SendLobbyChatMessage);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateLobbyInviteDialog", "lobby"), &SteamApi::ActivateLobbyInviteDialog);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateLobbyRemotePlayTogetherInviteDialog", "lobby"), &SteamApi::ActivateLobbyRemotePlayTogetherInviteDialog);

    // Input
    ClassDB::bind_static_method("SteamApi", D_METHOD("InputInit", "explicitlyCallRunFrame"), &SteamApi::InputInit);
    ClassDB::bind_static_method("SteamApi", D_METHOD("InputShutdown"), &SteamApi::InputShutdown);
    ClassDB::bind_static_method("SteamApi", D_METHOD("InputRunFrame"), &SteamApi::InputRunFrame);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetConnectedControllers"), &SteamApi::GetConnectedControllers);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetActionSetHandle", "actionSetName"), &SteamApi::GetActionSetHandle);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ActivateActionSet", "inputHandle", "actionSetHandle"), &SteamApi::ActivateActionSet);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetCurrentActionSet", "inputHandle"), &SteamApi::GetCurrentActionSet);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetDigitalActionHandle", "actionName"), &SteamApi::GetDigitalActionHandle);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetDigitalActionData", "inputHandle", "actionHandle"), &SteamApi::GetDigitalActionData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAnalogActionHandle", "actionName"), &SteamApi::GetAnalogActionHandle);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAnalogActionData", "inputHandle", "actionHandle"), &SteamApi::GetAnalogActionData);
    ClassDB::bind_static_method("SteamApi", D_METHOD("TriggerVibration", "inputHandle", "leftSpeed", "rightSpeed"), &SteamApi::TriggerVibration);

    // Parties
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetNumActiveBeacons"), &SteamApi::GetNumActiveBeacons);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetBeaconByIndex", "index"), &SteamApi::GetBeaconByIndex);
    ClassDB::bind_static_method("SteamApi", D_METHOD("CreateBeacon", "openSlots", "lobby", "connectString", "metadata", "callback"), &SteamApi::CreateBeacon);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DestroyBeacon", "beaconHandle"), &SteamApi::DestroyBeacon);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ChangeNumOpenSlots", "beaconHandle", "openSlots", "callback"), &SteamApi::ChangeNumOpenSlots);

    // UGC / Workshop
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcCreateItem", "appId", "fileType", "callback"), &SteamApi::UgcCreateItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcStartItemUpdate", "appId", "publishedFileId"), &SteamApi::UgcStartItemUpdate);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSetItemTitle", "updateHandle", "title"), &SteamApi::UgcSetItemTitle);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSetItemDescription", "updateHandle", "description"), &SteamApi::UgcSetItemDescription);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSetItemContent", "updateHandle", "contentFolder"), &SteamApi::UgcSetItemContent);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSetItemPreview", "updateHandle", "previewFile"), &SteamApi::UgcSetItemPreview);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSetItemVisibility", "updateHandle", "visibility"), &SteamApi::UgcSetItemVisibility);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcAddItemTag", "updateHandle", "tag"), &SteamApi::UgcAddItemTag);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSubmitItemUpdate", "updateHandle", "changeNotes", "callback"), &SteamApi::UgcSubmitItemUpdate);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcSubscribeItem", "publishedFileId", "callback"), &SteamApi::UgcSubscribeItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcUnsubscribeItem", "publishedFileId", "callback"), &SteamApi::UgcUnsubscribeItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcGetNumSubscribedItems"), &SteamApi::UgcGetNumSubscribedItems);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcGetSubscribedItems"), &SteamApi::UgcGetSubscribedItems);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcDownloadItem", "publishedFileId", "highPriority"), &SteamApi::UgcDownloadItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcGetItemInstallInfo", "publishedFileId"), &SteamApi::UgcGetItemInstallInfo);
    ClassDB::bind_static_method("SteamApi", D_METHOD("UgcGetItemDownloadInfo", "publishedFileId"), &SteamApi::UgcGetItemDownloadInfo);

    // Remote Storage
    ClassDB::bind_static_method("SteamApi", D_METHOD("FileWrite", "fileName", "data"), &SteamApi::FileWrite);
    ClassDB::bind_static_method("SteamApi", D_METHOD("FileRead", "fileName"), &SteamApi::FileRead);
    ClassDB::bind_static_method("SteamApi", D_METHOD("FileDelete", "fileName"), &SteamApi::FileDelete);
    ClassDB::bind_static_method("SteamApi", D_METHOD("FileExists", "fileName"), &SteamApi::FileExists);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetFileSize", "fileName"), &SteamApi::GetFileSize);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetFileCount"), &SteamApi::GetFileCount);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetFileNameAndSize", "index"), &SteamApi::GetFileNameAndSize);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetQuota", "callback"), &SteamApi::GetQuota);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsCloudEnabledForApp"), &SteamApi::IsCloudEnabledForApp);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IsCloudEnabledForAccount"), &SteamApi::IsCloudEnabledForAccount);

    // Remote Play
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetRemotePlaySessionCount"), &SteamApi::GetRemotePlaySessionCount);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetRemotePlaySessionID", "index"), &SteamApi::GetRemotePlaySessionID);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetRemotePlaySessionSteamID", "sessionId"), &SteamApi::GetRemotePlaySessionSteamID);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetRemotePlaySessionClientName", "sessionId"), &SteamApi::GetRemotePlaySessionClientName);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SendRemotePlayTogetherInvite", "user", "callback"), &SteamApi::SendRemotePlayTogetherInvite);

    // Screenshots
    ClassDB::bind_static_method("SteamApi", D_METHOD("TriggerScreenshot"), &SteamApi::TriggerScreenshot);
    ClassDB::bind_static_method("SteamApi", D_METHOD("HookScreenshots", "hook"), &SteamApi::HookScreenshots);
    ClassDB::bind_static_method("SteamApi", D_METHOD("AddScreenshotToLibrary", "fileName", "thumbnailFileName", "width", "height"), &SteamApi::AddScreenshotToLibrary);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetScreenshotLocation", "handle", "location"), &SteamApi::SetScreenshotLocation);
    ClassDB::bind_static_method("SteamApi", D_METHOD("TagScreenshotUser", "handle", "user"), &SteamApi::TagScreenshotUser);
    ClassDB::bind_static_method("SteamApi", D_METHOD("TagPublishedFile", "handle", "publishedFileId"), &SteamApi::TagPublishedFile);

    // Voice
    ClassDB::bind_static_method("SteamApi", D_METHOD("StartVoiceRecording"), &SteamApi::StartVoiceRecording);
    ClassDB::bind_static_method("SteamApi", D_METHOD("StopVoiceRecording"), &SteamApi::StopVoiceRecording);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAvailableVoice"), &SteamApi::GetAvailableVoice);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetVoice"), &SteamApi::GetVoice);
    ClassDB::bind_static_method("SteamApi", D_METHOD("DecompressVoice", "compressed", "sampleRate"), &SteamApi::DecompressVoice);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetVoiceOptimalSampleRate"), &SteamApi::GetVoiceOptimalSampleRate);

    // Timeline
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetTimelineGamePhase", "mode"), &SteamApi::SetTimelineGamePhase);
    ClassDB::bind_static_method("SteamApi", D_METHOD("AddTimelineEvent", "title", "description", "icon", "priority", "startOffset", "duration"), &SteamApi::AddTimelineEvent);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetTimelineStateDescription", "description", "timeDelta"), &SteamApi::SetTimelineStateDescription);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ClearTimelineStateDescription", "timeDelta"), &SteamApi::ClearTimelineStateDescription);
    ClassDB::bind_static_method("SteamApi", D_METHOD("SetTimelineTooltip", "description", "timeDelta"), &SteamApi::SetTimelineTooltip);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ClearTimelineTooltip", "timeDelta"), &SteamApi::ClearTimelineTooltip);

    // Inventory
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetAllItems", "callback"), &SteamApi::GetAllItems);
    ClassDB::bind_static_method("SteamApi", D_METHOD("GetItemsByDefinition", "definitionId", "callback"), &SteamApi::GetItemsByDefinition);
    ClassDB::bind_static_method("SteamApi", D_METHOD("AddPromoItem", "definitionId"), &SteamApi::AddPromoItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("AddPromoItems", "definitionIds"), &SteamApi::AddPromoItems);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ConsumeItem", "itemId", "quantity"), &SteamApi::ConsumeItem);
    ClassDB::bind_static_method("SteamApi", D_METHOD("TriggerItemDrop", "definitionId"), &SteamApi::TriggerItemDrop);
    ClassDB::bind_static_method("SteamApi", D_METHOD("ExchangeItems", "genDefIds", "genQty", "destItemIds", "destQty"), &SteamApi::ExchangeItems);
    ClassDB::bind_static_method("SteamApi", D_METHOD("RequestPrices", "callback"), &SteamApi::RequestPrices);

    // Utilities
    ClassDB::bind_static_method("SteamApi", D_METHOD("IpStringToUint", "ip"), &SteamApi::IpStringToUint);
    ClassDB::bind_static_method("SteamApi", D_METHOD("IpUintToString", "ip"), &SteamApi::IpUintToString);
}
