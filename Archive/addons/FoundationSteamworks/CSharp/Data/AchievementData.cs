using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class AchievementData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public AchievementData(GodotObject instance) => _instance = instance;

        public string ApiName => (string)_instance.Get("ApiName");

        public string DisplayName => (string)_instance.Get("Name");

        public string Description => (string)_instance.Get("Description");

        public bool IsHidden => (bool)_instance.Get("IsHidden");

        public bool IsAchieved
        {
            get => (bool)_instance.Get("IsAchieved");
            set => _instance.Set("IsAchieved", value);
        }

        public DateTime UnlockTime
        {
            get
            {
                int nixTime = (int)_instance.Get("UnlockTime");
                return nixTime > 0 ? DateTimeOffset.FromUnixTimeSeconds(nixTime).UtcDateTime : DateTime.MinValue;
            }
        }

        public float GlobalPercent => (float)_instance.Get("GlobalPercent");

        public void GetIcon(Action<Texture2D> callback)
        {
            Callable callable = Callable.From((Texture2D tex) => callback?.Invoke(tex));
            _instance.Call("GetIcon", callable);
        }

        public void Unlock() => _instance.Call("Unlock");

        public void Clear() => _instance.Call("Clear");

        public bool Store() => (bool)_instance.Call("Store");

        public static AchievementData Get(string apiName)
        {
            Variant result = ClassDB.ClassCallStatic("AchievementData", "Get", apiName);
            return result.Obj is GodotObject obj ? new AchievementData(obj) : null;
        }

        public static List<AchievementData> GetAll()
        {
            Variant result = ClassDB.ClassCallStatic("AchievementData", "GetAll");
            var list = new List<AchievementData>();
            if (result.VariantType != Variant.Type.Array)
                return list;
            foreach (Variant item in (Godot.Collections.Array)result)
                if (item.Obj is GodotObject obj)
                    list.Add(new AchievementData(obj));
            return list;
        }
    }
}
