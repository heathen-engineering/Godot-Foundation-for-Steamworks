#pragma once

#include "SteamApi.h"
#include "LeaderboardEntryData.h"

///<summary>
/// References a Steam leaderboard by name. Provides typed download/upload helpers.
///</summary>
class LeaderboardData : public RefCounted
{
    GDCLASS(LeaderboardData, RefCounted);

protected:
    String leaderboard_name;
    static void _bind_methods();

public:
    LeaderboardData() = default;
    explicit LeaderboardData(const String &name) : leaderboard_name(name) {}

    String GetLeaderboardName() const;
    int GetEntryCount() const;
    SteamLeaderboardDisplay::Type GetDisplayType() const;
    bool GetIsTopRankLowestScore() const;
    uint64_t GetNativeId() const;

    void DownloadGlobalEntries(int start, int end, int detailCount, const Callable &callback) const;
    void DownloadAroundUserEntries(int start, int end, int detailCount, const Callable &callback) const;
    void DownloadFriendsEntries(int start, int end, int detailCount, const Callable &callback) const;
    void DownloadEntriesForUsers(TypedArray<UserData> users, int detailCount, const Callable &callback) const;

    void UploadScore(int score, const Callable &callback) const;
    void UploadScoreWithDetails(int score, const PackedInt32Array &details, const Callable &callback) const;

    static Ref<LeaderboardData> Get(const String &leaderboardName);
    static Ref<LeaderboardData> FindOrCreate(const String &leaderboardName, bool lowestScoreIsTopRank, SteamLeaderboardDisplay::Type displayType);

    bool Equals(Ref<LeaderboardData> other) const;

    bool operator==(const LeaderboardData &other) const { return leaderboard_name == other.leaderboard_name; }
    bool operator!=(const LeaderboardData &other) const { return leaderboard_name != other.leaderboard_name; }
};
