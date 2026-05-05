#pragma once

#include "SteamApi.h"

///<summary>
/// Holds a Steam achievement API name. All operations delegate to SteamApi.
///</summary>
class AchievementData : public RefCounted
{
    GDCLASS(AchievementData, RefCounted);

protected:
    String api_name;
    static void _bind_methods();

public:
    AchievementData() = default;
    explicit AchievementData(const String &name) : api_name(name) {}

    String GetApiName() const;
    String GetName() const;
    String GetDescription() const;
    bool GetIsHidden() const;
    bool GetIsAchieved() const;
    void SetIsAchieved(bool value) const;
    int GetUnlockTime() const;
    float GetGlobalPercent() const;
    void GetIcon(const Callable &callback) const;

    void Unlock() const;
    void Clear() const;
    bool Store() const;

    static Ref<AchievementData> Get(const String &apiName);
    static TypedArray<AchievementData> GetAll();

    bool Equals(Ref<AchievementData> other) const;

    bool operator==(const AchievementData &other) const { return api_name == other.api_name; }
    bool operator!=(const AchievementData &other) const { return api_name != other.api_name; }
};
