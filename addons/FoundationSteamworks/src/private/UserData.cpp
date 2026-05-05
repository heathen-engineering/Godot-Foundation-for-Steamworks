#include "UserData.h"
#include "LobbyData.h"
#include <string>
#include <iomanip>

void UserData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetName"), &UserData::GetName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "UserName", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetName");

    ClassDB::bind_method(D_METHOD("GetIsValid"), &UserData::IsValid);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "IsValid", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIsValid");

    ClassDB::bind_method(D_METHOD("GetLevel"), &UserData::GetLevel);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Level", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetLevel");

    ClassDB::bind_method(D_METHOD("GetHexId"), &UserData::GetHexId);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "HexId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetHexId");

    ClassDB::bind_method(D_METHOD("GetIntId"), &UserData::GetIntId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "IntId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIntId");

    ClassDB::bind_method(D_METHOD("GetAvatar", "callback"), &UserData::GetAvatar);
    ClassDB::bind_method(D_METHOD("GetRichPresence", "key"), &UserData::GetRichPresence);
    ClassDB::bind_method(D_METHOD("InviteToGame", "connectionString"), &UserData::InviteToGame);
    ClassDB::bind_method(D_METHOD("InviteToLobby", "lobby"), &UserData::InviteToLobby);
    ClassDB::bind_method(D_METHOD("SetPlayedWith"), &UserData::SetPlayedWith);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &UserData::Equals);
    ClassDB::bind_method(D_METHOD("ActivateOverlay", "type"), &UserData::ActivateOverlay);

    ClassDB::bind_static_method("UserData", D_METHOD("Me"), &UserData::Me);
    ClassDB::bind_static_method("UserData", D_METHOD("FromHex", "hexId"), &UserData::FromHex);
    ClassDB::bind_static_method("UserData", D_METHOD("FromUInt64", "steamId"), &UserData::FromUInt64);
    ClassDB::bind_static_method("UserData", D_METHOD("SetRichPresence", "key", "value"), &UserData::SetRichPresence);
    ClassDB::bind_static_method("UserData", D_METHOD("ClearRichPresence"), &UserData::ClearRichPresence);
    ClassDB::bind_static_method("UserData", D_METHOD("GetFriends"), &UserData::GetFriends);
    ClassDB::bind_static_method("UserData", D_METHOD("GetCoplayFriends"), &UserData::GetCoplayFriends);
    ClassDB::bind_static_method("UserData", D_METHOD("GetFriendsFromSource", "sourceId"), &UserData::GetFriendsFromSource);
}

void UserData::GetAvatar(godot::Callable callback)
{
    if (SteamApi::GetIsReady())
        SteamApi::GetFriendAvatar(this, callback);
}

String UserData::GetRichPresence(const String &key) const
{
    if (SteamApi::GetIsReady())
        return SteamFriends()->GetFriendRichPresence(id, key.utf8().get_data());
    return "";
}

void UserData::SetRichPresence(const String &key, const String &value)
{
    if (SteamApi::GetIsReady())
        SteamFriends()->SetRichPresence(key.utf8().get_data(), value.utf8().get_data());
}

void UserData::ClearRichPresence()
{
    if (SteamApi::GetIsReady())
        SteamFriends()->ClearRichPresence();
}

void UserData::InviteToGame(const String &connectionString) const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->InviteUserToGame(id, connectionString.utf8().get_data());
}

void UserData::InviteToLobby(const Ref<LobbyData> &lobby) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->InviteUserToLobby(lobby->ToCSteamID(), id);
}

uint64_t UserData::GetIntId() const
{
    return id.ConvertToUint64();
}

String UserData::GetName() const
{
    if (SteamApi::GetIsReady())
        return String(SteamFriends()->GetFriendPersonaName(id));
    return "";
}

int UserData::GetLevel() const
{
    if (SteamApi::GetIsReady())
        return SteamFriends()->GetFriendSteamLevel(id);
    return 0;
}

bool UserData::IsValid() const
{
    return id != CSteamID() &&
           id.GetEAccountType() == k_EAccountTypeIndividual &&
           id.GetEUniverse() == k_EUniversePublic;
}

String UserData::GetHexId() const
{
    uint32 account_id = id.GetAccountID();
    std::stringstream ss;
    ss << std::hex << std::uppercase << account_id;
    return String(ss.str().c_str());
}

void UserData::SetHexId(String hexId)
{
    const char *hex_str = hexId.utf8().get_data();
    const uint32_t account_id = static_cast<uint32_t>(strtoul(hex_str, nullptr, 16));
    id.Set(account_id, k_EUniversePublic, k_EAccountTypeIndividual);
}

void UserData::SetPlayedWith() const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->SetPlayedWith(id);
}

Ref<UserData> UserData::Me()
{
    if (!SteamUser() || !SteamFriends())
        return Ref<UserData>();
    return Ref<UserData>(memnew(UserData(SteamUser()->GetSteamID())));
}

Ref<UserData> UserData::FromUInt64(uint64_t steam_id)
{
    return Ref<UserData>(memnew(UserData(CSteamID(static_cast<uint64>(steam_id)))));
}

Ref<UserData> UserData::FromHex(String hexId)
{
    uint32_t accountId = static_cast<uint32_t>(hexId.hex_to_int());
    CSteamID steamId(accountId, k_EUniversePublic, k_EAccountTypeIndividual);
    return memnew(UserData(steamId));
}

TypedArray<UserData> UserData::GetFriends()
{
    TypedArray<UserData> friends;
    if (!SteamApi::GetIsReady())
        return friends;
    const int count = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
    for (int i = 0; i < count; i++)
        friends.append(memnew(UserData(SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate))));
    return friends;
}

TypedArray<UserData> UserData::GetCoplayFriends()
{
    TypedArray<UserData> friends;
    if (!SteamApi::GetIsReady())
        return friends;
    const int count = SteamFriends()->GetCoplayFriendCount();
    for (int i = 0; i < count; i++)
        friends.append(memnew(UserData(SteamFriends()->GetCoplayFriend(i))));
    return friends;
}

TypedArray<UserData> UserData::GetFriendsFromSource(const uint64_t sourceId)
{
    TypedArray<UserData> friends;
    if (!SteamApi::GetIsReady())
        return friends;
    const auto source = CSteamID(static_cast<uint64>(sourceId));
    const int count = SteamFriends()->GetFriendCountFromSource(source);
    for (int i = 0; i < count; i++)
        friends.append(memnew(UserData(SteamFriends()->GetFriendFromSourceByIndex(source, i))));
    return friends;
}

UserData::operator CSteamID() const
{
    return id;
}

bool UserData::Equals(Ref<UserData> other) const
{
    return id == other->id;
}

void UserData::ActivateOverlay(const String &type) const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->ActivateGameOverlayToUser(type.utf8().get_data(), id);
}
