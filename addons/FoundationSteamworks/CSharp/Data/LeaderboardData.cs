using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class LeaderboardData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public LeaderboardData(GodotObject instance) => _instance = instance;

        public string LeaderboardName => (string)_instance.Get("LeaderboardName");

        public int EntryCount => (int)_instance.Get("EntryCount");

        public ulong NativeId => (ulong)_instance.Call("GetNativeId");

        public bool IsTopRankLowestScore => (bool)_instance.Call("GetIsTopRankLowestScore");

        public int DisplayType => (int)_instance.Call("GetDisplayType");

        public void DownloadGlobalEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadGlobalEntries", start, end, detailCount, BuildCallback(callback));
        }

        public void DownloadAroundUserEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadAroundUserEntries", start, end, detailCount, BuildCallback(callback));
        }

        public void DownloadFriendsEntries(int start, int end, int detailCount, Action<List<LeaderboardEntryData>, bool> callback)
        {
            _instance.Call("DownloadFriendsEntries", start, end, detailCount, BuildCallback(callback));
        }

        private static Callable BuildCallback(Action<List<LeaderboardEntryData>, bool> callback)
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

        /// <summary>Returns a reference immediately; the leaderboard resolves asynchronously via SteamToolsEvents.OnLeaderboardFound.</summary>
        public static LeaderboardData Get(string leaderboardName)
        {
            Variant result = ClassDB.ClassCallStatic("LeaderboardData", "Get", leaderboardName);
            return result.Obj is GodotObject obj ? new LeaderboardData(obj) : null;
        }

        /// <summary>Kicks off a FindOrCreate async op; listen for SteamToolsEvents.OnLeaderboardFound for completion.</summary>
        public static LeaderboardData FindOrCreate(string leaderboardName, bool lowestScoreIsTopRank = false, int displayType = 1)
        {
            Variant result = ClassDB.ClassCallStatic("LeaderboardData", "FindOrCreate", leaderboardName, lowestScoreIsTopRank, displayType);
            return result.Obj is GodotObject obj ? new LeaderboardData(obj) : null;
        }
    }
}
