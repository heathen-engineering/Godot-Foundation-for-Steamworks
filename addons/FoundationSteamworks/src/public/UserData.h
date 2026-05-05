#pragma once

#include "SteamApi.h"

class LobbyData;

class UserData : public RefCounted
{
    GDCLASS(UserData, RefCounted);

protected:
    CSteamID id;
    static void _bind_methods();

public:
    UserData() = default;
    UserData(const CSteamID steam_id) : id(steam_id) {}

    void GetAvatar(godot::Callable callback);
    String GetRichPresence(const String &key) const;
    static void SetRichPresence(const String &key, const String &value);
    static void ClearRichPresence();
    void InviteToGame(const String &connectionString) const;
    void InviteToLobby(const Ref<LobbyData> &lobby) const;

    CSteamID ToCSteamID() const { return id; }
    uint64_t GetIntId() const;
    String GetName() const;
    int GetLevel() const;
    bool IsValid() const;
    String GetHexId() const;
    void SetHexId(String hexId);
    void SetPlayedWith() const;

    static Ref<UserData> Me();
    static Ref<UserData> FromUInt64(uint64_t steam_id);
    static Ref<UserData> FromHex(String hexId);
    static TypedArray<UserData> GetFriends();
    static TypedArray<UserData> GetCoplayFriends();
    static TypedArray<UserData> GetFriendsFromSource(const uint64_t sourceId);

    operator CSteamID() const;
    bool Equals(Ref<UserData> other) const;
    void ActivateOverlay(const String &type) const;

    bool operator==(const UserData &other) const { return id == other.id; }
    bool operator!=(const UserData &other) const { return id != other.id; }
    bool operator==(const CSteamID &other) const { return id == other; }
    bool operator!=(const CSteamID &other) const { return id != other; }
    bool operator==(uint64_t other) const { return GetIntId() == other; }
    bool operator!=(uint64_t other) const { return GetIntId() != other; }
    bool operator<(const UserData &other) const { return GetIntId() < other.GetIntId(); }
    bool operator>(const UserData &other) const { return GetIntId() > other.GetIntId(); }
};
