using Godot;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class StatData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public StatData(GodotObject instance) => _instance = instance;

        public string ApiName => (string)_instance.Get("ApiName");

        public int IntValue
        {
            get => (int)_instance.Call("GetIntValue");
            set => _instance.Call("SetIntValue", value);
        }

        public float FloatValue
        {
            get => (float)_instance.Call("GetFloatValue");
            set => _instance.Call("SetFloatValue", value);
        }

        public int GetUserIntValue(UserData user) => (int)_instance.Call("GetUserIntValue", user.ToGDNative());

        public float GetUserFloatValue(UserData user) => (float)_instance.Call("GetUserFloatValue", user.ToGDNative());

        public bool Store() => (bool)_instance.Call("Store");

        public static StatData Get(string apiName)
        {
            Variant result = ClassDB.ClassCallStatic("StatData", "Get", apiName);
            return result.Obj is GodotObject obj ? new StatData(obj) : null;
        }
    }
}
