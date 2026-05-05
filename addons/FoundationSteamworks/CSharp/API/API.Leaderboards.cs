using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration.API
{
    public static class Leaderboards
    {
        public static void DownloadGlobalEntries(string leaderboard, int start, int end, int detailCount,
            Action<List<LeaderboardEntryData>, bool> callback) =>
            SteamTools.DownloadLeaderboardGlobalEntries(leaderboard, start, end, detailCount, callback);

        public static void DownloadAroundUserEntries(string leaderboard, int start, int end, int detailCount,
            Action<List<LeaderboardEntryData>, bool> callback) =>
            SteamTools.DownloadLeaderboardAroundUserEntries(leaderboard, start, end, detailCount, callback);

        public static void DownloadFriendsEntries(string leaderboard, int start, int end, int detailCount,
            Action<List<LeaderboardEntryData>, bool> callback) =>
            SteamTools.DownloadLeaderboardFriendsEntries(leaderboard, start, end, detailCount, callback);

        public static void Find(string leaderboard) => SteamTools.FindLeaderboard(leaderboard);

        /// <summary>Kicks off async Find or Create; listen for SteamToolsEvents.OnLeaderboardFound for completion.</summary>
        public static void FindOrCreate(string leaderboard, bool lowestScoreIsTopRank = false, int displayType = 1) =>
            SteamTools.FindOrCreateLeaderboard(leaderboard, lowestScoreIsTopRank, displayType);

        public static ulong GetNativeId(string leaderboard) => SteamTools.GetLeaderboardNativeId(leaderboard);

        public static int GetEntryCount(string leaderboard) => SteamTools.GetLeaderboardEntryCount(leaderboard);

        public static string GetName(string leaderboard) => SteamTools.GetLeaderboardName(leaderboard);

        public static bool IsTopRankLowestScore(string leaderboard) =>
            SteamTools.IsLeaderboardTopRankLowestScore(leaderboard);
    }
}
