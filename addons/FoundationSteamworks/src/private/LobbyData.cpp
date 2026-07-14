#include "LobbyData.h"
#include <string>
#include <iomanip>

void LobbyData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetIntId"), &LobbyData::GetIntId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIntId");

    ClassDB::bind_method(D_METHOD("GetHexId"), &LobbyData::ToHexId);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "HexId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetHexId");

    ClassDB::bind_method(D_METHOD("Equals", "other"), &LobbyData::Equals);
    ClassDB::bind_static_method("LobbyData", D_METHOD("FromHex", "hexId"), &LobbyData::FromHex);
}

uint64_t LobbyData::GetIntId() const
{
    return id.ConvertToUint64();
}

LobbyData::operator CSteamID() const
{
    return id;
}

// Hex of the 32-bit AccountID/FriendID component (CSteamID::GetAccountID()),
// NOT the full 64-bit lobby SteamID — that's Id (GetIntId(), ConvertToUint64())
// above. Same distinction as UserData::GetHexId().
String LobbyData::ToHexId() const
{
    // See UserData::GetHexId()'s comment — std::stringstream here segfaults
    // deep inside libstdc++'s codecvt machinery on this project's Mono/
    // CoreCLR-flavored Godot editor build, reproduced even against the
    // pre-existing, unmodified code. String::num_uint64's hex mode is the
    // same output with zero iostream involvement.
    return String::num_uint64(id.GetAccountID(), 16, true);
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
