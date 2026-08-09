#include "SteamInventoryItemDetail.h"

void SteamInventoryItemDetail::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getItemId"), &SteamInventoryItemDetail::getItemId);
    ClassDB::bind_method(D_METHOD("getDefinitionId"), &SteamInventoryItemDetail::getDefinitionId);
    ClassDB::bind_method(D_METHOD("getQuantity"), &SteamInventoryItemDetail::getQuantity);
    ClassDB::bind_method(D_METHOD("getFlags"), &SteamInventoryItemDetail::getFlags);
    ClassDB::bind_method(D_METHOD("getProperties"), &SteamInventoryItemDetail::getProperties);
    ClassDB::bind_method(D_METHOD("getTags"), &SteamInventoryItemDetail::getTags);
    ClassDB::bind_method(D_METHOD("getDynamicProperties"), &SteamInventoryItemDetail::getDynamicProperties);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "itemId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getItemId");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "definitionId", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getDefinitionId");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "quantity", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getQuantity");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "flags", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getFlags");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "properties", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getProperties");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "tags", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getTags");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "dynamicProperties", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "getDynamicProperties");
}

int SteamInventoryItemDetail::getItemId() const { return itemId; }
int SteamInventoryItemDetail::getDefinitionId() const { return definitionId; }
int SteamInventoryItemDetail::getQuantity() const { return quantity; }
int SteamInventoryItemDetail::getFlags() const { return flags; }
Dictionary SteamInventoryItemDetail::getProperties() const { return properties; }
Dictionary SteamInventoryItemDetail::getTags() const { return tags; }
String SteamInventoryItemDetail::getDynamicProperties() const { return dynamicProperties; }

void SteamInventoryItemDetail::setItemId(int value) { itemId = value; }
void SteamInventoryItemDetail::setDefinitionId(int value) { definitionId = value; }
void SteamInventoryItemDetail::setQuantity(int value) { quantity = value; }
void SteamInventoryItemDetail::setFlags(int value) { flags = value; }
void SteamInventoryItemDetail::setProperties(const Dictionary &value) { properties = value; }
void SteamInventoryItemDetail::setTags(const Dictionary &value) { tags = value; }
void SteamInventoryItemDetail::setDynamicProperties(const String &value) { dynamicProperties = value; }
