#include "LeaderboardData.h"

void LeaderboardData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetLeaderboardName"), &LeaderboardData::GetLeaderboardName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "LeaderboardName", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetLeaderboardName");

    ClassDB::bind_method(D_METHOD("GetEntryCount"), &LeaderboardData::GetEntryCount);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "EntryCount", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetEntryCount");

    ClassDB::bind_method(D_METHOD("GetDisplayType"), &LeaderboardData::GetDisplayType);
    ClassDB::bind_method(D_METHOD("GetIsTopRankLowestScore"), &LeaderboardData::GetIsTopRankLowestScore);
    ClassDB::bind_method(D_METHOD("GetNativeId"), &LeaderboardData::GetNativeId);

    ClassDB::bind_method(D_METHOD("DownloadGlobalEntries", "start", "end", "detailCount", "callback"), &LeaderboardData::DownloadGlobalEntries);
    ClassDB::bind_method(D_METHOD("DownloadAroundUserEntries", "start", "end", "detailCount", "callback"), &LeaderboardData::DownloadAroundUserEntries);
    ClassDB::bind_method(D_METHOD("DownloadFriendsEntries", "start", "end", "detailCount", "callback"), &LeaderboardData::DownloadFriendsEntries);
    ClassDB::bind_method(D_METHOD("DownloadEntriesForUsers", "users", "detailCount", "callback"), &LeaderboardData::DownloadEntriesForUsers);

    ClassDB::bind_method(D_METHOD("Equals", "other"), &LeaderboardData::Equals);

    ClassDB::bind_static_method("LeaderboardData", D_METHOD("Get", "leaderboardName"), &LeaderboardData::Get);
    ClassDB::bind_static_method("LeaderboardData", D_METHOD("FindOrCreate", "leaderboardName", "lowestScoreIsTopRank", "displayType"), &LeaderboardData::FindOrCreate);
}

String LeaderboardData::GetLeaderboardName() const
{
    return leaderboard_name;
}

int LeaderboardData::GetEntryCount() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    return SteamApi::GetLeaderboardEntryCount(leaderboard_name);
}

SteamLeaderboardDisplay::Type LeaderboardData::GetDisplayType() const
{
    if (!SteamApi::GetIsReady())
        return SteamLeaderboardDisplay::Type::Numeric;
    return SteamApi::GetLeaderboardDisplayType(leaderboard_name);
}

bool LeaderboardData::GetIsTopRankLowestScore() const
{
    if (!SteamApi::GetIsReady())
        return false;
    return SteamApi::IsLeaderboardTopRankLowestScore(leaderboard_name);
}

uint64_t LeaderboardData::GetNativeId() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    return SteamApi::GetLeaderboardNativeId(leaderboard_name);
}

void LeaderboardData::DownloadGlobalEntries(int start, int end, int detailCount, const Callable &callback) const
{
    if (SteamApi::GetIsReady())
        SteamApi::DownloadLeaderboardGlobalEntries(start, end, detailCount, leaderboard_name, callback);
}

void LeaderboardData::DownloadAroundUserEntries(int start, int end, int detailCount, const Callable &callback) const
{
    if (SteamApi::GetIsReady())
        SteamApi::DownloadLeaderboardAroundUserEntries(start, end, detailCount, leaderboard_name, callback);
}

void LeaderboardData::DownloadFriendsEntries(int start, int end, int detailCount, const Callable &callback) const
{
    if (SteamApi::GetIsReady())
        SteamApi::DownloadLeaderboardFriendsEntries(start, end, detailCount, leaderboard_name, callback);
}

void LeaderboardData::DownloadEntriesForUsers(TypedArray<UserData> users, int detailCount, const Callable &callback) const
{
    if (SteamApi::GetIsReady())
        SteamApi::DownloadLeaderboardEntriesForUsers(users, detailCount, leaderboard_name, callback);
}

Ref<LeaderboardData> LeaderboardData::Get(const String &leaderboardName)
{
    return Ref<LeaderboardData>(memnew(LeaderboardData(leaderboardName)));
}

Ref<LeaderboardData> LeaderboardData::FindOrCreate(const String &leaderboardName, bool lowestScoreIsTopRank, SteamLeaderboardDisplay::Type displayType)
{
    if (SteamApi::GetIsReady())
        SteamApi::FindOrCreateLeaderboard(leaderboardName, lowestScoreIsTopRank, displayType);
    return Ref<LeaderboardData>(memnew(LeaderboardData(leaderboardName)));
}

bool LeaderboardData::Equals(Ref<LeaderboardData> other) const
{
    return leaderboard_name == other->leaderboard_name;
}
