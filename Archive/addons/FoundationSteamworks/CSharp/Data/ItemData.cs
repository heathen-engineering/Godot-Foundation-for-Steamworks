using Godot;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class ItemData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public ItemData(GodotObject instance) => _instance = instance;

        public int DefId => (int)_instance.Get("DefId");

        public string Name => (string)_instance.Get("Name");

        public string Description => (string)_instance.Get("Description");

        public string Type => (string)_instance.Get("Type");

        public string GetProperty(string name) => (string)_instance.Call("GetProperty", name);

        /// <summary>Current price in the lowest denomination of the user's local currency.</summary>
        public long Price => (long)_instance.Call("GetPrice");

        public bool IsStore => (bool)_instance.Call("GetIsStore");

        public static ItemData Get(int defId)
        {
            Variant result = ClassDB.ClassCallStatic("ItemData", "Get", defId);
            return result.Obj is GodotObject obj ? new ItemData(obj) : null;
        }

        public static List<ItemData> GetAll()
        {
            Variant result = ClassDB.ClassCallStatic("ItemData", "GetAll");
            var list = new List<ItemData>();
            if (result.VariantType != Variant.Type.Array)
                return list;
            foreach (Variant item in (Godot.Collections.Array)result)
                if (item.Obj is GodotObject obj)
                    list.Add(new ItemData(obj));
            return list;
        }
    }
}
