using Godot;

namespace Heathen.SteamworksIntegration
{
    public class SteamInitialisationResponse
    {
        private GodotObject _instance;

        public SteamInitialisationResponse(GodotObject instance) => _instance = instance;

        public bool Success => (bool)_instance.Get("Success");

        public bool ShouldRestart => (bool)_instance.Get("ShouldRestart");

        public string Message => (string)_instance.Get("Message");
    }
}
