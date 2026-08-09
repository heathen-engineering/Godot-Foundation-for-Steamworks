#pragma once

#include "SteamApi.h"

class LobbyComparison : public RefCounted
{
    GDCLASS(LobbyComparison, RefCounted);

public:
    enum Comparison
    {
        EqualToOrLessThan = -2,
        LessThan = -1,
        Equal = 0,
        GreaterThan = 1,
        EqualToOrGreaterThan = 2,
        NotEqual = 3,
    };

protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(LobbyComparison::Comparison);

class LobbyDistance : public RefCounted
{
    GDCLASS(LobbyDistance, RefCounted);

public:
    enum Distance
    {
        Close,
        Default,
        Far,
        Worldwide,
    };

protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(LobbyDistance::Distance);

struct SteamStringFilter
{
    String m_pchValue;
    ELobbyComparison m_eComparison;

    SteamStringFilter(const String &value, LobbyComparison::Comparison comparison)
        : m_pchValue(value), m_eComparison(static_cast<ELobbyComparison>(comparison)) {}
};

struct SteamNumericFilter
{
    int m_pchValue;
    ELobbyComparison m_eComparison;

    SteamNumericFilter(const int value, LobbyComparison::Comparison comparison)
        : m_pchValue(value), m_eComparison(static_cast<ELobbyComparison>(comparison)) {}
};

class LobbyQuery : public RefCounted
{
    GDCLASS(LobbyQuery, RefCounted);

private:
    LobbyDistance::Distance m_eDistanceFilter = LobbyDistance::Distance::Default;
    int m_nMaxResults = 50;
    int m_nMaxMembers = 0;
    HashMap<String, SteamStringFilter> StringFilters;
    HashMap<String, SteamNumericFilter> NumericFilters;
    HashMap<String, int> NearFilters;

protected:
    static void _bind_methods();

public:
    LobbyQuery() {}

    void SetDistanceFilter(LobbyDistance::Distance distance);
    LobbyDistance::Distance GetDistanceFilter() const;
    void SetMaxResults(int max_results);
    int GetMaxResults() const;
    void SetMaxMembers(int max_members);
    int GetMaxMembers() const;

    void SetStringFilter(const String &key, const String &value, LobbyComparison::Comparison comparison);
    String GetStringFilterValue(const String &key);
    LobbyComparison::Comparison GetStringFilterComparison(const String &key);
    void ClearStringFilter(const String &key);
    void ClearAllStringFilters();

    void SetNumericalFilter(const String &key, int value, LobbyComparison::Comparison comparison);
    int GetNumericalFilterValue(const String &key);
    LobbyComparison::Comparison GetNumericalFilterComparison(const String &key);
    void ClearNumericalFilter(const String &key);
    void ClearAllNumericalFilters();

    void SetNearFilter(const String &key, int value);
    int GetNearFilterValue(const String &key);
    void ClearNearFilter(const String &key);
    void ClearAllNearFilters();

    void ExecuteQuery(const Callable &callback);
};
