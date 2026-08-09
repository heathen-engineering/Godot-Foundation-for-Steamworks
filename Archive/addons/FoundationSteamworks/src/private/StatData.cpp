#include "StatData.h"
#include "UserData.h"

void StatData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetApiName"), &StatData::GetApiName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "ApiName", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetApiName");

    ClassDB::bind_method(D_METHOD("GetIntValue"), &StatData::GetIntValue);
    ClassDB::bind_method(D_METHOD("GetFloatValue"), &StatData::GetFloatValue);
    ClassDB::bind_method(D_METHOD("SetIntValue", "value"), &StatData::SetIntValue);
    ClassDB::bind_method(D_METHOD("SetFloatValue", "value"), &StatData::SetFloatValue);
    ClassDB::bind_method(D_METHOD("GetUserIntValue", "user"), &StatData::GetUserIntValue);
    ClassDB::bind_method(D_METHOD("GetUserFloatValue", "user"), &StatData::GetUserFloatValue);
    ClassDB::bind_method(D_METHOD("Store"), &StatData::Store);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &StatData::Equals);

    ClassDB::bind_static_method("StatData", D_METHOD("Get", "apiName"), &StatData::Get);
}

String StatData::GetApiName() const
{
    return api_name;
}

int StatData::GetIntValue() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    int32 value = 0;
    SteamUserStats()->GetStat(api_name.utf8().get_data(), &value);
    return value;
}

float StatData::GetFloatValue() const
{
    if (!SteamApi::GetIsReady())
        return 0.f;
    float value = 0.f;
    SteamUserStats()->GetStat(api_name.utf8().get_data(), &value);
    return value;
}

void StatData::SetIntValue(int value) const
{
    if (SteamApi::GetIsReady())
        SteamUserStats()->SetStat(api_name.utf8().get_data(), static_cast<int32>(value));
}

void StatData::SetFloatValue(float value) const
{
    if (SteamApi::GetIsReady())
        SteamUserStats()->SetStat(api_name.utf8().get_data(), value);
}

int StatData::GetUserIntValue(const Ref<UserData> &user) const
{
    if (!SteamApi::GetIsReady())
        return 0;
    int32 value = 0;
    SteamUserStats()->GetUserStat(user->ToCSteamID(), api_name.utf8().get_data(), &value);
    return value;
}

float StatData::GetUserFloatValue(const Ref<UserData> &user) const
{
    if (!SteamApi::GetIsReady())
        return 0.f;
    float value = 0.f;
    SteamUserStats()->GetUserStat(user->ToCSteamID(), api_name.utf8().get_data(), &value);
    return value;
}

bool StatData::Store() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return SteamApi::StoreStats();
}

Ref<StatData> StatData::Get(const String &apiName)
{
    return Ref<StatData>(memnew(StatData(apiName)));
}

bool StatData::Equals(Ref<StatData> other) const
{
    return api_name == other->api_name;
}
