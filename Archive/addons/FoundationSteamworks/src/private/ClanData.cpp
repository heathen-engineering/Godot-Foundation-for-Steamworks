#include "ClanData.h"

void ClanData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetIntId"), &ClanData::GetIntId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "IntId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIntId");

    ClassDB::bind_method(D_METHOD("GetName"), &ClanData::GetName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetName");

    ClassDB::bind_method(D_METHOD("GetTag"), &ClanData::GetTag);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Tag", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetTag");

    ClassDB::bind_method(D_METHOD("GetMemberCount"), &ClanData::GetMemberCount);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "MemberCount", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetMemberCount");

    ClassDB::bind_method(D_METHOD("IsPublic"), &ClanData::IsPublic);
    ClassDB::bind_method(D_METHOD("IsOfficialGameGroup"), &ClanData::IsOfficialGameGroup);
    ClassDB::bind_method(D_METHOD("GetMembers"), &ClanData::GetMembers);
    ClassDB::bind_method(D_METHOD("ActivateChatOverlay"), &ClanData::ActivateChatOverlay);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &ClanData::Equals);

    ClassDB::bind_static_method("ClanData", D_METHOD("Get", "steamId"), &ClanData::Get);
    ClassDB::bind_static_method("ClanData", D_METHOD("GetClans"), &ClanData::GetClans);
}

uint64_t ClanData::GetIntId() const
{
    return id.ConvertToUint64();
}

String ClanData::GetName() const
{
    if (!SteamApi::GetIsReady())
        return "";
    return String(SteamFriends()->GetClanName(id));
}

String ClanData::GetTag() const
{
    if (!SteamApi::GetIsReady())
        return "";
    return String(SteamFriends()->GetClanTag(id));
}

int ClanData::GetMemberCount() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    return SteamFriends()->GetClanChatMemberCount(id);
}

bool ClanData::IsPublic() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return SteamFriends()->IsClanPublic(id);
}

bool ClanData::IsOfficialGameGroup() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return SteamFriends()->IsClanOfficialGameGroup(id);
}

Ref<ClanData> ClanData::Get(uint64_t steamId)
{
    return Ref<ClanData>(memnew(ClanData(CSteamID(static_cast<uint64>(steamId)))));
}

TypedArray<ClanData> ClanData::GetClans()
{
    TypedArray<ClanData> result;
    if (!SteamApi::GetIsReady())
        return result;
    const int count = SteamFriends()->GetClanCount();
    for (int i = 0; i < count; i++)
        result.append(memnew(ClanData(SteamFriends()->GetClanByIndex(i))));
    return result;
}

bool ClanData::Equals(Ref<ClanData> other) const
{
    return id == other->id;
}

TypedArray<UserData> ClanData::GetMembers() const
{
    TypedArray<UserData> members;
    if (!SteamApi::GetIsReady())
        return members;
    const int count = SteamFriends()->GetFriendCountFromSource(id);
    for (int i = 0; i < count; i++)
        members.append(memnew(UserData(SteamFriends()->GetFriendFromSourceByIndex(id, i))));
    return members;
}

void ClanData::ActivateChatOverlay() const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->ActivateGameOverlayToUser("chat", id);
}
