using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class LeaderboardData
    {
        private GodotObject _instance;
        private static readonly Dictionary<string, LeaderboardData> _cache = new();

        public GodotObject ToGDNative() => _instance;

        public LeaderboardData(GodotObject instance) => _instance = instance;

        public string LeaderboardName => (string)_instance.Get("LeaderboardName");

        public int EntryCount => (int)_instance.Get("EntryCount");

        public ulong NativeId => (ulong)_instance.Call("GetNativeId");

        public bool IsTopRankLowestScore => (bool)_instance.Call("GetIsTopRankLowestScore");

        public int DisplayType => (int)_instance.Call("GetDisplayType");

        /// <summary>True once the leaderboard handle has been resolved by Steam (i.e. FindOrCreate has completed).</summary>
        public bool IsReady => NativeId != 0;

        // --- Download ---

        public void DownloadGlobalEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadGlobalEntries", start, end, detailCount, BuildEntriesCallback(callback));
        }

        public void DownloadAroundUserEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadAroundUserEntries", start, end, detailCount, BuildEntriesCallback(callback));
        }

        public void DownloadFriendsEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadFriendsEntries", start, end, detailCount, BuildEntriesCallback(callback));
        }

        public void DownloadForUsers(List<UserData> users, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            var arr = new Godot.Collections.Array();
            foreach (var user in users)
                arr.Add(user.ToGDNative());
            _instance.Call("DownloadEntriesForUsers", arr, detailCount, BuildEntriesCallback(callback));
        }

        // --- Upload ---

        public void UploadScore(int score, Action<LeaderboardEntryData, bool> callback)
        {
            Callable callable = Callable.From((Variant entryVar, bool ioError) =>
            {
                LeaderboardEntryData entry = entryVar.Obj is GodotObject obj ? new LeaderboardEntryData(obj) : null;
                callback?.Invoke(entry, ioError);
            });
            Engine.GetSingleton("SteamApi").Call("UploadLeaderboardScore", LeaderboardName, score, callable);
        }

        public void UploadScore(int score, int[] details, Action<LeaderboardEntryData, bool> callback)
        {
            Callable callable = Callable.From((Variant entryVar, bool ioError) =>
            {
                LeaderboardEntryData entry = entryVar.Obj is GodotObject obj ? new LeaderboardEntryData(obj) : null;
                callback?.Invoke(entry, ioError);
            });
            Engine.GetSingleton("SteamApi").Call("UploadLeaderboardScoreWithDetails", LeaderboardName, score, details, callable);
        }

        // --- Static ---

        /// <summary>
        /// Returns a LeaderboardData for the given name. Boards pre-configured on the SteamApi autoload are
        /// resolved at startup and returned directly from cache; others create a wrapper that becomes functional
        /// once FindOrCreate completes.
        /// </summary>
        public static LeaderboardData Get(string leaderboardName)
        {
            if (_cache.TryGetValue(leaderboardName, out var cached))
                return cached;
            Variant result = ClassDB.ClassCallStatic("LeaderboardData", "Get", leaderboardName);
            return result.Obj is GodotObject obj ? new LeaderboardData(obj) : null;
        }

        /// <summary>Kicks off an async FindOrCreate; the board is cached and ready once OnLeaderboardFound fires.</summary>
        public static LeaderboardData FindOrCreate(string leaderboardName, bool lowestScoreIsTopRank = false, int displayType = 1)
        {
            Variant result = ClassDB.ClassCallStatic("LeaderboardData", "FindOrCreate", leaderboardName, lowestScoreIsTopRank, displayType);
            if (result.Obj is GodotObject obj)
            {
                var board = new LeaderboardData(obj);
                _cache[leaderboardName] = board;
                return board;
            }
            return null;
        }

        /// <summary>Called by SteamToolsInterface during initialisation to pre-populate the cache from leaderboardIds.</summary>
        internal static void CacheFromName(string leaderboardName)
        {
            if (_cache.ContainsKey(leaderboardName))
                return;
            Variant result = ClassDB.ClassCallStatic("LeaderboardData", "Get", leaderboardName);
            if (result.Obj is GodotObject obj)
                _cache[leaderboardName] = new LeaderboardData(obj);
        }

        // --- Helpers ---

        private static Callable BuildEntriesCallback(Action<List<LeaderboardEntryData>, bool> callback)
        {
            return Callable.From((Variant entriesVar, bool ioError) =>
            {
                var list = new List<LeaderboardEntryData>();
                if (entriesVar.VariantType == Variant.Type.Array)
                    foreach (Variant item in (Godot.Collections.Array)entriesVar)
                        if (item.Obj is GodotObject obj)
                            list.Add(new LeaderboardEntryData(obj));
                callback?.Invoke(list, ioError);
            });
        }
    }
}
