#pragma once

#include "steam/steam_api_flat.h"
#include "steam/steamnetworkingfakeip.h"
#include "steam/isteamdualsense.h"
#include <steam/steam_gameserver.h>

#include "LobbyType.h"
#include "SteamResult.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <functional>
#include <queue>
#include <sstream>
#include <unordered_map>

using namespace godot;

class UserData;
class LobbyData;
class DlcData;
class SteamInitialisationResponse;
class SteamTimedTrial;

struct LeaderboardFindRequest
{
    String leaderboard;
    bool createIfMissing;
    bool lowestScoreIsTopRank;
    SteamLeaderboardDisplay::Type displayType;
    std::function<void()> onComplete;
};

struct LeaderboardDownloadRequest
{
    SteamLeaderboard_t leaderboard;
    ELeaderboardDataRequest dataRequest;
    int start, end, detailCount;
    Vector<Ref<UserData>> userReferences;
    PackedInt64Array userIds;
    Callable onComplete;
};

///<summary>
/// Singleton node; add to a Global (AutoLoad) scene. Wraps the full Steamworks SDK.
///</summary>
class SteamApi : public Node,
    ISteamMatchmakingServerListResponse,
    ISteamMatchmakingPingResponse,
    ISteamMatchmakingPlayersResponse,
    ISteamMatchmakingRulesResponse
{
    GDCLASS(SteamApi, Node);

private:
    static SteamApi *singleton;
    bool debug = false;
    bool autoInitialise = true;
    bool autoLogOn = true;
    int appId = 480;
    String ipAddress = "0.0.0.0";
    int gamePort = 27015;
    int queryPort = 27016;
    bool bVACEnabled = false;
    String gameVersion = "1.0.0.0";
    PackedStringArray leaderboardIds;
    Ref<SteamInitialisationResponse> initialisationResponse;
    bool is_client = false;
    bool InventorySnapshotDirty;
    bool PendingInventoryNotificationCallable;
    bool PendingInventoryFullRefresh;
    Callable InventoryPendingCallable;
    std::unordered_map<uint64, Callable> AvatarRequests;
    std::unordered_map<uint64, Ref<Texture2D>> AvatarCache;
    std::queue<LeaderboardFindRequest> LeaderboardFindRequests;
    std::queue<LeaderboardDownloadRequest> LeaderboardDownloadRequests;
    HashMap<String, SteamLeaderboard_t> LeaderboardMap;
    PackedInt64Array MemberOfLobbies;
    TypedArray<DlcData> DlcCollection;
    static void ClientArtifactLoad();

    static Dictionary GameServerItemToDictionary(gameserveritem_t *serverItem);

    // Enumerates an item definition's static KV properties via the documented
    // GetItemDefinitionProperty(id, nullptr, ...) "list of property names" idiom.
    // 'tags' is derived from the conventional comma-delimited "tags" property, if present.
    static void PopulateItemDefinitionProperties(SteamItemDef_t definitionId, Dictionary &out_properties, Dictionary &out_tags);

    ISteamMatchmakingServerListResponse *server_list_response = this;
    ISteamMatchmakingPingResponse *ping_response = this;
    ISteamMatchmakingPlayersResponse *players_response = this;
    ISteamMatchmakingRulesResponse *rules_response = this;

public:
    SteamApi();
    ~SteamApi() override;

    static SteamApi *GetSingleton();
    static PackedInt64Array GetMemberOfLobbies();
    static bool GetIsReady();
    static void SetSingleton(SteamApi *p_singleton);

    static void SetFullConfiguration(int app_id, const String &ip_address, int game_port, int query_port, bool bVACEnabled, const String &game_version);
    static void SetMinimalConfiguration(int app_id);
    static Ref<SteamInitialisationResponse> InitialiseClient();
    static Ref<SteamInitialisationResponse> InitialiseServer();

    // ISteamMatchmakingServerListResponse
    void ServerResponded(HServerListRequest listRequestHandle, int server) override;
    void ServerFailedToRespond(HServerListRequest listRequestHandle, int server) override;
    void RefreshComplete(HServerListRequest listRequestHandle, EMatchMakingServerResponse response) override;
    // ISteamMatchmakingPingResponse
    void ServerResponded(gameserveritem_t &server) override;
    void ServerFailedToRespond() override;
    // ISteamMatchmakingPlayersResponse
    void AddPlayerToList(const char *playerName, int score, float timePlayed) override;
    void PlayersFailedToRespond() override;
    void PlayersRefreshComplete() override;
    // ISteamMatchmakingRulesResponse
    void RulesResponded(const char *rule, const char *value) override;
    void RulesFailedToRespond() override;
    void RulesRefreshComplete() override;

    // Properties
    void set_debug(bool v);
    bool get_debug() const;
    void set_appId(int appId);
    void set_ipAddress(const String &ipAddress);
    void set_gamePort(int gamePort);
    void set_queryPort(int queryPort);
    void set_bVACEnabled(bool bVACEnabled);
    void set_gameVersion(const String &gameVersion);
    void set_autoInitialise(bool autoInitialise);
    void set_autoLogOn(bool autoLogOn);
    void set_leaderboardIds(const PackedStringArray &leaderboardIds);
    int get_appId() const;
    String get_ipAddress() const;
    int get_gamePort() const;
    int get_queryPort() const;
    bool get_bVACEnabled() const;
    String get_gameVersion() const;
    bool get_autoInitialise() const;
    bool get_autoLogOn() const;
    PackedStringArray get_leaderboardIds() const;

    // --- App ---
    static int GetAppBuildId();
    static String GetAppInstallDir(int appId);
    static Ref<UserData> GetAppOwner();
    static String GetAvailableGameLanguages();
    static String GetCurrentBetaName();
    static String GetCurrentGameLanguage();
    static int GetDLCCount();
    static TypedArray<DlcData> GetDLC();
    static int GetEarliestPurchaseUnixTime(int appId);
    static String GetLaunchCommandLine();
    static String GetLaunchQueryParam(const String &key);
    static bool IsAppInstalled(int appId);
    static bool IsCybercafe();
    static bool IsLowViolence();
    static bool IsSubscribed();
    static bool IsSubscribedApp(int appId);
    static bool IsSubscribedFromFamilySharing();
    static bool IsSubscribedFromFreeWeekend();
    static Ref<SteamTimedTrial> IsTimedTrial();
    static bool IsVACBanned();
    static bool MarkContentCorrupt(bool missingFilesOnly);

    // --- Overlay ---
    static void ActivateGameOverlay(const String &type);
    static void ActivateGameOverlayInviteDialogConnectString(const String &connectString);
    static void ActivateGameOverlayToStore(int app_id);
    static void ActivateGameOverlayToWebPage(const String &url, bool modal);

    // --- Leaderboards ---
    static void AttachLeaderboardFile(const String &file, const String &leaderboard, const Callable &callback);
    static void DownloadLeaderboardGlobalEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback);
    static void DownloadLeaderboardAroundUserEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback);
    static void DownloadLeaderboardFriendsEntries(int start, int end, int detailCount, const String &leaderboard, const Callable &callback);
    static void DownloadLeaderboardEntriesForUsers(TypedArray<UserData> users, int detailCount, const String &leaderboard, const Callable &callback);
    static void DownloadLeaderboardEntriesForUserIds(PackedInt64Array users, int detailCount, const String &leaderboard, const Callable &callback);
    static void FindLeaderboard(const String &leaderboard);
    static void FindOrCreateLeaderboard(const String &leaderboard, bool lowestScoreIsTopRank, SteamLeaderboardDisplay::Type displayType);
    static uint64_t GetLeaderboardNativeId(const String &leaderboard);
    static SteamLeaderboardDisplay::Type GetLeaderboardDisplayType(const String &leaderboard);
    static int GetLeaderboardEntryCount(const String &leaderboard);
    static String GetLeaderboardName(const String &leaderboard);
    static bool IsLeaderboardTopRankLowestScore(const String &leaderboard);
    static void UploadLeaderboardScore(const String &leaderboard, int score, const Callable &callback);
    static void UploadLeaderboardScoreWithDetails(const String &leaderboard, int score, const PackedInt32Array &details, const Callable &callback);

    // --- Stats & Achievements ---
    static void GetNumberOfCurrentPlayers(Callable callback);
    static void RequestUserRefStats(const Ref<UserData> &user);
    static void RequestUserIdStats(uint64_t steamId);
    static bool ResetAllStats(bool achievementsToo = true);
    static bool StoreStats();

    static bool SetAchievement(const String &achievement);
    static bool IsAchievementUnlocked(const String &achievement);
    static bool IsUserAchievementUnlocked(const Ref<UserData> &user, const String &achievement);
    static int GetAchievementUnlockTime(const String &achievement);
    static int GetUserAchievementUnlockTime(const Ref<UserData> &user, const String &achievement);
    static float GetAchievementAchievedPercent(const String &achievement);
    static String GetAchievementName(const String &achievement);
    static String GetAchievementDescription(const String &achievement);
    static bool GetAchievementIsHidden(const String &achievement);
    static void GetAchievementIcon(const String &achievement, const Callable &callback);
    static PackedStringArray GetAchievements();
    static TypedArray<Dictionary> GetMostAchievedInfo();
    static bool IndicateAchievementProgress(const String &achievement, int current, int max);
    static bool ClearAchievement(const String &achievement);

    static bool SetStatFloat(const String &stat, float value);
    static bool SetStatInt(const String &stat, int value);
    static bool UpdateAvgRateStat(const String &stat, float thisSession, double sessionLength);
    static void RequestGlobalAchievementPercentages(const Callable &callback);
    static void RequestGlobalStats(int historyDays, const Callable &callback);
    static uint64_t GetGlobalStatInt(const String &stat);
    static double GetGlobalStatFloat(const String &stat);
    static float GetStatFloat(const String &stat);
    static int GetStatInt(const String &stat);
    static float GetUserStatFloat(const Ref<UserData> &user, const String &stat);
    static int GetUserStatInt(const Ref<UserData> &user, const String &stat);
    static PackedInt64Array GetGlobalStatIntHistory(const String &stat, int historyDays);
    static PackedFloat64Array GetGlobalStatFloatHistory(const String &stat, int historyDays);

    // --- Friends / User ---
    static void GetFriendAvatar(const Ref<UserData> &user, const Callable &callback);

    // --- Matchmaking ---
    static void LeaveLobby(const Ref<LobbyData> &lobby);
    static void CreateLobby(LobbyType::Type type, LobbyUseHint::Hint hint, int maxMembers, const Callable &callback);
    static void JoinLobby(const Ref<LobbyData> &lobby, const Callable &callback);
    static void JoinLobbyById(int64_t lobby_id, const Callable &callback);
    static void JoinLobbyByHex(const String &hexId, const Callable &callback);
    static void LobbyMatchList(const Callable &callback);

    // --- Matchmaking: lobby metadata/property plumbing ---
    // Raw SDK operations only — the ergonomic LobbyDataExtensions convenience layer
    // (defaults, chaining, higher-level workflows) is Toolkit for Steamworks' to add.
    static void InviteUserToLobby(const Ref<LobbyData> &lobby, const Ref<UserData> &user);
    static void SetLobbyData(const Ref<LobbyData> &lobby, const String &key, const String &value);
    static String GetLobbyData(const Ref<LobbyData> &lobby, const String &key);
    static void SetLobbyMemberData(const Ref<LobbyData> &lobby, const String &key, const String &value);
    static String GetLobbyMemberData(const Ref<LobbyData> &lobby, const Ref<UserData> &user, const String &key);
    static String GetLobbyName(const Ref<LobbyData> &lobby);
    static void SetLobbyName(const Ref<LobbyData> &lobby, const String &name);
    static int GetLobbyMemberCount(const Ref<LobbyData> &lobby);
    static int GetLobbyMaxMembers(const Ref<LobbyData> &lobby);
    static void SetLobbyMaxMembers(const Ref<LobbyData> &lobby, int max_members);
    static LobbyUseHint::Hint GetLobbyUseHint(const Ref<LobbyData> &lobby);
    static void SetLobbyUseHint(const Ref<LobbyData> &lobby, LobbyUseHint::Hint hint);
    static LobbyType::Type GetLobbyType(const Ref<LobbyData> &lobby);
    static void SetLobbyType(const Ref<LobbyData> &lobby, LobbyType::Type type);
    static bool IsLobbyFull(const Ref<LobbyData> &lobby);
    static bool IsLobbyOwner(const Ref<LobbyData> &lobby);
    static void SetLobbyOwner(const Ref<LobbyData> &lobby, const Ref<UserData> &user);
    static bool IsLobbyMember(const Ref<LobbyData> &lobby);
    static void SetLobbyJoinable(const Ref<LobbyData> &lobby, bool joinable);
    static void SetLobbyListenServer(const Ref<LobbyData> &lobby);
    static void SetLobbyDedicatedServer(const Ref<LobbyData> &lobby, uint64_t serverId, const String &ip, uint16_t port);
    static bool LobbyHasGameServer(const Ref<LobbyData> &lobby);
    static uint64_t GetLobbyServerId(const Ref<LobbyData> &lobby);
    static String GetLobbyServerIp(const Ref<LobbyData> &lobby);
    static uint16_t GetLobbyServerPort(const Ref<LobbyData> &lobby);
    static TypedArray<UserData> GetLobbyMemberList(const Ref<LobbyData> &lobby);
    static Ref<UserData> GetLobbyOwner(const Ref<LobbyData> &lobby);
    static bool SendLobbyChatMessage(const Ref<LobbyData> &lobby, const String &message);
    static void ActivateLobbyInviteDialog(const Ref<LobbyData> &lobby);
    static void ActivateLobbyRemotePlayTogetherInviteDialog(const Ref<LobbyData> &lobby);

    // --- Remote Storage ---
    static bool FileWrite(const String &fileName, const PackedByteArray &data);
    static PackedByteArray FileRead(const String &fileName);
    static bool FileDelete(const String &fileName);
    static bool FileExists(const String &fileName);
    static int GetFileSize(const String &fileName);
    static int GetFileCount();
    static Array GetFileNameAndSize(int index);
    static void GetQuota(const Callable &callback);
    static bool IsCloudEnabledForApp();
    static bool IsCloudEnabledForAccount();

    // --- Remote Play ---
    static int GetRemotePlaySessionCount();
    static uint32_t GetRemotePlaySessionID(int index);
    static Ref<UserData> GetRemotePlaySessionSteamID(uint64_t sessionId);
    static String GetRemotePlaySessionClientName(uint64_t sessionId);
    static void SendRemotePlayTogetherInvite(const Ref<UserData> &user, const Callable &callback);

    // --- Screenshots ---
    static void TriggerScreenshot();
    static void HookScreenshots(bool hook);
    static uint32_t AddScreenshotToLibrary(const String &fileName, const String &thumbnailFileName, int width, int height);
    static void SetScreenshotLocation(uint32_t screenshotHandle, const String &location);
    static void TagScreenshotUser(uint32_t screenshotHandle, const Ref<UserData> &user);
    static void TagPublishedFile(int64_t screenshotHandle, int64_t publishedFileId);

    // --- Voice ---
    static void StartVoiceRecording();
    static void StopVoiceRecording();
    static int GetAvailableVoice();
    static PackedByteArray GetVoice();
    static PackedByteArray DecompressVoice(const PackedByteArray &compressed, int sampleRate);
    static int GetVoiceOptimalSampleRate();

    // --- Timeline ---
    static void SetTimelineGamePhase(int mode);
    static void AddTimelineEvent(const String &title, const String &description, const String &icon, int priority, float startOffset, float duration);
    static void SetTimelineStateDescription(const String &description, float timeDelta);
    static void ClearTimelineStateDescription(float timeDelta);
    static void SetTimelineTooltip(const String &description, float timeDelta);
    static void ClearTimelineTooltip(float timeDelta);

    // --- Input ---
    static void InputInit(bool explicitlyCallRunFrame);
    static void InputShutdown();
    static void InputRunFrame();
    static PackedInt64Array GetConnectedControllers();
    static uint64_t GetActionSetHandle(const String &actionSetName);
    static void ActivateActionSet(int64_t inputHandle, int64_t actionSetHandle);
    static uint64_t GetCurrentActionSet(int64_t inputHandle);
    static uint64_t GetDigitalActionHandle(const String &actionName);
    static bool GetDigitalActionData(int64_t inputHandle, int64_t actionHandle);
    static uint64_t GetAnalogActionHandle(const String &actionName);
    static Vector2 GetAnalogActionData(int64_t inputHandle, int64_t actionHandle);
    static void TriggerVibration(int64_t inputHandle, int leftSpeed, int rightSpeed);

    // --- Parties ---
    static int GetNumActiveBeacons();
    static uint64_t GetBeaconByIndex(int index);
    static void CreateBeacon(int openSlots, const Ref<LobbyData> &lobby, const String &connectString, const String &metadata, const Callable &callback);
    static void DestroyBeacon(int64_t beaconHandle);
    static void ChangeNumOpenSlots(int64_t beaconHandle, int openSlots, const Callable &callback);

    // --- UGC / Workshop ---
    static void UgcCreateItem(int appId, int fileType, const Callable &callback);
    static int64_t UgcStartItemUpdate(int appId, int64_t publishedFileId);
    static bool UgcSetItemTitle(int64_t updateHandle, const String &title);
    static bool UgcSetItemDescription(int64_t updateHandle, const String &description);
    static bool UgcSetItemContent(int64_t updateHandle, const String &contentFolder);
    static bool UgcSetItemPreview(int64_t updateHandle, const String &previewFile);
    static bool UgcSetItemVisibility(int64_t updateHandle, int visibility);
    static bool UgcAddItemTag(int64_t updateHandle, const String &tag);
    static void UgcSubmitItemUpdate(int64_t updateHandle, const String &changeNotes, const Callable &callback);
    static void UgcSubscribeItem(int64_t publishedFileId, const Callable &callback);
    static void UgcUnsubscribeItem(int64_t publishedFileId, const Callable &callback);
    static uint32_t UgcGetNumSubscribedItems();
    static PackedInt64Array UgcGetSubscribedItems();
    static bool UgcDownloadItem(int64_t publishedFileId, bool highPriority);
    static Array UgcGetItemInstallInfo(int64_t publishedFileId);
    static Array UgcGetItemDownloadInfo(int64_t publishedFileId);

    // --- Inventory ---
    static void GetAllItems(const Callable &callback);
    static void GetItemsByDefinition(int definitionId, const Callable &callback);
    static void AddPromoItem(int definitionId);
    static void AddPromoItems(const Array &definitionIds);
    static void ConsumeItem(int itemId, int quantity);
    static void TriggerItemDrop(int definitionId);
    static void ExchangeItems(const Array &genDefIds, const Array &genQty, const Array &destItemIds, const Array &destQty);
    static void RequestPrices(const Callable &callback);

    // --- Utilities ---
    static uint32_t IpStringToUint(const String &ip);
    static String IpUintToString(const uint32_t ip);

    void _enter_tree() override;
    void _exit_tree() override;
    void _ready() override;

    /// Steam's per-frame callback pump — was Node::_process(double), driven
    /// by set_process(true)/false toggled around readiness. Now called
    /// directly by SteamworksSubsystem::tick() (see SubsystemTicker in
    /// Godot-Game-Framework) — one tick mechanism for the whole framework
    /// instead of SteamApi self-ticking in parallel with everything else.
    /// No-op if !IsReady, same guard the old _process() body had.
    void pump_callbacks(double delta);

