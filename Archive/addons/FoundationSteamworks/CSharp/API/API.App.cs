using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration.API
{
    public static class App
    {
        public static int BuildId => SteamTools.AppBuildId;

        public static string GetInstallDir(int appId) => SteamTools.GetAppInstallDir(appId);

        public static UserData Owner => SteamTools.AppOwner;

        public static string AvailableGameLanguages => SteamTools.AvailableGameLanguages;

        public static string CurrentBetaName => SteamTools.CurrentBetaName;

        public static string CurrentGameLanguage => SteamTools.CurrentGameLanguage;

        public static int DlcCount => SteamTools.DlcCount;

        public static List<DlcData> GetDlc() => SteamTools.GetDlc();

        public static DateTime GetEarliestPurchaseTime(int appId) => SteamTools.GetEarliestPurchaseTime(appId);

        public static string LaunchCommandLine => SteamTools.LaunchCommandLine;

        public static string GetLaunchQueryParam(string key) => SteamTools.GetLaunchQueryParam(key);

        public static bool IsInstalled(int appId) => SteamTools.IsAppInstalled(appId);

        public static bool IsCybercafe => SteamTools.IsCybercafe;

        public static bool IsLowViolence => SteamTools.IsLowViolence;

        public static bool IsSubscribed => SteamTools.IsSubscribed;

        public static bool IsSubscribedApp(int appId) => SteamTools.IsSubscribedApp(appId);

        public static bool IsSubscribedFromFamilySharing => SteamTools.IsSubscribedFromFamilySharing;

        public static bool IsSubscribedFromFreeWeekend => SteamTools.IsSubscribedFromFreeWeekend;

        public static SteamTimedTrial TimedTrial => SteamTools.IsTimedTrial;

        public static bool IsVACBanned => SteamTools.IsVACBanned;

        public static bool MarkContentCorrupt(bool missingFilesOnly) => SteamTools.MarkContentCorrupt(missingFilesOnly);

        public static void ActivateGameOverlay(string type) => SteamTools.ActivateGameOverlay(type);

        public static void ActivateGameOverlayInviteDialogConnectString(string connectString) =>
            SteamTools.ActivateGameOverlayInviteDialogConnectString(connectString);

        public static void ActivateGameOverlayToStore(int appId) => SteamTools.ActivateGameOverlayToStore(appId);

        public static void ActivateGameOverlayToWebPage(string url, bool modal = false) =>
            SteamTools.ActivateGameOverlayToWebPage(url, modal);
    }
}
