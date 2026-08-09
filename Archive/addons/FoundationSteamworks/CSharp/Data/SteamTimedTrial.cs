using Godot;

namespace Heathen.SteamworksIntegration
{
    public struct SteamTimedTrial
    {
        public bool Active { get; private set; }
        public int Allowed { get; private set; }
        public int Played { get; private set; }

        public SteamTimedTrial(GodotObject instance)
        {
            Active = (bool)instance.Call("IsActive");
            Allowed = (int)instance.Call("GetAllowed");
            Played = (int)instance.Call("GetPlayed");
        }
    }
}
