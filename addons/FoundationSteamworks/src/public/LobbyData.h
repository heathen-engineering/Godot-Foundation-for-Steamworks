#pragma once

#include "SteamApi.h"

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

    CSteamID ToCSteamID() const { return id; }
    uint64_t GetIntId() const;
    operator CSteamID() const;
    String ToHexId() const;
    static Ref<LobbyData> FromHex(String hexId);
    bool Equals(Ref<LobbyData> other) const;

    bool operator==(const LobbyData &other) const { return id == other.id; }
    bool operator!=(const LobbyData &other) const { return id != other.id; }
    bool operator==(const CSteamID &other) const { return id == other; }
    bool operator!=(const CSteamID &other) const { return id != other; }
    bool operator==(uint64_t other) const { return GetIntId() == other; }
    bool operator!=(uint64_t other) const { return GetIntId() != other; }
    bool operator<(const LobbyData &other) const { return GetIntId() < other.GetIntId(); }
    bool operator>(const LobbyData &other) const { return GetIntId() > other.GetIntId(); }
};
