#include "AchievementData.h"

void AchievementData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetApiName"), &AchievementData::GetApiName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "ApiName", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetApiName");

    ClassDB::bind_method(D_METHOD("GetName"), &AchievementData::GetName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetName");

    ClassDB::bind_method(D_METHOD("GetDescription"), &AchievementData::GetDescription);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Description", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetDescription");

    ClassDB::bind_method(D_METHOD("GetIsHidden"), &AchievementData::GetIsHidden);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "IsHidden", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIsHidden");

    ClassDB::bind_method(D_METHOD("GetIsAchieved"), &AchievementData::GetIsAchieved);
    ClassDB::bind_method(D_METHOD("SetIsAchieved", "value"), &AchievementData::SetIsAchieved);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "IsAchieved"), "SetIsAchieved", "GetIsAchieved");

    ClassDB::bind_method(D_METHOD("GetUnlockTime"), &AchievementData::GetUnlockTime);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "UnlockTime", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetUnlockTime");

    ClassDB::bind_method(D_METHOD("GetGlobalPercent"), &AchievementData::GetGlobalPercent);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "GlobalPercent", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetGlobalPercent");

    ClassDB::bind_method(D_METHOD("GetIcon", "callback"), &AchievementData::GetIcon);
    ClassDB::bind_method(D_METHOD("Unlock"), &AchievementData::Unlock);
    ClassDB::bind_method(D_METHOD("Clear"), &AchievementData::Clear);
    ClassDB::bind_method(D_METHOD("Store"), &AchievementData::Store);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &AchievementData::Equals);

    ClassDB::bind_static_method("AchievementData", D_METHOD("Get", "apiName"), &AchievementData::Get);
    ClassDB::bind_static_method("AchievementData", D_METHOD("GetAll"), &AchievementData::GetAll);
}

String AchievementData::GetApiName() const
{
    return api_name;
}

String AchievementData::GetName() const
{
    if (!SteamApi::GetIsReady())
        return "";
    return String(SteamUserStats()->GetAchievementDisplayAttribute(api_name.utf8().get_data(), "name"));
}

String AchievementData::GetDescription() const
{
    if (!SteamApi::GetIsReady())
        return "";
    return String(SteamUserStats()->GetAchievementDisplayAttribute(api_name.utf8().get_data(), "desc"));
}

bool AchievementData::GetIsHidden() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return String(SteamUserStats()->GetAchievementDisplayAttribute(api_name.utf8().get_data(), "hidden")) == "1";
}

bool AchievementData::GetIsAchieved() const
{
    if (!SteamApi::GetIsReady())
        return false;
    bool achieved = false;
    SteamUserStats()->GetAchievement(api_name.utf8().get_data(), &achieved);
    return achieved;
}

void AchievementData::SetIsAchieved(bool value) const
{
    if (!SteamApi::GetIsReady())
        return;
    if (value)
        SteamUserStats()->SetAchievement(api_name.utf8().get_data());
    else
        SteamUserStats()->ClearAchievement(api_name.utf8().get_data());
}

int AchievementData::GetUnlockTime() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    bool achieved = false;
    uint32 unlock_time = 0;
    SteamUserStats()->GetAchievementAndUnlockTime(api_name.utf8().get_data(), &achieved, &unlock_time);
    return achieved ? static_cast<int>(unlock_time) : 0;
}

float AchievementData::GetGlobalPercent() const
{
    if (!SteamApi::GetIsReady())
        return 0.f;
    float percent = 0.f;
    SteamUserStats()->GetAchievementAchievedPercent(api_name.utf8().get_data(), &percent);
    return percent;
}

void AchievementData::GetIcon(const Callable &callback) const
{
    if (SteamApi::GetIsReady())
        SteamApi::GetAchievementIcon(api_name, callback);
}

void AchievementData::Unlock() const
{
    SetIsAchieved(true);
}

void AchievementData::Clear() const
{
    SetIsAchieved(false);
}

bool AchievementData::Store() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return SteamApi::StoreStats();
}

Ref<AchievementData> AchievementData::Get(const String &apiName)
{
    return Ref<AchievementData>(memnew(AchievementData(apiName)));
}

TypedArray<AchievementData> AchievementData::GetAll()
{
    TypedArray<AchievementData> result;
    if (!SteamApi::GetIsReady())
        return result;
    PackedStringArray names = SteamApi::GetAchievements();
    for (int i = 0; i < names.size(); i++)
        result.append(memnew(AchievementData(names[i])));
    return result;
}

bool AchievementData::Equals(Ref<AchievementData> other) const
{
    return api_name == other->api_name;
}
