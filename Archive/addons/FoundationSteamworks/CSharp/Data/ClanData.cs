using Godot;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class ClanData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public ClanData(GodotObject instance) => _instance = instance;

        public ulong Id => (ulong)_instance.Get("IntId");

        public string Name => (string)_instance.Get("Name");

        public string Tag => (string)_instance.Get("Tag");

        public int MemberCount => (int)_instance.Get("MemberCount");

        public bool IsPublic => (bool)_instance.Call("IsPublic");

        public bool IsOfficialGameGroup => (bool)_instance.Call("IsOfficialGameGroup");

        public static ClanData Get(ulong clanId)
        {
            Variant result = ClassDB.ClassCallStatic("ClanData", "Get", clanId);
            return result.Obj is GodotObject obj ? new ClanData(obj) : null;
        }

        public static List<ClanData> GetClans()
        {
            Variant result = ClassDB.ClassCallStatic("ClanData", "GetClans");
            var list = new List<ClanData>();
            if (result.VariantType != Variant.Type.Array)
                return list;
            foreach (Variant item in (Godot.Collections.Array)result)
                if (item.Obj is GodotObject obj)
                    list.Add(new ClanData(obj));
            return list;
        }
    }
}
