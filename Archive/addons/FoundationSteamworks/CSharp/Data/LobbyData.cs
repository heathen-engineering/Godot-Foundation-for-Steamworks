using Godot;

namespace Heathen.SteamworksIntegration
{
    public class LobbyData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public LobbyData(GodotObject instance) => _instance = instance;

        public ulong Id => (ulong)_instance.Get("Id");

        public string HexId => (string)_instance.Get("HexId");

        public static LobbyData FromHex(string hexId)
        {
            Variant result = ClassDB.ClassCallStatic("LobbyData", "FromHex", hexId);
            return result.Obj is GodotObject obj ? new LobbyData(obj) : null;
        }
    }
}
