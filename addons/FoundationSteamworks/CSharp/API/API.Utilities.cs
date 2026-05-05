namespace Heathen.SteamworksIntegration.API
{
    public static class Utilities
    {
        public static uint IpStringToUint(string ip) => SteamTools.IpStringToUint(ip);

        public static string IpUintToString(uint ip) => SteamTools.IpUintToString(ip);
    }
}
