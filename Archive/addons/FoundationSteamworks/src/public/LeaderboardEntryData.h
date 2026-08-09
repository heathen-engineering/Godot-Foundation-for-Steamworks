#pragma once

#include "UserData.h"

class LeaderboardEntryData : public UserData
{
    GDCLASS(LeaderboardEntryData, UserData);

private:
    int rank;
    int score;
    PackedInt32Array details;
    UGCHandle_t ugcHandle;

protected:
    static void _bind_methods();

public:
    LeaderboardEntryData() = default;

    int GetRank() const;
    int GetScore() const;
    PackedInt32Array GetDetails() const;
    uint64_t GetUgcHandle() const;

    static Ref<LeaderboardEntryData> create(const CSteamID user_id, const int rankValue, const int scoreValue, const PackedInt32Array &detailsData, UGCHandle_t ugcHandleValue);
};
