using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration.API
{
    public static class StatsAndAchievements
    {
        // --- Achievements ---

        public static bool Unlock(string achievement) => SteamTools.SetAchievement(achievement);

        public static bool Clear(string achievement) => SteamTools.ClearAchievement(achievement);

        public static bool IsUnlocked(string achievement) => SteamTools.IsAchievementUnlocked(achievement);

        public static bool IsUnlocked(UserData user, string achievement) =>
            SteamTools.IsUserAchievementUnlocked(user, achievement);

        public static DateTime GetUnlockTime(string achievement) =>
            SteamTools.GetAchievementUnlockTime(achievement);

        public static DateTime GetUnlockTime(UserData user, string achievement) =>
            SteamTools.GetUserAchievementUnlockTime(user, achievement);

        public static float GetGlobalPercent(string achievement) =>
            SteamTools.GetAchievementAchievedPercent(achievement);

        public static string GetDisplayName(string achievement) =>
            SteamTools.GetAchievementDisplayName(achievement);

        public static string GetDescription(string achievement) =>
            SteamTools.GetAchievementDescription(achievement);

        public static bool GetIsHidden(string achievement) =>
            SteamTools.GetAchievementIsHidden(achievement);

        public static List<string> GetAchievements() => SteamTools.GetAchievements();

        public static bool IndicateProgress(string achievement, int current, int max) =>
            SteamTools.IndicateAchievementProgress(achievement, current, max);

        public static void RequestGlobalPercentages(Action<bool> callback) =>
            SteamTools.RequestGlobalAchievementPercentages(callback);

        // --- Stats ---

        public static bool SetStat(string stat, int value) => SteamTools.SetStatInt(stat, value);

        public static bool SetStat(string stat, float value) => SteamTools.SetStatFloat(stat, value);

        public static int GetStatInt(string stat) => SteamTools.GetStatInt(stat);

        public static float GetStatFloat(string stat) => SteamTools.GetStatFloat(stat);

        public static int GetUserStatInt(UserData user, string stat) => SteamTools.GetUserStatInt(user, stat);

        public static float GetUserStatFloat(UserData user, string stat) => SteamTools.GetUserStatFloat(user, stat);

        public static ulong GetGlobalStatInt(string stat) => SteamTools.GetGlobalStatInt(stat);

        public static double GetGlobalStatFloat(string stat) => SteamTools.GetGlobalStatFloat(stat);

        public static bool UpdateAvgRateStat(string stat, float thisSession, double sessionLength) =>
            SteamTools.UpdateAvgRateStat(stat, thisSession, sessionLength);

        public static void RequestGlobalStats(int historyDays, Action<bool> callback) =>
            SteamTools.RequestGlobalStats(historyDays, callback);

        public static bool StoreStats() => SteamTools.StoreStats();

        public static bool ResetAllStats(bool achievementsToo = true) => SteamTools.ResetAllStats(achievementsToo);

        public static void GetNumberOfCurrentPlayers(Action<int, bool> callback) =>
            SteamTools.GetNumberOfCurrentPlayers(callback);

        /// <summary>Mirrors Unity Foundation's API.StatsAndAchievements.Client nesting for call-site parity.</summary>
        public static class Client
        {
            public static bool Unlock(string achievement) => StatsAndAchievements.Unlock(achievement);
            public static bool Clear(string achievement) => StatsAndAchievements.Clear(achievement);
            public static bool IsUnlocked(string achievement) => StatsAndAchievements.IsUnlocked(achievement);
            public static bool IsUnlocked(UserData user, string achievement) => StatsAndAchievements.IsUnlocked(user, achievement);
            public static DateTime GetUnlockTime(string achievement) => StatsAndAchievements.GetUnlockTime(achievement);
            public static DateTime GetUnlockTime(UserData user, string achievement) => StatsAndAchievements.GetUnlockTime(user, achievement);
            public static float GetGlobalPercent(string achievement) => StatsAndAchievements.GetGlobalPercent(achievement);
            public static string GetDisplayName(string achievement) => StatsAndAchievements.GetDisplayName(achievement);
            public static string GetDescription(string achievement) => StatsAndAchievements.GetDescription(achievement);
            public static bool GetIsHidden(string achievement) => StatsAndAchievements.GetIsHidden(achievement);
            public static List<string> GetAchievements() => StatsAndAchievements.GetAchievements();
            public static bool IndicateProgress(string achievement, int current, int max) => StatsAndAchievements.IndicateProgress(achievement, current, max);
            public static void RequestGlobalPercentages(Action<bool> callback) => StatsAndAchievements.RequestGlobalPercentages(callback);

            public static bool SetStat(string stat, int value) => StatsAndAchievements.SetStat(stat, value);
            public static bool SetStat(string stat, float value) => StatsAndAchievements.SetStat(stat, value);
            public static int GetStatInt(string stat) => StatsAndAchievements.GetStatInt(stat);
            public static float GetStatFloat(string stat) => StatsAndAchievements.GetStatFloat(stat);
            public static int GetUserStatInt(UserData user, string stat) => StatsAndAchievements.GetUserStatInt(user, stat);
            public static float GetUserStatFloat(UserData user, string stat) => StatsAndAchievements.GetUserStatFloat(user, stat);
            public static ulong GetGlobalStatInt(string stat) => StatsAndAchievements.GetGlobalStatInt(stat);
            public static double GetGlobalStatFloat(string stat) => StatsAndAchievements.GetGlobalStatFloat(stat);
            public static bool UpdateAvgRateStat(string stat, float thisSession, double sessionLength) => StatsAndAchievements.UpdateAvgRateStat(stat, thisSession, sessionLength);
            public static void RequestGlobalStats(int historyDays, Action<bool> callback) => StatsAndAchievements.RequestGlobalStats(historyDays, callback);
            public static bool StoreStats() => StatsAndAchievements.StoreStats();
            public static bool ResetAllStats(bool achievementsToo = true) => StatsAndAchievements.ResetAllStats(achievementsToo);
            public static void GetNumberOfCurrentPlayers(Action<int, bool> callback) => StatsAndAchievements.GetNumberOfCurrentPlayers(callback);
        }

        // Server-side (ISteamGameServerStats) is not yet wrapped natively — see README "Not yet ported".
        // A Server nested class will land once SteamApi grows the game-server stat/achievement calls.
    }
}
