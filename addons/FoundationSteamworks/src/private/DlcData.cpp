#include "DlcData.h"

void DlcData::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetIntId"), &DlcData::GetIntId);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "Id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIntId");

    ClassDB::bind_method(D_METHOD("GetIsInstalled"), &DlcData::GetIsInstalled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Installed", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIsInstalled");

    ClassDB::bind_method(D_METHOD("GetIsSubscribed"), &DlcData::GetIsSubscribed);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Subscribed", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIsSubscribed");

    ClassDB::bind_method(D_METHOD("GetIsAvailable"), &DlcData::GetIsAvailable);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Available", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetIsAvailable");

    ClassDB::bind_method(D_METHOD("GetDlcName"), &DlcData::GetDlcName);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "DlcName", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_READ_ONLY), StringName(), "GetDlcName");

    ClassDB::bind_method(D_METHOD("Install"), &DlcData::Install);
    ClassDB::bind_method(D_METHOD("Uninstall"), &DlcData::Uninstall);
    ClassDB::bind_method(D_METHOD("DownloadProgress"), &DlcData::DownloadProgress);

    ClassDB::bind_static_method("DlcData", D_METHOD("Get", "appId"), &DlcData::Get);
    ClassDB::bind_static_method("DlcData", D_METHOD("GetDlcData", "appId", "isAvailable", "name"), &DlcData::GetDlcData);
}

bool DlcData::GetIsInstalled() const
{
    if (SteamApi::GetIsReady())
        return SteamApps()->BIsDlcInstalled(id);
    return false;
}

bool DlcData::GetIsSubscribed() const
{
    if (SteamApi::GetIsReady())
        return SteamApps()->BIsSubscribedApp(id);
    return false;
}

bool DlcData::GetIsAvailable() const
{
    return available;
}

String DlcData::GetDlcName() const
{
    return name;
}

void DlcData::Install() const
{
    SteamApps()->InstallDLC(id);
}

void DlcData::Uninstall() const
{
    SteamApps()->UninstallDLC(id);
}

float DlcData::DownloadProgress() const
{
    uint64 total = 1;
    uint64 downloaded = 0;
    SteamApps()->GetDlcDownloadProgress(id, &downloaded, &total);
    return total > 0 ? static_cast<float>(downloaded) / static_cast<float>(total) : 0.f;
}

Ref<DlcData> DlcData::GetDlcData(uint32 appId, bool isAvailable, const String &cName)
{
    Ref<DlcData> nDlc(memnew(DlcData));
    nDlc->id = appId;
    nDlc->available = isAvailable;
    nDlc->name = cName;
    return nDlc;
}

Ref<DlcData> DlcData::Get(int appId)
{
    TypedArray<DlcData> dlc_list = SteamApi::GetDLC();
    for (int i = 0; i < dlc_list.size(); i++)
    {
        Ref<DlcData> dlc = dlc_list[i];
        if (dlc.is_valid() && dlc->id == (uint32_t)appId)
            return dlc;
    }
    return Ref<DlcData>();
}

int DlcData::GetIntId() const
{
    return static_cast<int>(id);
}

DlcData::operator AppId_t() const
{
    return id;
}
