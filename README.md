# Foundation for Steamworks

![License](https://img.shields.io/badge/License-Apache_2.0-blue?style=flat-square)
![Maintained](https://img.shields.io/badge/Maintained%3F-yes-green?style=flat-square)
![Godot](https://img.shields.io/badge/Godot-4.6%20%2B-478CBF?style=flat-square&logo=godotengine&logoColor=white)
![Steamworks](https://img.shields.io/badge/Steamworks_SDK-1.63-lightgrey?style=flat-square)

A GDExtension that wraps the Steamworks SDK for Godot 4, exposing Steam features through a clean singleton API and strongly-typed data types. Fully functional as a standalone open-source addon. Forms the base layer for the commercial **Toolkit for Steamworks** extension.

-----

## Also Available For

[![Unity](https://img.shields.io/badge/Unity-6%20%2B-black?style=for-the-badge&logo=unity&logoColor=white)](https://github.com/heathen-engineering/Unity-Foundation-for-Steamworks)
[![O3DE](https://img.shields.io/badge/O3DE-25.10%20%2B-%2300AEEF?style=for-the-badge)](https://github.com/heathen-engineering/O3DE-Foundation-for-Steamworks)

-----

## Become a GitHub Sponsor

[![Discord](https://img.shields.io/badge/Discord--1877F2?style=social&logo=discord)](https://discord.gg/6X3xrRc)
[![GitHub followers](https://img.shields.io/github/followers/heathen-engineering?style=social)](https://github.com/heathen-engineering?tab=followers)

Support Heathen by becoming a [GitHub Sponsor](https://github.com/sponsors/heathen-engineering). Sponsorship directly funds development and maintenance of free tools like this, our game dev [Knowledge Base](https://heathen.group/), and our community on [Discord](https://discord.gg/6X3xrRc).

Sponsors get access to our private SourceRepo, which includes **Toolkit for Steamworks** — a commercial extension that adds a high-level `SteamTools` singleton with auto-initialisation, leaderboard management, lobby helpers, inventory, avatar loading, and game server browsing.

-----

## What It Does

Foundation provides a direct, strongly-typed interface to the Steamworks SDK from Godot 4 via GDExtension (C++). It is designed to be used from both **GDScript** and **C#**.

| System | Coverage |
|--------|----------|
| **App** | App ID, DLC enumeration, install info, beta names, launch parameters |
| **User** | Local user identity, persona name, avatar loading |
| **Friends** | Friend lists, clan/group membership, rich presence, overlays |
| **Stats & Achievements** | Read/write stats, unlock achievements, global percentages |
| **Leaderboards** | Find, upload scores, download global/friend/user entries |
| **Lobby** | Create, join, query, and manage lobbies with typed query filters |
| **Inventory** | Item definitions, grants, exchange, instance details |
| **Utilities** | IP helpers, Steam Deck detection, floating-point time |
| **Remote Play** | Session events, guest invite links |
| **Timeline** | Game phase, event markers, state tooltips (SDK 1.61+) |
| **Game Server** | Dedicated server initialisation, VAC, anonymous logon |
| **Matchmaking Servers** | Server list requests, ping, player and rules queries |

All Steam callback events are surfaced as Godot signals on the `SteamApi` node.

-----

## Requirements

- **Godot 4.6** or compatible
- A registered [Steamworks developer account](https://partner.steamgames.com/)
- **Steamworks SDK 1.63** — download from the Steamworks partner portal and place in `addons/FoundationSteamworks/include/sdk/`
- A C++ build environment (GCC/Clang + CMake) **only if building from source**

> Pre-built binaries for Linux x86_64 are included. Windows and macOS builds require compiling from source using the provided `CMakeLists.txt`.

-----

## Installation

### 1. Copy the addon

Copy the `addons/FoundationSteamworks/` folder into your Godot project's `addons/` directory.

### 2. Add the Steamworks SDK

Download the Steamworks SDK from the [Steamworks partner portal](https://partner.steamgames.com/). Place the SDK contents inside:

```
addons/FoundationSteamworks/include/sdk/
```

A `SteamSDKGoesHere.md` placeholder marks the correct location.

### 3. Enable the plugin

In Godot, open **Project → Project Settings → Plugins** and enable **Foundation for Steamworks**.

### 4. Add the AutoLoad scene

In **Project → Project Settings → AutoLoad**, add:

| Name | Path |
|------|------|
| `SteamApi` | `res://addons/FoundationSteamworks/FoundationAutoload.tscn` |

`SteamApi` must appear **before** any scene or node that accesses Steam.

### 5. Configure the node

Select the `SteamApi` AutoLoad entry and configure it in the Inspector:

| Property | Description |
|----------|-------------|
| **Debug** | Print verbose init steps to the Godot console |
| **AppId** | Your Steam App ID (default: `480` — Spacewar test app) |
| **AutoInitialise** | Initialise Steam automatically on `_ready` (default: `true`) |
| **LeaderboardIds** | Array of leaderboard names to pre-resolve at startup |

-----

## Building from Source

```bash
cd addons/FoundationSteamworks
cmake -B build -DGODOT_CPP_PATH=/path/to/godot-cpp -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The compiled library is written to `addons/FoundationSteamworks/bin/`.

The `CMakeLists.txt` exposes two cache variables you can override:

| Variable | Default | Purpose |
|----------|---------|---------|
| `GODOT_CPP_PATH` | `/home/loden/Dev/GitHub/Godot-cpp` | Path to your [godot-cpp](https://github.com/godotengine/godot-cpp) checkout |
| `STEAM_SDK_PATH` | `include/sdk/public` | Path to the Steamworks SDK `public/` folder |

-----

## Usage

### GDScript

```gdscript
func _ready():
    SteamApi.OnReady.connect(_on_steam_ready)

func _on_steam_ready():
    print("Logged in as: ", SteamApi.GetPersonaName())
    print("App ID: ", SteamApi.GetAppId())

# Achievements
SteamApi.SetAchievement("ACH_WIN_ONE_GAME")
SteamApi.StoreStats()

# Leaderboards (resolved at startup via LeaderboardIds property)
SteamApi.UploadLeaderboardScore("Top Scores", 12345, func(result, error):
    if not error:
        print("New rank: ", result.global_rank_new))

# Lobbies
SteamApi.CreateLobby(LobbyType.Public, 4, func(result, error):
    if not error:
        print("Lobby ID: ", result.lobby_id))
```

### C#

```csharp
using Godot;

public partial class MyNode : Node
{
    private GodotObject _api;

    public override void _Ready()
    {
        _api = Engine.GetSingleton("SteamApi");
        _api.Connect("OnReady", Callable.From(OnSteamReady));
    }

    private void OnSteamReady()
    {
        GD.Print("Steam ready: " + _api.Call("GetPersonaName"));
        _api.Call("SetAchievement", "ACH_WIN_ONE_GAME");
        _api.Call("StoreStats");
    }
}
```

> **Toolkit for Steamworks** (available to [GitHub Sponsors](https://github.com/sponsors/heathen-engineering)) adds a full strongly-typed C# wrapper layer built on top of Foundation.

-----

## AutoLoad Order When Used with Toolkit

If you install **Toolkit for Steamworks** alongside Foundation, load order and `AutoInitialise` settings are important:

| AutoLoad Name | Scene | AutoInitialise |
|---------------|-------|----------------|
| `SteamApi` | `FoundationAutoload.tscn` | **false** |
| `SteamTools` | `ToolkitAutoload.tscn` | **true** |

Foundation must be loaded first — it registers all base types and owns the Steam callback loop (`SteamAPI_RunCallbacks`). Toolkit owns initialisation when both are present, so set Foundation's `AutoInitialise` to `false` to avoid calling `SteamAPI_Init` twice.

When using **Foundation alone**, keep `AutoInitialise` set to `true`.

-----

## Key Signals

| Signal | Fires When |
|--------|------------|
| `OnReady` | Steam is fully initialised and leaderboard handles are resolved |
| `OnUserStatsReceived(gameId, result, user)` | Stats/achievements loaded for a user |
| `OnUserStatsStored(gameId, result)` | Stats committed to Steam servers |
| `OnUserAchievementStored(gameId, groupAchievements, name)` | Achievement unlocked |
| `OnAvatarImageLoaded(user, image, width, height)` | Avatar image ready |
| `OnLobbyCreated(result, lobbyId)` | Lobby creation complete |
| `OnLobbyEntered(lobby, chatPermissions, blocked, response)` | Joined a lobby |
| `OnLobbyChatUpdate(lobby, userChanged, userMaking, stateChange)` | Lobby membership changed |
| `OnLeaderboardFindResult(leaderboard, found)` | Leaderboard handle resolved |
| `OnLeaderboardScoresDownloaded(leaderboard, entries)` | Leaderboard entries ready |

See `src/public/SteamApi.h` for the complete signal list.

-----

## License

Apache 2.0 — see [LICENSE](LICENSE).
