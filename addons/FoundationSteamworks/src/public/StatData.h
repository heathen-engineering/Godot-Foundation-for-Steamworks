#pragma once

#include "SteamApi.h"

///<summary>
/// Identifies a Steam stat by API name. Provides typed get/set helpers.
///</summary>
class StatData : public RefCounted
{
    GDCLASS(StatData, RefCounted);

protected:
    String api_name;
    static void _bind_methods();

public:
    StatData() = default;
    explicit StatData(const String &name) : api_name(name) {}

    String GetApiName() const;

    int GetIntValue() const;
    float GetFloatValue() const;
    void SetIntValue(int value) const;
    void SetFloatValue(float value) const;

    int GetUserIntValue(const Ref<UserData> &user) const;
    float GetUserFloatValue(const Ref<UserData> &user) const;

    bool Store() const;

    static Ref<StatData> Get(const String &apiName);

    bool Equals(Ref<StatData> other) const;

    bool operator==(const StatData &other) const { return api_name == other.api_name; }
    bool operator!=(const StatData &other) const { return api_name != other.api_name; }
};
