using Godot;

namespace Heathen.SteamworksIntegration
{
    public class SteamInventoryItemDetail
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public SteamInventoryItemDetail(GodotObject instance) => _instance = instance;

        public int ItemId => (int)_instance.Get("itemId");

        public int DefId => (int)_instance.Get("definitionId");

        public int Quantity => (int)_instance.Get("quantity");

        public int Flags => (int)_instance.Get("flags");

        public Godot.Collections.Dictionary Properties =>
            (Godot.Collections.Dictionary)_instance.Get("properties");

        public Godot.Collections.Dictionary Tags =>
            (Godot.Collections.Dictionary)_instance.Get("tags");

        public string DynamicProperties => (string)_instance.Get("dynamicProperties");

        public ItemData Definition => ItemData.Get(DefId);
    }
}