protected:
    static void _bind_methods();
    void _init();

private:
    bool SelfInitialised;
    bool IsReady;

    static void LoadAvatar(CSteamID steam_id);

    STEAM_CALLBACK(SteamApi, EventUserStatsReceived, UserStatsReceived_t);
    STEAM_CALLBACK(SteamApi, EventUserStatsUnloaded, UserStatsUnloaded_t);
    STEAM_CALLBACK(SteamApi, EventUserStatsStored, UserStatsStored_t);
    STEAM_CALLBACK(SteamApi, EventUserAchievementStored, UserAchievementStored_t);
    STEAM_CALLBACK(SteamApi, EventDlcInstalled, DlcInstalled_t);
    STEAM_CALLBACK(SteamApi, EventSteamServerConnectFailure, SteamServerConnectFailure_t);
    STEAM_CALLBACK(SteamApi, EventSteamServersConnected, SteamServersConnected_t);
    STEAM_CALLBACK(SteamApi, EventSteamServersDisconnected, SteamServersDisconnected_t);
    STEAM_CALLBACK(SteamApi, EventFriendRichPresenceUpdate, FriendRichPresenceUpdate_t);
    STEAM_CALLBACK(SteamApi, EventPersonaStateChange, PersonaStateChange_t);
    STEAM_CALLBACK(SteamApi, EventAvatarImageLoaded, AvatarImageLoaded_t);
    STEAM_CALLBACK(SteamApi, EventGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t);
    STEAM_CALLBACK(SteamApi, EventGameLobbyJoinRequested, GameLobbyJoinRequested_t);
    STEAM_CALLBACK(SteamApi, EventGameOverlayActivated, GameOverlayActivated_t);
    STEAM_CALLBACK(SteamApi, EventSteamInventoryResultReady, SteamInventoryResultReady_t);
    STEAM_CALLBACK(SteamApi, EventSteamInventoryDefinitionUpdate, SteamInventoryDefinitionUpdate_t);
    STEAM_CALLBACK(SteamApi, EventMicroTxnAuthorizationResponse, MicroTxnAuthorizationResponse_t);
    STEAM_CALLBACK(SteamApi, EventFavoritesListAccountsUpdated, FavoritesListAccountsUpdated_t);
    STEAM_CALLBACK(SteamApi, EventFavoritesListChanged, FavoritesListChanged_t);
    STEAM_CALLBACK(SteamApi, EventLobbyChatMsg, LobbyChatMsg_t);
    STEAM_CALLBACK(SteamApi, EventLobbyChatUpdate, LobbyChatUpdate_t);
    STEAM_CALLBACK(SteamApi, EventLobbyDataUpdate, LobbyDataUpdate_t);
    STEAM_CALLBACK(SteamApi, EventLobbyGameCreated, LobbyGameCreated_t);
    STEAM_CALLBACK(SteamApi, EventLobbyInvite, LobbyInvite_t);
    STEAM_CALLBACK(SteamApi, EventRemotePlaySessionConnected, SteamRemotePlaySessionConnected_t);
    STEAM_CALLBACK(SteamApi, EventRemotePlaySessionDisconnected, SteamRemotePlaySessionDisconnected_t);
    STEAM_CALLBACK(SteamApi, EventRemotePlaySessionGuestInvite, SteamRemotePlayTogetherGuestInvite_t);
    STEAM_CALLBACK(SteamApi, EventValidateAuthTicket, ValidateAuthTicketResponse_t);

