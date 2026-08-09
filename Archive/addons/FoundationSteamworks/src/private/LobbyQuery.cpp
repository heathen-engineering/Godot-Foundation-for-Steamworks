#include "LobbyQuery.h"

void LobbyComparison::_bind_methods()
{
    BIND_ENUM_CONSTANT(EqualToOrLessThan);
    BIND_ENUM_CONSTANT(LessThan);
    BIND_ENUM_CONSTANT(Equal);
    BIND_ENUM_CONSTANT(GreaterThan);
    BIND_ENUM_CONSTANT(EqualToOrGreaterThan);
    BIND_ENUM_CONSTANT(NotEqual);
}

void LobbyDistance::_bind_methods()
{
    BIND_ENUM_CONSTANT(Close);
    BIND_ENUM_CONSTANT(Default);
    BIND_ENUM_CONSTANT(Far);
    BIND_ENUM_CONSTANT(Worldwide);
}

void LobbyQuery::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("SetDistanceFilter", "distance"), &LobbyQuery::SetDistanceFilter);
    ClassDB::bind_method(D_METHOD("GetDistanceFilter"), &LobbyQuery::GetDistanceFilter);
    ClassDB::bind_method(D_METHOD("SetMaxResults", "max_results"), &LobbyQuery::SetMaxResults);
    ClassDB::bind_method(D_METHOD("GetMaxResults"), &LobbyQuery::GetMaxResults);
    ClassDB::bind_method(D_METHOD("SetMaxMembers", "max_members"), &LobbyQuery::SetMaxMembers);
    ClassDB::bind_method(D_METHOD("GetMaxMembers"), &LobbyQuery::GetMaxMembers);
    ClassDB::bind_method(D_METHOD("SetStringFilter", "key", "value", "comparison"), &LobbyQuery::SetStringFilter);
    ClassDB::bind_method(D_METHOD("GetStringFilterValue", "key"), &LobbyQuery::GetStringFilterValue);
    ClassDB::bind_method(D_METHOD("GetStringFilterComparison", "key"), &LobbyQuery::GetStringFilterComparison);
    ClassDB::bind_method(D_METHOD("ClearStringFilter", "key"), &LobbyQuery::ClearStringFilter);
    ClassDB::bind_method(D_METHOD("ClearAllStringFilters"), &LobbyQuery::ClearAllStringFilters);
    ClassDB::bind_method(D_METHOD("SetNumericalFilter", "key", "value", "comparison"), &LobbyQuery::SetNumericalFilter);
    ClassDB::bind_method(D_METHOD("GetNumericalFilterValue", "key"), &LobbyQuery::GetNumericalFilterValue);
    ClassDB::bind_method(D_METHOD("GetNumericalFilterComparison", "key"), &LobbyQuery::GetNumericalFilterComparison);
    ClassDB::bind_method(D_METHOD("ClearNumericalFilter", "key"), &LobbyQuery::ClearNumericalFilter);
    ClassDB::bind_method(D_METHOD("ClearAllNumericalFilters"), &LobbyQuery::ClearAllNumericalFilters);
    ClassDB::bind_method(D_METHOD("SetNearFilter", "key", "value"), &LobbyQuery::SetNearFilter);
    ClassDB::bind_method(D_METHOD("GetNearFilterValue", "key"), &LobbyQuery::GetNearFilterValue);
    ClassDB::bind_method(D_METHOD("ClearNearFilter", "key"), &LobbyQuery::ClearNearFilter);
    ClassDB::bind_method(D_METHOD("ClearAllNearFilters"), &LobbyQuery::ClearAllNearFilters);
    ClassDB::bind_method(D_METHOD("ExecuteQuery", "callback"), &LobbyQuery::ExecuteQuery);
}

void LobbyQuery::SetDistanceFilter(LobbyDistance::Distance distance) { m_eDistanceFilter = distance; }
LobbyDistance::Distance LobbyQuery::GetDistanceFilter() const { return m_eDistanceFilter; }
void LobbyQuery::SetMaxResults(int max_results) { m_nMaxResults = max_results; }
int LobbyQuery::GetMaxResults() const { return m_nMaxResults; }
void LobbyQuery::SetMaxMembers(int max_members) { m_nMaxMembers = max_members; }
int LobbyQuery::GetMaxMembers() const { return m_nMaxMembers; }

void LobbyQuery::SetStringFilter(const String &key, const String &value, LobbyComparison::Comparison comparison)
{
    StringFilters.insert(key, SteamStringFilter(value, comparison));
}
String LobbyQuery::GetStringFilterValue(const String &key)
{
    return StringFilters.has(key) ? StringFilters.get(key).m_pchValue : "";
}
LobbyComparison::Comparison LobbyQuery::GetStringFilterComparison(const String &key)
{
    return StringFilters.has(key) ? static_cast<LobbyComparison::Comparison>(StringFilters.get(key).m_eComparison) : LobbyComparison::Comparison::Equal;
}
void LobbyQuery::ClearStringFilter(const String &key) { StringFilters.erase(key); }
void LobbyQuery::ClearAllStringFilters() { StringFilters.clear(); }

void LobbyQuery::SetNumericalFilter(const String &key, int value, LobbyComparison::Comparison comparison)
{
    NumericFilters.insert(key, SteamNumericFilter(value, comparison));
}
int LobbyQuery::GetNumericalFilterValue(const String &key)
{
    return NumericFilters.has(key) ? NumericFilters.get(key).m_pchValue : 0;
}
LobbyComparison::Comparison LobbyQuery::GetNumericalFilterComparison(const String &key)
{
    return NumericFilters.has(key) ? static_cast<LobbyComparison::Comparison>(NumericFilters.get(key).m_eComparison) : LobbyComparison::Comparison::Equal;
}
void LobbyQuery::ClearNumericalFilter(const String &key) { NumericFilters.erase(key); }
void LobbyQuery::ClearAllNumericalFilters() { NumericFilters.clear(); }

void LobbyQuery::SetNearFilter(const String &key, int value) { NearFilters.insert(key, value); }
int LobbyQuery::GetNearFilterValue(const String &key) { return NearFilters.has(key) ? NearFilters.get(key) : 0; }
void LobbyQuery::ClearNearFilter(const String &key) { NearFilters.erase(key); }
void LobbyQuery::ClearAllNearFilters() { NearFilters.clear(); }

void LobbyQuery::ExecuteQuery(const Callable &callback)
{
    SteamMatchmaking()->AddRequestLobbyListDistanceFilter(static_cast<ELobbyDistanceFilter>(m_eDistanceFilter));
    if (m_nMaxMembers > 0)
        SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(m_nMaxMembers);
    SteamMatchmaking()->AddRequestLobbyListResultCountFilter(m_nMaxResults);

    for (const auto &[key, value] : StringFilters)
        SteamMatchmaking()->AddRequestLobbyListStringFilter(key.utf8().get_data(), value.m_pchValue.utf8().get_data(), static_cast<ELobbyComparison>(value.m_eComparison));
    for (const auto &[key, value] : NumericFilters)
        SteamMatchmaking()->AddRequestLobbyListNumericalFilter(key.utf8().get_data(), value.m_pchValue, static_cast<ELobbyComparison>(value.m_eComparison));
    for (const auto &[key, value] : NearFilters)
        SteamMatchmaking()->AddRequestLobbyListNearValueFilter(key.utf8().get_data(), value);

    SteamApi::LobbyMatchList(callback);
}
