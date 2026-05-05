#include "LobbyData.h"
#include "UserData.h"
#include "LobbyType.h"
#include <string>
#include <iomanip>

void LobbyData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetIntId"), &LobbyData::GetIntId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIntId");

    ClassDB::bind_method(D_METHOD("GetHexId"), &LobbyData::ToHexId);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "HexId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetHexId");

    ClassDB::bind_static_method("LobbyData", D_METHOD("CreateLobby", "type", "hint", "maxMembers", "callback"), &LobbyData::CreateLobby);
    ClassDB::bind_method(D_METHOD("JoinLobby", "callback"), &LobbyData::JoinLobby);
    ClassDB::bind_static_method("LobbyData", D_METHOD("JoinLobbyById", "lobby_id", "callback"), &LobbyData::JoinLobbyById);
    ClassDB::bind_static_method("LobbyData", D_METHOD("JoinLobbyByHex", "lobby_id", "callback"), &LobbyData::JoinLobbyByHex);
    ClassDB::bind_method(D_METHOD("LeaveLobby"), &LobbyData::LeaveLobby);
    ClassDB::bind_method(D_METHOD("SetLobbyData", "key", "value"), &LobbyData::SetLobbyData);
    ClassDB::bind_method(D_METHOD("GetLobbyData", "key"), &LobbyData::GetLobbyData);
    ClassDB::bind_method(D_METHOD("SetMemberData", "key", "value"), &LobbyData::SetMemberData);
    ClassDB::bind_method(D_METHOD("GetMemberData", "user", "key"), &LobbyData::GetMemberData);

    ClassDB::bind_method(D_METHOD("GetLobbyName"), &LobbyData::GetLobbyName);
    ClassDB::bind_method(D_METHOD("SetLobbyName", "name"), &LobbyData::SetLobbyName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "LobbyName"), "SetLobbyName", "GetLobbyName");

    ClassDB::bind_method(D_METHOD("GetMemberCount"), &LobbyData::GetMemberCount);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "MemberCount", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetMemberCount");

    ClassDB::bind_method(D_METHOD("GetMaxMembers"), &LobbyData::GetMaxMembers);
    ClassDB::bind_method(D_METHOD("SetMaxMembers", "max"), &LobbyData::SetMaxMembers);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "MaxMembers"), "SetMaxMembers", "GetMaxMembers");

    ClassDB::bind_method(D_METHOD("GetUseHint"), &LobbyData::GetUseHint);
    ClassDB::bind_method(D_METHOD("SetUseHint", "hint"), &LobbyData::SetUseHint);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "UseHint", PROPERTY_HINT_ENUM, "General,Session,Party"), "SetUseHint", "GetUseHint");

    ClassDB::bind_method(D_METHOD("GetType"), &LobbyData::GetType);
    ClassDB::bind_method(D_METHOD("SetType", "type"), &LobbyData::SetType);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Type", PROPERTY_HINT_ENUM, "Private,FriendsOnly,Public,Invisible"), "SetType", "GetType");

    ClassDB::bind_method(D_METHOD("IsFull"), &LobbyData::IsFull);
    ClassDB::bind_method(D_METHOD("IsOwner"), &LobbyData::IsOwner);
    ClassDB::bind_method(D_METHOD("SetOwner", "user"), &LobbyData::SetOwner);
    ClassDB::bind_method(D_METHOD("IsMember"), &LobbyData::IsMember);
    ClassDB::bind_method(D_METHOD("SetJoinable", "joinable"), &LobbyData::SetJoinable);
    ClassDB::bind_method(D_METHOD("SetListenServer"), &LobbyData::SetListenServer);
    ClassDB::bind_method(D_METHOD("SetDedicatedServer", "serverId", "ip", "port"), &LobbyData::SetDedicatedServer);
    ClassDB::bind_method(D_METHOD("HasGameServer"), &LobbyData::HasGameServer);
    ClassDB::bind_method(D_METHOD("GetServerId"), &LobbyData::GetServerId);
    ClassDB::bind_method(D_METHOD("GetServerIp"), &LobbyData::GetServerIp);
    ClassDB::bind_method(D_METHOD("GetServerPort"), &LobbyData::GetServerPort);
    ClassDB::bind_method(D_METHOD("GetMemberList"), &LobbyData::GetMemberList);
    ClassDB::bind_method(D_METHOD("GetOwner"), &LobbyData::GetOwner);
    ClassDB::bind_method(D_METHOD("SendChatMessage", "message"), &LobbyData::SendChatMessage);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &LobbyData::Equals);
    ClassDB::bind_method(D_METHOD("ActivateInviteDialog"), &LobbyData::ActivateInviteDialog);
    ClassDB::bind_method(D_METHOD("ActivateRemotePlayTogetherInviteDialog"), &LobbyData::ActivateRemotePlayTogetherInviteDialog);

    ClassDB::bind_static_method("LobbyData", D_METHOD("FromHex", "hexId"), &LobbyData::FromHex);
}

