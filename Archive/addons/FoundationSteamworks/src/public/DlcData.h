#pragma once

#include "SteamApi.h"

class DlcData : public RefCounted
{
    GDCLASS(DlcData, RefCounted);

protected:
    AppId_t id;
    bool installed = false;
    bool subscribed = false;
    bool available = false;
    String name;
    static void _bind_methods();

public:
    DlcData() = default;

    bool GetIsInstalled() const;
    bool GetIsSubscribed() const;
    bool GetIsAvailable() const;
    String GetDlcName() const;
    void Install() const;
    void Uninstall() const;
    float DownloadProgress() const;

    static Ref<DlcData> GetDlcData(uint32 appId, bool isAvailable, const String &cName);
    static Ref<DlcData> Get(int appId);

    int GetIntId() const;
    operator AppId_t() const;

    bool operator==(const DlcData &other) const { return id == other.id; }
    bool operator!=(const DlcData &other) const { return id != other.id; }
    bool operator==(const AppId_t &other) const { return id == other; }
    bool operator!=(const AppId_t &other) const { return id != other; }
    bool operator==(uint32_t other) const { return GetIntId() == other; }
    bool operator!=(uint32_t other) const { return GetIntId() != other; }
    bool operator<(const DlcData &other) const { return GetIntId() < other.GetIntId(); }
    bool operator>(const DlcData &other) const { return GetIntId() > other.GetIntId(); }
};
