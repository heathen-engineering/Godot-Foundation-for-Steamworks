using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public class LobbyData
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public LobbyData(GodotObject instance) => _instance = instance;

        public ulong Id => (ulong)_instance.Get("Id");

        public string HexId => (string)_instance.Get("HexId");

        public string LobbyName
        {
            get => (string)_instance.Call("GetLobbyName");
            set => _instance.Call("SetLobbyName", value);
        }

        public bool IsFull => (bool)_instance.Get("Full");

        public bool IsOwner => (bool)_instance.Call("IsOwner");

        public bool IsMember => (bool)_instance.Call("IsMember");

        public int MemberCount => (int)_instance.Get("MemberCount");

        public int MaxMembers
        {
            get => (int)_instance.Call("GetMaxMembers");
            set => _instance.Call("SetMaxMembers", value);
        }

        public LobbyUseHint UseHint
        {
            get => (LobbyUseHint)(int)_instance.Call("GetUseHint");
            set => _instance.Call("SetUseHint", (int)value);
        }

        public LobbyType Type
        {
            get => (LobbyType)(int)_instance.Call("GetType");
            set => _instance.Call("SetType", (int)value);
        }

        public string this[string key]
        {
            get => (string)_instance.Call("GetLobbyData", key);
            set => _instance.Call("SetLobbyData", key, value);
        }

        public static LobbyData FromHex(string hexId)
        {
            Variant result = ClassDB.ClassCallStatic("LobbyData", "FromHex", hexId);
            return result.Obj is GodotObject obj ? new LobbyData(obj) : null;
        }

        public static void CreateLobby(LobbyType type, LobbyUseHint hint, int maxMembers, Action<LobbyData, SteamResult, bool> callback) =>
            SteamTools.CreateLobby(type, hint, maxMembers, callback);

        public void JoinLobby(Action<LobbyData, LobbyEnterResponse, bool> callback) =>
            SteamTools.JoinLobby(this, callback);

        public static void JoinLobbyById(ulong id, Action<LobbyData, LobbyEnterResponse, bool> callback) =>
            SteamTools.JoinLobbyById(id, callback);

        public static void JoinLobbyByHex(string hexId, Action<LobbyData, LobbyEnterResponse, bool> callback) =>
            SteamTools.JoinLobbyByHex(hexId, callback);

        public void LeaveLobby() => SteamTools.LeaveLobby(this);

        public void SetMemberData(string key, string value) => _instance.Call("SetMemberData", key, value);

        public string GetMemberData(string key)
        {
            var nativeUser = UserData.Me.ToGDNative();
            return (string)_instance.Call("GetMemberData", nativeUser, key);
        }

        public string GetMemberData(UserData user, string key) =>
            (string)_instance.Call("GetMemberData", user.ToGDNative(), key);

        public void SetOwner(UserData user) => _instance.Call("SetOwner", user.ToGDNative());

        public List<UserData> GetMemberList()
        {
            Variant result = _instance.Call("GetMemberList");
            var list = new List<UserData>();
            if (result.VariantType != Variant.Type.Array)
                return list;
            foreach (Variant item in (Godot.Collections.Array)result)
                if (item.Obj is GodotObject obj)
                    list.Add(new UserData(obj));
            return list;
        }

        public void ActivateInviteDialog() => _instance.Call("ActivateInviteDialog");

        public void ActivateRemotePlayTogetherInviteDialog() => _instance.Call("ActivateRemotePlayTogetherInviteDialog");

        public void SetJoinable(bool joinable) => _instance.Call("SetJoinable", joinable);

        public void SetListenServer() => _instance.Call("SetListenServer");

        public void SetDedicatedServer(ulong serverId, string ip, ushort port) =>
            _instance.Call("SetDedicatedServer", serverId, ip, port);

        public bool HasGameServer() => (bool)_instance.Call("HasGameServer");

        public ulong GetServerId() => (ulong)_instance.Call("GetServerId");

        public string GetServerIp() => (string)_instance.Call("GetServerIp");

        public ushort GetServerPort() => (ushort)_instance.Call("GetServerPort");
    }
}