void LobbyData::InviteToLobby(const Ref<UserData> &user) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->InviteUserToLobby(id, user->ToCSteamID());
}

void LobbyData::CreateLobby(LobbyType::Type type, LobbyUseHint::Hint hint, int maxMembers, const Callable &callback)
{
    SteamApi::CreateLobby(type, hint, maxMembers, callback);
}

void LobbyData::JoinLobby(Callable callback)
{
    SteamApi::JoinLobby(this, callback);
}

void LobbyData::JoinLobbyById(int64_t lobby_id, Callable callback)
{
    SteamApi::JoinLobbyById(lobby_id, callback);
}

void LobbyData::JoinLobbyByHex(String lobby_id, Callable callback)
{
    SteamApi::JoinLobbyByHex(lobby_id, callback);
}

void LobbyData::LeaveLobby() const
{
    SteamApi::LeaveLobby(Ref<LobbyData>(this));
}

void LobbyData::SetLobbyData(const String &key, const String &value) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyData(id, key.utf8().get_data(), value.utf8().get_data());
}

String LobbyData::GetLobbyData(const String &key) const
{
    if (SteamApi::GetIsReady())
        return String(SteamMatchmaking()->GetLobbyData(id, key.utf8().get_data()));
    return "";
}

void LobbyData::SetMemberData(const String &key, const String &value) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyMemberData(id, key.utf8().get_data(), value.utf8().get_data());
}

String LobbyData::GetMemberData(const Ref<UserData> &user, const String &key) const
{
    if (SteamApi::GetIsReady())
        return String(SteamMatchmaking()->GetLobbyMemberData(id, user->ToCSteamID(), key.utf8().get_data()));
    return "";
}

String LobbyData::GetLobbyName() const
{
    if (SteamApi::GetIsReady())
        return String(SteamMatchmaking()->GetLobbyData(id, DataName));
    return "";
}

void LobbyData::SetLobbyName(const String &name) const
{
    SetLobbyData(DataName, name);
}

int LobbyData::GetMemberCount() const
{
    if (SteamApi::GetIsReady())
        return SteamMatchmaking()->GetNumLobbyMembers(id);
    return 0;
}

int LobbyData::GetMaxMembers() const
{
    if (SteamApi::GetIsReady())
        return SteamMatchmaking()->GetLobbyMemberLimit(id);
    return 0;
}

void LobbyData::SetMaxMembers(int max_members) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyMemberLimit(id, max_members);
}

LobbyUseHint::Hint LobbyData::GetUseHint() const
{
    if (!SteamApi::GetIsReady())
        return LobbyUseHint::Hint::General;
    const char *value = SteamMatchmaking()->GetLobbyData(id, DataMode);
    if (value == nullptr) return LobbyUseHint::Hint::General;
    if (strcmp(value, DataModeGeneral) == 0) return LobbyUseHint::Hint::General;
    if (strcmp(value, DataModeSession) == 0) return LobbyUseHint::Hint::Session;
    if (strcmp(value, DataModeParty) == 0) return LobbyUseHint::Hint::Party;
    return LobbyUseHint::Hint::General;
}

void LobbyData::SetUseHint(LobbyUseHint::Hint hint) const
{
    if (hint == LobbyUseHint::Hint::General) SetLobbyData(DataMode, DataModeGeneral);
    else if (hint == LobbyUseHint::Hint::Session) SetLobbyData(DataMode, DataModeSession);
    else if (hint == LobbyUseHint::Hint::Party) SetLobbyData(DataMode, DataModeParty);
}

LobbyType::Type LobbyData::GetType() const
{
    if (!SteamApi::GetIsReady())
        return LobbyType::Type::Public;
    const char *value = SteamMatchmaking()->GetLobbyData(id, DataType);
    if (value == nullptr) return LobbyType::Type::Public;
    if (strcmp(value, DataTypePrivate) == 0) return LobbyType::Type::Private;
    if (strcmp(value, DataTypeFriendOnly) == 0) return LobbyType::Type::FriendsOnly;
    if (strcmp(value, DataTypePublic) == 0) return LobbyType::Type::Public;
    if (strcmp(value, DataTypeInvisible) == 0) return LobbyType::Type::Invisible;
    return LobbyType::Type::Public;
}