#if STEAM_MAJOR > 1 || (STEAM_MAJOR == 1 && STEAM_MINOR >= 57)
    STEAM_CALLBACK(SteamApi, EventGetTicketForWebApiResponse, GetTicketForWebApiResponse_t);
#endif

    STEAM_GAMESERVER_CALLBACK(SteamApi, EventSteamGameServerConnectFailure, SteamServerConnectFailure_t);
    STEAM_GAMESERVER_CALLBACK(SteamApi, EventSteamGameServersConnected, SteamServersConnected_t);
    STEAM_GAMESERVER_CALLBACK(SteamApi, EventSteamGameServersDisconnected, SteamServersDisconnected_t);

    CCallResult<SteamApi, LobbyCreated_t> m_LobbyCreate_t;
    Callable m_LobbyCreated_callback;
    LobbyUseHint::Hint m_LobbyCreatedHint;
    void OnLobbyCreated(LobbyCreated_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LobbyEnter_t> m_LobbyEnter_t;
    Callable m_LobbyEnter_callback;
    void OnLobbyEnter(LobbyEnter_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LobbyMatchList_t> m_LobbyMatchList_t;
    Callable m_LobbyMatchList_callback;
    void OnLobbyMatchList(LobbyMatchList_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LeaderboardFindResult_t> m_LeaderboardFindResult_t;
    LeaderboardFindRequest m_LeaderboardPending;
    void OnLeaderboardFindResult(LeaderboardFindResult_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LeaderboardScoresDownloaded_t> m_LeaderboardScoresDownloaded_t;
    LeaderboardDownloadRequest m_LeaderboardScoresPending;
    void OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LeaderboardScoreUploaded_t> m_LeaderboardScoreUploaded_t;
    Callable m_LeaderboardScoreUploaded_callback;
    void OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, NumberOfCurrentPlayers_t> m_NumberOfCurrentPlayers_t;
    Callable m_NumberOfCurrentPlayers_callback;
    void OnNumberOfCurrentPlayers(NumberOfCurrentPlayers_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, GlobalAchievementPercentagesReady_t> m_GlobalAchievementPercentagesReady_t;
    Callable m_GlobalAchievementPercentagesReady_callback;
    void OnGlobalAchievementPercentagesReady(GlobalAchievementPercentagesReady_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, GlobalStatsReceived_t> m_GlobalStatsReceived_t;
    Callable m_GlobalStatsReceived_callback;
    void OnGlobalStatsReceived(GlobalStatsReceived_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, RemoteStorageFileShareResult_t> m_RemoteStorageFileShareResult_t;
    String m_RemoteStorageFileShareResult_leaderboard;
    Callable m_RemoteStorageFileShareResult_callback;
    void OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, LeaderboardUGCSet_t> m_LeaderboardUgcSet_t;
    Callable m_LeaderboardUgcSet_callback;
    void OnLeaderboardUgcSet(LeaderboardUGCSet_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, CreateBeaconCallback_t> m_CreateBeacon_t;
    Callable m_CreateBeacon_callback;
    void OnCreateBeacon(CreateBeaconCallback_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, ChangeNumOpenSlotsCallback_t> m_ChangeNumOpenSlots_t;
    Callable m_ChangeNumOpenSlots_callback;
    void OnChangeNumOpenSlots(ChangeNumOpenSlotsCallback_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, CreateItemResult_t> m_UgcCreateItem_t;
    Callable m_UgcCreateItem_callback;
    void OnUgcCreateItem(CreateItemResult_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, SubmitItemUpdateResult_t> m_UgcSubmitItemUpdate_t;
    Callable m_UgcSubmitItemUpdate_callback;
    void OnUgcSubmitItemUpdate(SubmitItemUpdateResult_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, RemoteStorageSubscribePublishedFileResult_t> m_UgcSubscribeItem_t;
    Callable m_UgcSubscribeItem_callback;
    void OnUgcSubscribeItem(RemoteStorageSubscribePublishedFileResult_t *pResult, bool bIOFailure);

    CCallResult<SteamApi, RemoteStorageUnsubscribePublishedFileResult_t> m_UgcUnsubscribeItem_t;
    Callable m_UgcUnsubscribeItem_callback;
    void OnUgcUnsubscribeItem(RemoteStorageUnsubscribePublishedFileResult_t *pResult, bool bIOFailure);

    HashMap<int, Callable> InventoryResultCallbacks;
    HashMap<int, int> InventoryResultFilters;
    CCallResult<SteamApi, SteamInventoryRequestPricesResult_t> m_InventoryRequestPrices_t;
    Callable m_InventoryRequestPrices_callback;
    void OnInventoryRequestPricesResult(SteamInventoryRequestPricesResult_t *pResult, bool bIOFailure);
};
