#include "LeaderboardEntryData.h"

void LeaderboardEntryData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetRank"), &LeaderboardEntryData::GetRank);
    ClassDB::bind_method(D_METHOD("GetScore"), &LeaderboardEntryData::GetScore);
    ClassDB::bind_method(D_METHOD("GetDetails"), &LeaderboardEntryData::GetDetails);
    ClassDB::bind_method(D_METHOD("GetUgcHandle"), &LeaderboardEntryData::GetUgcHandle);
}

int LeaderboardEntryData::GetRank() const { return rank; }
int LeaderboardEntryData::GetScore() const { return score; }
PackedInt32Array LeaderboardEntryData::GetDetails() const { return details; }
uint64_t LeaderboardEntryData::GetUgcHandle() const { return ugcHandle; }

Ref<LeaderboardEntryData> LeaderboardEntryData::create(
    const CSteamID user_id, const int rankValue, const int scoreValue,
    const PackedInt32Array &detailsData, UGCHandle_t ugcHandleValue)
{
    Ref<LeaderboardEntryData> obj;
    obj.instantiate();
    obj->id = user_id;
    obj->rank = rankValue;
    obj->score = scoreValue;
    obj->details = detailsData;
    obj->ugcHandle = ugcHandleValue;
    return obj;
}