void LobbyData::SetType(LobbyType::Type type) const
{
    SteamMatchmaking()->SetLobbyType(id, static_cast<ELobbyType>(type));
    if (type == LobbyType::Type::Private) SetLobbyData(DataType, DataTypePrivate);
    else if (type == LobbyType::Type::FriendsOnly) SetLobbyData(DataType, DataTypeFriendOnly);
    else if (type == LobbyType::Type::Public) SetLobbyData(DataType, DataTypePublic);
    else if (type == LobbyType::Type::Invisible) SetLobbyData(DataType, DataTypeInvisible);
}

bool LobbyData::IsFull() const
{
    if (SteamApi::GetIsReady())
        return GetMemberCount() >= GetMaxMembers();
    return false;
}

bool LobbyData::IsOwner() const
{
    if (SteamApi::GetIsReady())
        return SteamMatchmaking()->GetLobbyOwner(id) == SteamUser()->GetSteamID();
    return false;
}

void LobbyData::SetOwner(Ref<UserData> user) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyOwner(id, user->ToCSteamID());
}

bool LobbyData::IsMember() const
{
    if (!SteamApi::GetIsReady())
        return false;
    const int count = SteamMatchmaking()->GetNumLobbyMembers(id);
    for (int i = 0; i < count; i++)
    {
        if (SteamMatchmaking()->GetLobbyMemberByIndex(id, i) == SteamUser()->GetSteamID())
            return true;
    }
    return false;
}

void LobbyData::SetJoinable(bool joinable) const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyJoinable(id, joinable);
}

void LobbyData::SetListenServer() const
{
    if (SteamApi::GetIsReady())
        SteamMatchmaking()->SetLobbyGameServer(id, 0, 0, SteamMatchmaking()->GetLobbyOwner(id));
}

void LobbyData::SetDedicatedServer(uint64_t serverId, const String &ip, uint16_t port) const
{
    if (!SteamApi::GetIsReady())
        return;
    const uint32 unIP = SteamApi::IpStringToUint(ip);
    SteamMatchmaking()->SetLobbyGameServer(id, unIP, port, CSteamID(static_cast<uint64>(serverId)));
}

bool LobbyData::HasGameServer() const
{
    if (!SteamApi::GetIsReady())
        return false;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    return SteamMatchmaking()->GetLobbyGameServer(id, &unIP, &usPort, &steamServerID);
}

uint64_t LobbyData::GetServerId() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(id, &unIP, &usPort, &steamServerID);
    return steamServerID.ConvertToUint64();
}

String LobbyData::GetServerIp() const
{
    if (!SteamApi::GetIsReady())
        return "";
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(id, &unIP, &usPort, &steamServerID);
    return SteamApi::IpUintToString(unIP);
}

uint16_t LobbyData::GetServerPort() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    uint32 unIP = 0;
    uint16 usPort = 0;
    CSteamID steamServerID;
    SteamMatchmaking()->GetLobbyGameServer(id, &unIP, &usPort, &steamServerID);
    return usPort;
}

TypedArray<UserData> LobbyData::GetMemberList() const
{
    TypedArray<UserData> members;
    if (!SteamApi::GetIsReady())
        return members;
    const int count = SteamMatchmaking()->GetNumLobbyMembers(id);
    for (int i = 0; i < count; i++)
        members.append(memnew(UserData(SteamMatchmaking()->GetLobbyMemberByIndex(id, i))));
    return members;
}

Ref<UserData> LobbyData::GetOwner() const
{
    if (!SteamApi::GetIsReady())
        return {};
    return Ref<UserData>(memnew(UserData(SteamMatchmaking()->GetLobbyOwner(id))));
}

bool LobbyData::SendChatMessage(const String &message) const
{
    if (!SteamApi::GetIsReady())
        return false;
    const std::string utf8 = message.utf8().get_data();
    return SteamMatchmaking()->SendLobbyChatMsg(id, utf8.c_str(), static_cast<int>(utf8.size()));
}

uint64_t LobbyData::GetIntId() const
{
    return id.ConvertToUint64();
}

LobbyData::operator CSteamID() const
{
    return id;
}

String LobbyData::ToHexId() const
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << id.GetAccountID();
    return String(ss.str().c_str());
}

Ref<LobbyData> LobbyData::FromHex(String hexId)
{
    uint32_t accountId = static_cast<uint32_t>(hexId.hex_to_int());
    CSteamID steamId(accountId, 393216, k_EUniversePublic, k_EAccountTypeChat);
    return memnew(LobbyData(steamId));
}

bool LobbyData::Equals(Ref<LobbyData> other) const
{
    return id == other->id;
}

void LobbyData::ActivateInviteDialog() const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->ActivateGameOverlayInviteDialog(id);
}

void LobbyData::ActivateRemotePlayTogetherInviteDialog() const
{
    if (SteamApi::GetIsReady())
        SteamFriends()->ActivateGameOverlayRemotePlayTogetherInviteDialog(id);
}
