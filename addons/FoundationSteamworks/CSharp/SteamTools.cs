using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public static class SteamTools
    {
        static GodotObject Instance => Engine.GetSingleton("SteamApi");

        // --- Initialisation ---

        public static SteamInitialisationResponse InitialiseClient()
        {
            Variant result = Instance.Call("InitialiseClient");
            if (result.VariantType != Variant.Type.Object || result.Obj == null)
                return null;
            return new SteamInitialisationResponse((GodotObject)result.Obj);
        }

        public static SteamInitialisationResponse InitialiseServer()
        {
            Variant result = Instance.Call("InitialiseServer");
            if (result.VariantType != Variant.Type.Object || result.Obj == null)
                return null;
            return new SteamInitialisationResponse((GodotObject)result.Obj);
        }

        public static bool IsReady => (bool)Instance.Call("GetIsReady");

        public static int AppId => (int)Instance.Get("appId");

        // --- App ---

        public static int AppBuildId => (int)Instance.Call("GetAppBuildId");

        public static string GetAppInstallDir(int appId) => (string)Instance.Call("GetAppInstallDir", appId);

        public static UserData AppOwner
        {
            get
            {
                Variant result = Instance.Call("GetAppOwner");
                if (result.VariantType != Variant.Type.Object || result.Obj == null)
                    return null;
                return new UserData((GodotObject)result.Obj);
            }
        }

        public static string AvailableGameLanguages => (string)Instance.Call("GetAvailableGameLanguages");

        public static string CurrentBetaName => (string)Instance.Call("GetCurrentBetaName");

        public static string CurrentGameLanguage => (string)Instance.Call("GetCurrentGameLanguage");

        public static int DlcCount => (int)Instance.Call("GetDLCCount");

        public static List<DlcData> GetDlc()
        {
            Variant result = Instance.Call("GetDLC");
            if (result.VariantType != Variant.Type.Array)
                return new List<DlcData>();
            var array = (Godot.Collections.Array)result;
            var list = new List<DlcData>();
            foreach (Variant item in array)
                if (item.Obj is GodotObject obj)
                    list.Add(new DlcData(obj));
            return list;
        }

        public static DateTime GetEarliestPurchaseTime(int appId)
        {
            int nixTime = (int)Instance.Call("GetEarliestPurchaseUnixTime", appId);
            return DateTimeOffset.FromUnixTimeSeconds(nixTime).UtcDateTime;
        }

        public static string LaunchCommandLine => (string)Instance.Call("GetLaunchCommandLine");

        public static string GetLaunchQueryParam(string key) => (string)Instance.Call("GetLaunchQueryParam", key);

        public static bool IsAppInstalled(int appId) => (bool)Instance.Call("IsAppInstalled", appId);

        public static bool IsCybercafe => (bool)Instance.Call("IsCybercafe");

        public static bool IsLowViolence => (bool)Instance.Call("IsLowViolence");

        public static bool IsSubscribed => (bool)Instance.Call("IsSubscribed");

        public static bool IsSubscribedApp(int appId) => (bool)Instance.Call("IsSubscribedApp", appId);

        public static bool IsSubscribedFromFamilySharing => (bool)Instance.Call("IsSubscribedFromFamilySharing");

        public static bool IsSubscribedFromFreeWeekend => (bool)Instance.Call("IsSubscribedFromFreeWeekend");

        public static SteamTimedTrial IsTimedTrial
        {
            get
            {
                Variant result = Instance.Call("IsTimedTrial");
                if (result.VariantType != Variant.Type.Object || result.Obj == null)
                    return default;
                return new SteamTimedTrial((GodotObject)result.Obj);
            }
        }

        public static bool IsVACBanned => (bool)Instance.Call("IsVACBanned");

        public static bool MarkContentCorrupt(bool missingFilesOnly) => (bool)Instance.Call("MarkContentCorrupt", missingFilesOnly);

        // --- Overlay ---

        public static void ActivateGameOverlay(string type) => Instance.Call("ActivateGameOverlay", type);

        public static void ActivateGameOverlayInviteDialogConnectString(string connectString) => Instance.Call("ActivateGameOverlayInviteDialogConnectString", connectString);

        public static void ActivateGameOverlayToStore(int appId) => Instance.Call("ActivateGameOverlayToStore", appId);

        public static void ActivateGameOverlayToWebPage(string url, bool modal) => Instance.Call("ActivateGameOverlayToWebPage", url, modal);

        // --- Leaderboards ---

        public static void DownloadLeaderboardGlobalEntries(string leaderboard, int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            Callable callable = Callable.From((Variant entriesVar, bool ioError) =>
            {
                callback?.Invoke(ExtractLeaderboardEntries(entriesVar), ioError);
            });
            Instance.Call("DownloadLeaderboardGlobalEntries", start, end, detailCount, leaderboard, callable);
        }

        public static void DownloadLeaderboardAroundUserEntries(string leaderboard, int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            Callable callable = Callable.From((Variant entriesVar, bool ioError) =>
            {
                callback?.Invoke(ExtractLeaderboardEntries(entriesVar), ioError);
            });
            Instance.Call("DownloadLeaderboardAroundUserEntries", start, end, detailCount, leaderboard, callable);
        }

        public static void DownloadLeaderboardFriendsEntries(string leaderboard, int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            Callable callable = Callable.From((Variant entriesVar, bool ioError) =>
            {
                callback?.Invoke(ExtractLeaderboardEntries(entriesVar), ioError);
            });
            Instance.Call("DownloadLeaderboardFriendsEntries", start, end, detailCount, leaderboard, callable);
        }

        public static void FindLeaderboard(string leaderboard) => Instance.Call("FindLeaderboard", leaderboard);

        public static void FindOrCreateLeaderboard(string leaderboard, bool lowestScoreIsTopRank, int displayType) =>
            Instance.Call("FindOrCreateLeaderboard", leaderboard, lowestScoreIsTopRank, displayType);

        public static ulong GetLeaderboardNativeId(string leaderboard) => (ulong)Instance.Call("GetLeaderboardNativeId", leaderboard);

        public static int GetLeaderboardEntryCount(string leaderboard) => (int)Instance.Call("GetLeaderboardEntryCount", leaderboard);

        public static string GetLeaderboardName(string leaderboard) => (string)Instance.Call("GetLeaderboardName", leaderboard);

        public static bool IsLeaderboardTopRankLowestScore(string leaderboard) => (bool)Instance.Call("IsLeaderboardTopRankLowestScore", leaderboard);

        public static void UploadLeaderboardScore(string leaderboard, int score, Action<LeaderboardEntryData, bool> callback)
        {
            Callable callable = Callable.From((Variant entryVar, bool ioError) =>
            {
                LeaderboardEntryData entry = entryVar.Obj is GodotObject obj ? new LeaderboardEntryData(obj) : null;
                callback?.Invoke(entry, ioError);
            });
            Instance.Call("UploadLeaderboardScore", leaderboard, score, callable);
        }

        public static void UploadLeaderboardScoreWithDetails(string leaderboard, int score, int[] details, Action<LeaderboardEntryData, bool> callback)
        {
            Callable callable = Callable.From((Variant entryVar, bool ioError) =>
            {
                LeaderboardEntryData entry = entryVar.Obj is GodotObject obj ? new LeaderboardEntryData(obj) : null;
                callback?.Invoke(entry, ioError);
            });
            Instance.Call("UploadLeaderboardScoreWithDetails", leaderboard, score, new PackedInt32Array(details), callable);
        }

        private static List<LeaderboardEntryData> ExtractLeaderboardEntries(Variant entriesVar)
        {
            var list = new List<LeaderboardEntryData>();
            if (entriesVar.VariantType != Variant.Type.Array)
                return list;
            var array = (Godot.Collections.Array)entriesVar;
            foreach (Variant item in array)
                if (item.Obj is GodotObject obj)
                    list.Add(new LeaderboardEntryData(obj));
            return list;
        }

        // --- Stats & Achievements ---

        public static void GetNumberOfCurrentPlayers(Action<int, bool> callback)
        {
            Callable callable = Callable.From((int count, bool ioError) => callback?.Invoke(count, ioError));
            Instance.Call("GetNumberOfCurrentPlayers", callable);
        }

        public static void RequestUserStats(UserData user) => Instance.Call("RequestUserRefStats", user.ToGDNative());

        public static void RequestUserStats(ulong steamId) => Instance.Call("RequestUserIdStats", steamId);

        public static bool ResetAllStats(bool achievementsToo = true) => (bool)Instance.Call("ResetAllStats", achievementsToo);

        public static bool StoreStats() => (bool)Instance.Call("StoreStats");

        public static bool SetAchievement(string achievement) => (bool)Instance.Call("SetAchievement", achievement);

        public static bool IsAchievementUnlocked(string achievement) => (bool)Instance.Call("IsAchievementUnlocked", achievement);

        public static bool IsUserAchievementUnlocked(UserData user, string achievement) =>
            (bool)Instance.Call("IsUserAchievementUnlocked", user.ToGDNative(), achievement);

        public static DateTime GetAchievementUnlockTime(string achievement)
        {
            int nixTime = (int)Instance.Call("GetAchievementUnlockTime", achievement);
            return nixTime > 0 ? DateTimeOffset.FromUnixTimeSeconds(nixTime).UtcDateTime : DateTime.MinValue;
        }

        public static DateTime GetUserAchievementUnlockTime(UserData user, string achievement)
        {
            int nixTime = (int)Instance.Call("GetUserAchievementUnlockTime", user.ToGDNative(), achievement);
            return nixTime > 0 ? DateTimeOffset.FromUnixTimeSeconds(nixTime).UtcDateTime : DateTime.MinValue;
        }

        public static float GetAchievementAchievedPercent(string achievement) => (float)Instance.Call("GetAchievementAchievedPercent", achievement);

        public static string GetAchievementDisplayName(string achievement) => (string)Instance.Call("GetAchievementName", achievement);

        public static string GetAchievementDescription(string achievement) => (string)Instance.Call("GetAchievementDescription", achievement);

        public static bool GetAchievementIsHidden(string achievement) => (bool)Instance.Call("GetAchievementIsHidden", achievement);

        public static void GetAchievementIcon(string achievement, Action<SteamResult, Texture2D> callback)
        {
            Callable callable = Callable.From((int result, Texture2D tex) => callback?.Invoke((SteamResult)result, tex));
            Instance.Call("GetAchievementIcon", achievement, callable);
        }

        public static List<string> GetAchievements()
        {
            Variant result = Instance.Call("GetAchievements");
            var list = new List<string>();
            if (result.VariantType == Variant.Type.PackedStringArray)
                foreach (string s in result.As<string[]>())
                    list.Add(s);
            return list;
        }

        public static bool IndicateAchievementProgress(string achievement, int current, int max) =>
            (bool)Instance.Call("IndicateAchievementProgress", achievement, current, max);

        public static bool ClearAchievement(string achievement) => (bool)Instance.Call("ClearAchievement", achievement);

        public static bool SetStatFloat(string stat, float value) => (bool)Instance.Call("SetStatFloat", stat, value);

        public static bool SetStatInt(string stat, int value) => (bool)Instance.Call("SetStatInt", stat, value);

        public static bool UpdateAvgRateStat(string stat, float thisSession, double sessionLength) =>
            (bool)Instance.Call("UpdateAvgRateStat", stat, thisSession, sessionLength);

        public static void RequestGlobalAchievementPercentages(Action<bool> callback)
        {
            Callable callable = Callable.From((bool ioError) => callback?.Invoke(ioError));
            Instance.Call("RequestGlobalAchievementPercentages", callable);
        }

        public static void RequestGlobalStats(int historyDays, Action<bool> callback)
        {
            Callable callable = Callable.From((bool ioError) => callback?.Invoke(ioError));
            Instance.Call("RequestGlobalStats", historyDays, callable);
        }

        public static ulong GetGlobalStatInt(string stat) => (ulong)Instance.Call("GetGlobalStatInt", stat);

        public static double GetGlobalStatFloat(string stat) => (double)Instance.Call("GetGlobalStatFloat", stat);

        public static float GetStatFloat(string stat) => (float)Instance.Call("GetStatFloat", stat);

        public static int GetStatInt(string stat) => (int)Instance.Call("GetStatInt", stat);

        public static float GetUserStatFloat(UserData user, string stat) => (float)Instance.Call("GetUserStatFloat", user.ToGDNative(), stat);

        public static int GetUserStatInt(UserData user, string stat) => (int)Instance.Call("GetUserStatInt", user.ToGDNative(), stat);

        // --- Friends / User ---

        public static void GetFriendAvatar(UserData user, Action<Texture2D> callback)
        {
            Callable callable = Callable.From((Texture2D tex) => callback?.Invoke(tex));
            Instance.Call("GetFriendAvatar", user.ToGDNative(), callable);
        }

        // --- Matchmaking ---

        public static void LeaveLobby(LobbyData lobby) => Instance.Call("LeaveLobby", lobby.ToGDNative());

        public static void CreateLobby(LobbyType type, LobbyUseHint hint, int maxMembers, Action<LobbyData, SteamResult, bool> callback)
        {
            Callable callable = Callable.From((Variant lobbyVar, int result, bool ioError) =>
            {
                LobbyData lobby = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                callback?.Invoke(lobby, (SteamResult)result, ioError);
            });
            Instance.Call("CreateLobby", (int)type, (int)hint, maxMembers, callable);
        }

        public static void JoinLobby(LobbyData lobby, Action<LobbyData, LobbyEnterResponse, bool> callback)
        {
            Callable callable = Callable.From((Variant lobbyVar, int response, bool ioError) =>
            {
                LobbyData joined = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                callback?.Invoke(joined, (LobbyEnterResponse)response, ioError);
            });
            Instance.Call("JoinLobby", lobby.ToGDNative(), callable);
        }

        public static void JoinLobbyById(ulong lobbyId, Action<LobbyData, LobbyEnterResponse, bool> callback)
        {
            Callable callable = Callable.From((Variant lobbyVar, int response, bool ioError) =>
            {
                LobbyData joined = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                callback?.Invoke(joined, (LobbyEnterResponse)response, ioError);
            });
            Instance.Call("JoinLobbyById", (long)lobbyId, callable);
        }

        public static void JoinLobbyByHex(string hexId, Action<LobbyData, LobbyEnterResponse, bool> callback)
        {
            Callable callable = Callable.From((Variant lobbyVar, int response, bool ioError) =>
            {
                LobbyData joined = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                callback?.Invoke(joined, (LobbyEnterResponse)response, ioError);
            });
            Instance.Call("JoinLobbyByHex", hexId, callable);
        }

        // --- Utilities ---

        public static uint IpStringToUint(string ip) => (uint)Instance.Call("IpStringToUint", ip);

        public static string IpUintToString(uint ip) => (string)Instance.Call("IpUintToString", ip);
    }
}
