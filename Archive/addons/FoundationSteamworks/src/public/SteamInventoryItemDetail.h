#pragma once

#include "SteamApi.h"

class SteamInventoryItemDetail : public RefCounted
{
    GDCLASS(SteamInventoryItemDetail, RefCounted);

private:
    int itemId;
    int definitionId;
    int quantity;
    int flags;
    Dictionary properties;
    Dictionary tags;
    String dynamicProperties;

protected:
    static void _bind_methods();

public:
    SteamInventoryItemDetail() = default;

    int getItemId() const;
    int getDefinitionId() const;
    int getQuantity() const;
    int getFlags() const;
    Dictionary getProperties() const;
    Dictionary getTags() const;
    String getDynamicProperties() const;

    void setItemId(int value);
    void setDefinitionId(int value);
    void setQuantity(int value);
    void setFlags(int value);
    void setProperties(const Dictionary &value);
    void setTags(const Dictionary &value);
    void setDynamicProperties(const String &value);
};
