#include "ItemData.h"

void ItemData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetDefId"), &ItemData::GetDefId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "DefId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetDefId");

    ClassDB::bind_method(D_METHOD("GetName"), &ItemData::GetName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetName");

    ClassDB::bind_method(D_METHOD("GetDescription"), &ItemData::GetDescription);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Description", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetDescription");

    ClassDB::bind_method(D_METHOD("GetType"), &ItemData::GetType);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetType");

    ClassDB::bind_method(D_METHOD("GetProperty", "propertyName"), &ItemData::GetProperty);
    ClassDB::bind_method(D_METHOD("GetPrice"), &ItemData::GetPrice);
    ClassDB::bind_method(D_METHOD("GetIsStore"), &ItemData::GetIsStore);
    ClassDB::bind_method(D_METHOD("Equals", "other"), &ItemData::Equals);

    ClassDB::bind_static_method("ItemData", D_METHOD("Get", "defId"), &ItemData::Get);
    ClassDB::bind_static_method("ItemData", D_METHOD("GetAll"), &ItemData::GetAll);
}

int ItemData::GetDefId() const
{
    return static_cast<int>(item_def_id);
}

String ItemData::GetName() const
{
    if (!SteamApi::GetIsReady())
        return "";
    char buf[256];
    uint32 bufSize = sizeof(buf);
    SteamInventory()->GetItemDefinitionProperty(item_def_id, "name", buf, &bufSize);
    return String(buf);
}

String ItemData::GetDescription() const
{
    if (!SteamApi::GetIsReady())
        return "";
    char buf[512];
    uint32 bufSize = sizeof(buf);
    SteamInventory()->GetItemDefinitionProperty(item_def_id, "description", buf, &bufSize);
    return String(buf);
}

String ItemData::GetType() const
{
    if (!SteamApi::GetIsReady())
        return "";
    char buf[128];
    uint32 bufSize = sizeof(buf);
    SteamInventory()->GetItemDefinitionProperty(item_def_id, "type", buf, &bufSize);
    return String(buf);
}

String ItemData::GetProperty(const String &propertyName) const
{
    if (!SteamApi::GetIsReady())
        return "";
    char buf[1024];
    uint32 bufSize = sizeof(buf);
    SteamInventory()->GetItemDefinitionProperty(item_def_id, propertyName.utf8().get_data(), buf, &bufSize);
    return String(buf);
}

int64_t ItemData::GetPrice() const
{
    if (!SteamApi::GetIsReady())
        return 0;
    uint64 currentPrice = 0;
    uint64 basePrice = 0;
    SteamInventory()->GetItemPrice(item_def_id, &currentPrice, &basePrice);
    return static_cast<int64_t>(currentPrice);
}

bool ItemData::GetIsStore() const
{
    if (!SteamApi::GetIsReady())
        return false;
    char buf[8];
    uint32 bufSize = sizeof(buf);
    SteamInventory()->GetItemDefinitionProperty(item_def_id, "store_hidden", buf, &bufSize);
    return String(buf) != "1";
}

Ref<ItemData> ItemData::Get(int defId)
{
    return Ref<ItemData>(memnew(ItemData(static_cast<SteamItemDef_t>(defId))));
}

TypedArray<ItemData> ItemData::GetAll()
{
    TypedArray<ItemData> result;
    if (!SteamApi::GetIsReady())
        return result;
    SteamItemDef_t *pItemDefIDs = nullptr;
    uint32 count = 0;
    if (SteamInventory()->GetItemDefinitionIDs(pItemDefIDs, &count) && count > 0)
    {
        pItemDefIDs = new SteamItemDef_t[count];
        if (SteamInventory()->GetItemDefinitionIDs(pItemDefIDs, &count))
        {
            for (uint32 i = 0; i < count; i++)
                result.append(memnew(ItemData(pItemDefIDs[i])));
        }
        delete[] pItemDefIDs;
    }
    return result;
}

bool ItemData::Equals(Ref<ItemData> other) const
{
    return item_def_id == other->item_def_id;
}
