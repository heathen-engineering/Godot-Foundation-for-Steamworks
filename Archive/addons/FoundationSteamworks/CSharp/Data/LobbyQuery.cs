using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public enum LobbyComparison
    {
        EqualToOrLessThan,
        LessThan,
        Equal,
        GreaterThan,
        EqualToOrGreaterThan,
        NotEqual,
    }

    public enum LobbyDistance
    {
        Close,
        Default,
        Far,
        Worldwide,
    }

    public class LobbyQuery
    {
        private GodotObject _instance;

        public GodotObject ToGDNative() => _instance;

        public LobbyQuery()
        {
            _instance = (GodotObject)ClassDB.Instantiate("LobbyQuery");
        }

        public LobbyQuery(GodotObject instance) => _instance = instance;

        public LobbyDistance DistanceFilter
        {
            get => (LobbyDistance)(int)_instance.Call("GetDistanceFilter");
            set => _instance.Call("SetDistanceFilter", (int)value);
        }

        public int MaxResults
        {
            get => (int)_instance.Call("GetMaxResults");
            set => _instance.Call("SetMaxResults", value);
        }

        public int MaxMembers
        {
            get => (int)_instance.Call("GetMaxMembers");
            set => _instance.Call("SetMaxMembers", value);
        }

        public void SetStringFilter(string key, string value, LobbyComparison comparison) =>
            _instance.Call("SetStringFilter", key, value, (int)comparison);

        public string GetStringFilterValue(string key) => (string)_instance.Call("GetStringFilterValue", key);

        public LobbyComparison GetStringFilterComparison(string key) =>
            (LobbyComparison)(int)_instance.Call("GetStringFilterComparison", key);

        public void ClearStringFilter(string key) => _instance.Call("ClearStringFilter", key);

        public void ClearAllStringFilters() => _instance.Call("ClearAllStringFilters");

        public void SetNumericalFilter(string key, int value, LobbyComparison comparison) =>
            _instance.Call("SetNumericalFilter", key, value, (int)comparison);

        public int GetNumericalFilterValue(string key) => (int)_instance.Call("GetNumericalFilterValue", key);

        public LobbyComparison GetNumericalFilterComparison(string key) =>
            (LobbyComparison)(int)_instance.Call("GetNumericalFilterComparison", key);

        public void ClearNumericalFilter(string key) => _instance.Call("ClearNumericalFilter", key);

        public void ClearAllNumericalFilters() => _instance.Call("ClearAllNumericalFilters");

        public void SetNearFilter(string key, int value) => _instance.Call("SetNearFilter", key, value);

        public int GetNearFilterValue(string key) => (int)_instance.Call("GetNearFilterValue", key);

        public void ClearNearFilter(string key) => _instance.Call("ClearNearFilter", key);

        public void ClearAllNearFilters() => _instance.Call("ClearAllNearFilters");

        public void ExecuteQuery(Action<List<LobbyData>, bool> callback)
        {
            Callable callable = Callable.From((Variant listVar, bool ioError) =>
            {
                var lobbies = new List<LobbyData>();
                if (listVar.VariantType == Variant.Type.Array)
                    foreach (Variant item in (Godot.Collections.Array)listVar)
                        if (item.Obj is GodotObject obj)
                            lobbies.Add(new LobbyData(obj));
                callback?.Invoke(lobbies, ioError);
            });
            _instance.Call("ExecuteQuery", callable);
        }
    }
}
