using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration.API
{
    public static class User
    {
        public static UserData Me => UserData.Me;

        public static void GetAvatar(UserData user, Action<Texture2D> callback) =>
            SteamTools.GetFriendAvatar(user, callback);

        public static void RequestStats(UserData user) => SteamTools.RequestUserStats(user);

        public static void RequestStats(ulong steamId) => SteamTools.RequestUserStats(steamId);

        public static void SetRichPresence(string key, string value) =>
            UserData.SetRichPresence(key, value);

        public static void ClearRichPresence() => UserData.ClearRichPresence();

        public static List<UserData> GetFriends() => UserData.GetFriends();

        public static List<UserData> GetCoplayFriends() => UserData.GetCoplayFriends();

        public static List<UserData> GetFriendsFromSource(ulong sourceId) =>
            UserData.GetFriendsFromSource(sourceId);

        /// <summary>Mirrors Unity Foundation's API.User.Client nesting for call-site parity.</summary>
        public static class Client
        {
            public static UserData Id => Me;
            public static void GetAvatar(UserData user, Action<Texture2D> callback) => User.GetAvatar(user, callback);
            public static void RequestStats(UserData user) => User.RequestStats(user);
            public static void RequestStats(ulong steamId) => User.RequestStats(steamId);
            public static void SetRichPresence(string key, string value) => User.SetRichPresence(key, value);
            public static void ClearRichPresence() => User.ClearRichPresence();
            public static List<UserData> GetFriends() => User.GetFriends();
            public static List<UserData> GetCoplayFriends() => User.GetCoplayFriends();
            public static List<UserData> GetFriendsFromSource(ulong sourceId) => User.GetFriendsFromSource(sourceId);
        }
    }
}
