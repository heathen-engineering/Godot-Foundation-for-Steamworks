#pragma once

#include "SteamApi.h"
#include "LobbyType.h"

class LobbyData : public RefCounted
{
    GDCLASS(LobbyData, RefCounted);

protected:
    CSteamID id;
    static void _bind_methods();

public:
    LobbyData() = default;
    LobbyData(CSteamID steam_id) : id(steam_id) {}
    LobbyData(uint64 steam_id) : id(steam_id) {}

    void InviteToLobby(const Ref<UserData> &user) const;
    static void CreateLobby(LobbyType::Type type, LobbyUseHint::Hint hint, int maxMembers, const Callable &callback);
    void JoinLobby(Callable callback);
    static void JoinLobbyById(int64_t lobby_id, Callable callback);
    static void JoinLobbyByHex(String lobby_id, Callable callback);
    void LeaveLobby() const;
    void SetLobbyData(const String &key, const String &value) const;
    String GetLobbyData(const String &key) const;
    void SetMemberData(const String &key, const String &value) const;
    String GetMemberData(const Ref<UserData> &user, const String &key) const;
    String GetLobbyName() const;
    void SetLobbyName(const String &name) const;
    int GetMemberCount() const;
    int GetMaxMembers() const;
    void SetMaxMembers(int max_members) const;
    LobbyUseHint::Hint GetUseHint() const;
    void SetUseHint(LobbyUseHint::Hint hint) const;
    LobbyType::Type GetType() const;
    void SetType(LobbyType::Type type) const;
    bool IsFull() const;
    bool IsOwner() const;
    void SetOwner(Ref<UserData> user) const;
    bool IsMember() const;
    void SetJoinable(bool joinable) const;
    void SetListenServer() const;
    void SetDedicatedServer(uint64_t serverId, const String &ip, uint16_t port) const;
    bool HasGameServer() const;
    uint64_t GetServerId() const;
    String GetServerIp() const;
    uint16_t GetServerPort() const;
    TypedArray<UserData> GetMemberList() const;
    Ref<UserData> GetOwner() const;
    bool SendChatMessage(const String &message) const;
    CSteamID ToCSteamID() const { return id; }
    uint64_t GetIntId() const;
    operator CSteamID() const;
    String ToHexId() const;
    static Ref<LobbyData> FromHex(String hexId);
    bool Equals(Ref<LobbyData> other) const;
    void ActivateInviteDialog() const;
    void ActivateRemotePlayTogetherInviteDialog() const;

    bool operator==(const LobbyData &other) const { return id == other.id; }
    bool operator!=(const LobbyData &other) const { return id != other.id; }
    bool operator==(const CSteamID &other) const { return id == other; }
    bool operator!=(const CSteamID &other) const { return id != other; }
    bool operator==(uint64_t other) const { return GetIntId() == other; }
    bool operator!=(uint64_t other) const { return GetIntId() != other; }
    bool operator<(const LobbyData &other) const { return GetIntId() < other.GetIntId(); }
    bool operator>(const LobbyData &other) const { return GetIntId() > other.GetIntId(); }

    static constexpr auto DataName = "name";
    static constexpr auto DataVersion = "z_heathenGameVersion";
    static constexpr auto DataReady = "z_heathenReady";
    static constexpr auto DataKick = "z_heathenKick";
    static constexpr auto DataMode = "z_heathenMode";
    static constexpr auto DataType = "z_heathenType";
    static constexpr auto DataModeGeneral = "General";
    static constexpr auto DataModeSession = "Session";
    static constexpr auto DataModeParty = "Party";
    static constexpr auto DataTypePrivate = "Private";
    static constexpr auto DataTypeFriendOnly = "FriendOnly";
    static constexpr auto DataTypePublic = "Public";
    static constexpr auto DataTypeInvisible = "Invisible";
};
