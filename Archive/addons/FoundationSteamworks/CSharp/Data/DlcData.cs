using Godot;

namespace Heathen.SteamworksIntegration
{
    public class DlcData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public DlcData(GodotObject instance) => _instance = instance;

        public int Id => (int)_instance.Get("Id");

        public bool Installed => (bool)_instance.Get("Installed");

        public bool Subscribed => (bool)_instance.Get("Subscribed");

        public bool Available => (bool)_instance.Get("Available");

        public string DlcName => (string)_instance.Get("DlcName");

        public void Install() => _instance.Call("Install");

        public void Uninstall() => _instance.Call("Uninstall");

        public float DownloadProgress() => (float)_instance.Call("DownloadProgress");

        public static DlcData Get(int appId)
        {
            foreach (var dlc in SteamTools.GetDlc())
                if (dlc.Id == appId)
                    return dlc;
            return null;
        }
    }
}
