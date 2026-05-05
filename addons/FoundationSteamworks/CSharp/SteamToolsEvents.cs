using Godot;
using System;
using System.Collections.Generic;

namespace Heathen.SteamworksIntegration
{
    public static class SteamToolsEvents
    {
        public static event Action OnReady;

        // --- Stats & Achievements ---
        public static event Action<ulong, SteamResult, ulong> OnUserStatsReceived;
        public static event Action<ulong> OnUserStatsUnloaded;
        public static event Action<ulong, SteamResult> OnUserStatsStored;
        public static event Action<ulong, bool, string, int, int> OnUserAchievementStored;

        // --- App ---
        public static event Action<int> OnDlcInstalled;

        // --- Steam Servers ---
        public static event Action<SteamResult, bool> OnSteamServerConnectFailure;
        public static event Action OnSteamServersConnected;
        public static event Action<SteamResult> OnSteamServersDisconnected;

        // --- Friends ---
        public static event Action<ulong, int> OnFriendRichPresenceUpdate;
        public static event Action<ulong, int> OnPersonaStateChange;
        public static event Action<ulong, string> OnGameRichPresenceJoinRequested;

        // --- Overlay ---
        public static event Action<bool> OnGameOverlayActivated;

        // --- Lobby Join Request ---
        public static event Action<ulong, ulong> OnGameLobbyJoinRequested;

        // --- Inventory ---
        public static event Action<int, SteamResult> OnSteamInventoryResultReady;
        public static event Action OnSteamInventoryDefinitionUpdate;

        // --- MicroTxn ---
        public static event Action<int, ulong, bool> OnMicroTxnAuthorizationResponse;

        // --- Favorites ---
        public static event Action<SteamResult> OnFavoritesListAccountsUpdated;
        public static event Action OnFavoritesListChanged;

        // --- Lobby ---
        public static event Action<ulong, ulong, string> OnLobbyChatMsg;
        public static event Action<ulong, ulong, ulong, int> OnLobbyChatUpdate;
        public static event Action<ulong, ulong, bool> OnLobbyDataUpdate;
        public static event Action<ulong, ulong, uint, ushort> OnLobbyGameCreated;
        public static event Action<ulong, ulong> OnLobbyInvite;
        public static event Action<LobbyData> OnLobbyCreated;
        public static event Action<LobbyData, LobbyEnterResponse> OnLobbyEntered;
        public static event Action<List<LobbyData>> OnLobbyMatchList;

        // --- Remote Play ---
        public static event Action<int> OnRemotePlaySessionConnected;
        public static event Action<int> OnRemotePlaySessionDisconnected;
        public static event Action<string> OnRemotePlaySessionGuestInvite;

        // --- Auth ---
        public static event Action<ulong, int> OnValidateAuthTicket;
        public static event Action<int, SteamResult> OnGetTicketForWebApiResponse;

        // --- Leaderboards ---
        public static event Action<string> OnLeaderboardFound;

        // --- Game Server ---
        public static event Action<SteamResult, bool> OnSteamGameServerConnectFailure;
        public static event Action OnSteamGameServersConnected;
        public static event Action<SteamResult> OnSteamGameServersDisconnected;

