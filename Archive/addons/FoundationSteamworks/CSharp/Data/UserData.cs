using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class UserData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public UserData(GodotObject instance) => _instance = instance;

        public static UserData Me
        {
            get
            {
                Variant item = ClassDB.ClassCallStatic("UserData", "Me");
                return item.Obj is GodotObject obj ? new UserData(obj) : null;
            }
        }

        public ulong Id => (ulong)_instance.Get("IntId");

        public string HexId => (string)_instance.Get("HexId");

        public string UserName => (string)_instance.Get("UserName");

        public bool IsValid => (bool)_instance.Get("IsValid");

        public int Level => (int)_instance.Get("Level");

        public static UserData FromHex(string hexId)
        {
            Variant result = ClassDB.ClassCallStatic("UserData", "FromHex", hexId);
            return result.Obj is GodotObject obj ? new UserData(obj) : null;
        }

        public void GetAvatar(Action<Texture2D> callback)
        {
            SteamTools.GetFriendAvatar(this, callback);
        }

        public void ActivateOverlay(string type) => _instance.Call("ActivateOverlay", type);

        public static void SetRichPresence(string key, string value) =>
            ClassDB.ClassCallStatic("UserData", "SetRichPresence", key, value);

        public static void ClearRichPresence() =>
            ClassDB.ClassCallStatic("UserData", "ClearRichPresence");

        public static List<UserData> GetFriends()
        {
            Variant result = ClassDB.ClassCallStatic("UserData", "GetFriends");
            return ExtractUserList(result);
        }

        public static List<UserData> GetCoplayFriends()
        {
            Variant result = ClassDB.ClassCallStatic("UserData", "GetCoplayFriends");
            return ExtractUserList(result);
        }

        public static List<UserData> GetFriendsFromSource(ulong sourceId)
        {
            Variant result = ClassDB.ClassCallStatic("UserData", "GetFriendsFromSource", sourceId);
            return ExtractUserList(result);
        }

        private static List<UserData> ExtractUserList(Variant result)
        {
            var list = new List<UserData>();
            if (result.VariantType != Variant.Type.Array)
                return list;
            foreach (Variant item in (Godot.Collections.Array)result)
                if (item.Obj is GodotObject obj)
                    list.Add(new UserData(obj));
            return list;
        }
    }
}
