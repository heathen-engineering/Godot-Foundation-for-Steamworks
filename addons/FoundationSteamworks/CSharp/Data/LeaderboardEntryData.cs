using Godot;

namespace Heathen.SteamworksIntegration
{
    public class LeaderboardEntryData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public LeaderboardEntryData(GodotObject instance) => _instance = instance;

        // Inherited from UserData
        public ulong Id => (ulong)_instance.Get("IntId");
        public string HexId => (string)_instance.Get("HexId");
        public string UserName => (string)_instance.Get("UserName");
        public bool IsValid => (bool)_instance.Get("IsValid");

        public UserData User => new UserData(_instance);

        // Leaderboard entry fields
        public int Rank => (int)_instance.Call("GetRank");
        public int Score => (int)_instance.Call("GetScore");
        public ulong UgcHandle => (ulong)_instance.Call("GetUgcHandle");

        public int[] Details
        {
            get
            {
                Variant v = _instance.Call("GetDetails");
                if (v.VariantType == Variant.Type.PackedInt32Array)
                    return (int[])v.As<int[]>();
                return System.Array.Empty<int>();
            }
        }
    }
}