        public static void Connect(GodotObject singleton)
        {
            singleton.Connect("OnReady", Callable.From(() => OnReady?.Invoke()));

            singleton.Connect("OnUserStatsReceived", Callable.From((ulong gameId, int result, ulong steamId) =>
                OnUserStatsReceived?.Invoke(gameId, (SteamResult)result, steamId)));
            singleton.Connect("OnUserStatsUnloaded", Callable.From((ulong steamId) =>
                OnUserStatsUnloaded?.Invoke(steamId)));
            singleton.Connect("OnUserStatsStored", Callable.From((ulong gameId, int result) =>
                OnUserStatsStored?.Invoke(gameId, (SteamResult)result)));
            singleton.Connect("OnUserAchievementStored", Callable.From((ulong gameId, bool groupAchievement, string name, int curProgress, int maxProgress) =>
                OnUserAchievementStored?.Invoke(gameId, groupAchievement, name, curProgress, maxProgress)));

            singleton.Connect("OnDlcInstalled", Callable.From((int appId) =>
                OnDlcInstalled?.Invoke(appId)));

            singleton.Connect("OnSteamServerConnectFailure", Callable.From((int result, bool stillRetrying) =>
                OnSteamServerConnectFailure?.Invoke((SteamResult)result, stillRetrying)));
            singleton.Connect("OnSteamServersConnected", Callable.From(() =>
                OnSteamServersConnected?.Invoke()));
            singleton.Connect("OnSteamServersDisconnected", Callable.From((int result) =>
                OnSteamServersDisconnected?.Invoke((SteamResult)result)));

            singleton.Connect("OnFriendRichPresenceUpdate", Callable.From((ulong steamId, int appId) =>
                OnFriendRichPresenceUpdate?.Invoke(steamId, appId)));
            singleton.Connect("OnPersonaStateChange", Callable.From((ulong steamId, int changeFlags) =>
                OnPersonaStateChange?.Invoke(steamId, changeFlags)));
            singleton.Connect("OnGameRichPresenceJoinRequested", Callable.From((ulong steamId, string connect) =>
                OnGameRichPresenceJoinRequested?.Invoke(steamId, connect)));

            singleton.Connect("OnGameOverlayActivated", Callable.From((bool active) =>
                OnGameOverlayActivated?.Invoke(active)));

            singleton.Connect("OnGameLobbyJoinRequested", Callable.From((ulong lobbyId, ulong friendId) =>
                OnGameLobbyJoinRequested?.Invoke(lobbyId, friendId)));

            singleton.Connect("OnSteamInventoryResultReady", Callable.From((int handle, int result) =>
                OnSteamInventoryResultReady?.Invoke(handle, (SteamResult)result)));
            singleton.Connect("OnSteamInventoryDefinitionUpdate", Callable.From(() =>
                OnSteamInventoryDefinitionUpdate?.Invoke()));

            singleton.Connect("OnMicroTxnAuthorizationResponse", Callable.From((int appId, ulong orderId, bool authorized) =>
                OnMicroTxnAuthorizationResponse?.Invoke(appId, orderId, authorized)));

            singleton.Connect("OnFavoritesListAccountsUpdated", Callable.From((int result) =>
                OnFavoritesListAccountsUpdated?.Invoke((SteamResult)result)));
            singleton.Connect("OnFavoritesListChanged", Callable.From(() =>
                OnFavoritesListChanged?.Invoke()));

            singleton.Connect("OnLobbyChatMsg", Callable.From((ulong lobbyId, ulong userId, string message) =>
                OnLobbyChatMsg?.Invoke(lobbyId, userId, message)));
            singleton.Connect("OnLobbyChatUpdate", Callable.From((ulong lobbyId, ulong userChanged, ulong makingChange, int stateChange) =>
                OnLobbyChatUpdate?.Invoke(lobbyId, userChanged, makingChange, stateChange)));
            singleton.Connect("OnLobbyDataUpdate", Callable.From((ulong lobbyId, ulong memberId, bool success) =>
                OnLobbyDataUpdate?.Invoke(lobbyId, memberId, success)));
            singleton.Connect("OnLobbyGameCreated", Callable.From((ulong lobbyId, ulong gameServerId, uint ip, ushort port) =>
                OnLobbyGameCreated?.Invoke(lobbyId, gameServerId, ip, port)));
            singleton.Connect("OnLobbyInvite", Callable.From((ulong userId, ulong lobbyId) =>
                OnLobbyInvite?.Invoke(userId, lobbyId)));

            singleton.Connect("OnLobbyCreated", Callable.From((Variant lobbyVar) =>
            {
                LobbyData lobby = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                OnLobbyCreated?.Invoke(lobby);
            }));
            singleton.Connect("OnLobbyEntered", Callable.From((Variant lobbyVar, int response) =>
            {
                LobbyData lobby = lobbyVar.Obj is GodotObject obj ? new LobbyData(obj) : null;
                OnLobbyEntered?.Invoke(lobby, (LobbyEnterResponse)response);
            }));
            singleton.Connect("OnLobbyMatchList", Callable.From((Variant listVar) =>
            {
                var lobbies = new List<LobbyData>();
                if (listVar.VariantType == Variant.Type.Array)
                    foreach (Variant item in (Godot.Collections.Array)listVar)
                        if (item.Obj is GodotObject obj)
                            lobbies.Add(new LobbyData(obj));
                OnLobbyMatchList?.Invoke(lobbies);
            }));

            singleton.Connect("OnRemotePlaySessionConnected", Callable.From((int sessionId) =>
                OnRemotePlaySessionConnected?.Invoke(sessionId)));
            singleton.Connect("OnRemotePlaySessionDisconnected", Callable.From((int sessionId) =>
                OnRemotePlaySessionDisconnected?.Invoke(sessionId)));
            singleton.Connect("OnRemotePlaySessionGuestInvite", Callable.From((string url) =>
                OnRemotePlaySessionGuestInvite?.Invoke(url)));

            singleton.Connect("OnValidateAuthTicket", Callable.From((ulong steamId, int response) =>
                OnValidateAuthTicket?.Invoke(steamId, response)));
            singleton.Connect("OnGetTicketForWebApiResponse", Callable.From((int ticket, int result) =>
                OnGetTicketForWebApiResponse?.Invoke(ticket, (SteamResult)result)));

            singleton.Connect("OnLeaderboardFound", Callable.From((string name) =>
                OnLeaderboardFound?.Invoke(name)));

            singleton.Connect("OnSteamGameServerConnectFailure", Callable.From((int result, bool stillRetrying) =>
                OnSteamGameServerConnectFailure?.Invoke((SteamResult)result, stillRetrying)));
            singleton.Connect("OnSteamGameServersConnected", Callable.From(() =>
                OnSteamGameServersConnected?.Invoke()));
            singleton.Connect("OnSteamGameServersDisconnected", Callable.From((int result) =>
                OnSteamGameServersDisconnected?.Invoke((SteamResult)result)));
        }
    }
}
