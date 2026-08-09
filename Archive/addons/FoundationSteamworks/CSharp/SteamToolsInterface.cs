using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    /// <summary>
    /// Manages Steam initialisation and ready-state. Add SteamApiAutoload as an AutoLoad scene
    /// and it will call Initialise() from _Ready(). Query IsReady or register via WhenReady/OnReady.
    /// </summary>
    public static class SteamToolsInterface
    {
        private static GodotObject _singleton;
        private static readonly List<Action> _whenReadyCallbacks = new();

        public static bool IsReady => SteamTools.IsReady;

        public static void WhenReady(Action callback)
        {
            if (IsReady)
                callback?.Invoke();
            else
                _whenReadyCallbacks.Add(callback);
        }

        /// <summary>
        /// Called once from your AutoLoad node's _Ready(). Connects signals and initialises Steam.
        /// </summary>
        public static SteamInitialisationResponse Initialise()
        {
            _singleton = Engine.GetSingleton("SteamApi");
            SteamToolsEvents.Connect(_singleton);
            SteamToolsEvents.OnReady += HandleReady;

            return SteamTools.InitialiseClient();
        }

        private static void HandleReady()
        {
            SteamToolsEvents.OnReady -= HandleReady;

            var idsVariant = _singleton.Get("leaderboardIds");
            if (idsVariant.VariantType == Variant.Type.PackedStringArray)
                foreach (string name in idsVariant.As<string[]>())
                    if (!string.IsNullOrEmpty(name))
                        LeaderboardData.CacheFromName(name);

            foreach (var cb in _whenReadyCallbacks)
                cb?.Invoke();
            _whenReadyCallbacks.Clear();
        }
    }
}
