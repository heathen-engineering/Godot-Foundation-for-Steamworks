#pragma once

#include "SteamApi.h"
#include "UserData.h"

///<summary>
/// Represents a Steam Clan (Group). Provides name, tag, and member-count access.
///</summary>
class ClanData : public RefCounted
{
    GDCLASS(ClanData, RefCounted);

protected:
    CSteamID id;
    static void _bind_methods();

public:
    ClanData() = default;
    explicit ClanData(CSteamID steam_id) : id(steam_id) {}

    CSteamID ToCSteamID() const { return id; }
    uint64_t GetIntId() const;
    String GetName() const;
    String GetTag() const;
    int GetMemberCount() const;
    bool IsPublic() const;
    bool IsOfficialGameGroup() const;
    TypedArray<UserData> GetMembers() const;
    void ActivateChatOverlay() const;

    static Ref<ClanData> Get(uint64_t steamId);
    static TypedArray<ClanData> GetClans();

    bool Equals(Ref<ClanData> other) const;

    bool operator==(const ClanData &other) const { return id == other.id; }
    bool operator!=(const ClanData &other) const { return id != other.id; }
    bool operator<(const ClanData &other) const { return GetIntId() < other.GetIntId(); }
    bool operator>(const ClanData &other) const { return GetIntId() > other.GetIntId(); }
};
