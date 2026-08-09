#pragma once

#include "SteamApi.h"

///<summary>
/// Identifies a Steam Inventory item definition. Provides quantity and property access.
///</summary>
class ItemData : public RefCounted
{
    GDCLASS(ItemData, RefCounted);

protected:
    SteamItemDef_t item_def_id;
    static void _bind_methods();

public:
    ItemData() = default;
    explicit ItemData(SteamItemDef_t id) : item_def_id(id) {}

    int GetDefId() const;
    String GetName() const;
    String GetDescription() const;
    String GetType() const;
    String GetProperty(const String &propertyName) const;
    int64_t GetPrice() const;
    bool GetIsStore() const;

    static Ref<ItemData> Get(int defId);
    static TypedArray<ItemData> GetAll();

    bool Equals(Ref<ItemData> other) const;

    bool operator==(const ItemData &other) const { return item_def_id == other.item_def_id; }
    bool operator!=(const ItemData &other) const { return item_def_id != other.item_def_id; }
};
