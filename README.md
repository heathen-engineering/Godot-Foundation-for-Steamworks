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

For GDScript projects, a **Steamworks settings dock** (Project Settings → Plugins → enable Foundation for Steamworks) lets you configure your App ID, Achievements, Stats, Leaderboards, and DLC visually, then generates a type-safe `SteamGame.gd` wrapper plus the AutoLoad that initialises it — see [Installation](#installation) and [Usage](#usage).

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

## Foundation vs. Toolkit — what's owned where

Mirrors [Unity Foundation for Steamworks](https://github.com/heathen-engineering/Unity-Foundation-for-Steamworks)' split with [Toolkit for Steamworks](https://github.com/heathen-engineering/Unity-Toolkit-for-Steamworks) exactly, including the namespace: both packages share `Heathen.SteamworksIntegration`/`.API` — the split is by package, not by namespace, so nothing needs renaming if you later add Toolkit.

- **User, Achievements, Stats, Leaderboards, App/Utilities** — Foundation owns these **fully**: the data types *and* every operation. Usable standalone; nothing about them requires Toolkit.
- **Everything else** (Lobby/Matchmaking, Friends beyond the basics, Inventory, UGC/Workshop, Input, Overlay, Parties, RemoteStorage, RemotePlay, Screenshots, Voice, Timeline, Clans) — Foundation owns the **data types and native SDK plumbing only** (`LobbyData`, `ItemData`, `ClanData`, the `SteamApi` singleton's native methods for these sections). The *convenience* operational surface — `LobbyDataExtensions`, `API.Matchmaking`, and friends — is Toolkit's to add, matching Unity's `XxxDataExtensions` pattern exactly. You can still call these sections directly off `SteamApi`/`SteamTools.cs` today; Toolkit exists to make that ergonomic (typed extension methods, workflow components), not to unlock capability Foundation lacks.

The native `SteamApi` GDExtension plays the role Steamworks.NET plays in the Unity stack: a complete low-level SDK wrap that both tiers sit on top of. There's no native/C++ split between Foundation and Toolkit — Toolkit (when present) calls the same compiled `SteamApi` singleton Foundation ships, via the same `Engine.GetSingleton("SteamApi")` + `Variant.Call` boundary Foundation's own C# facade already uses, so Toolkit never needs its own copy of the Steamworks SDK or a C++ build step.

-----

## Requirements

- **Godot 4.6** or compatible
- A registered [Steamworks developer account](https://partner.steamgames.com/)
- **Steamworks SDK 1.63** — download from the Steamworks partner portal and place in `addons/FoundationSteamworks/include/sdk/`
- A C++ build environment (GCC/Clang + CMake) **only if building from source**

> Pre-built binaries for **Windows, macOS, and Linux** (Debug and Release) are included — most developers never need to compile anything. Full source is included too, if you want to rebuild from scratch, audit the code, or modify it, using the provided `CMakeLists.txt`.

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

In Godot, open **Project → Project Settings → Plugins** and enable **Foundation for Steamworks**. This activates a **Steamworks** bottom-panel dock — the recommended way to configure the rest of this section.

### 4. Configure your App ID/Achievements/Stats/Leaderboards/DLC

Open the **Steamworks** dock (bottom panel). Set your App ID, and add your Achievement API names, Stat API names, Leaderboard names, and DLC App IDs — whatever your game actually uses. This is stored in `res://steam_settings.json`, plain JSON so it stays readable/diffable/portable, not a Godot-only Resource.

<img width="2560" height="1440" alt="image" src="https://github.com/user-attachments/assets/d07f579c-0336-4b1e-87a5-e1ae124fc490" />


### 5. Click "Generate Code"

The dock's **Generate Code** button (top toolbar) does everything from here automatically:

- Writes `SteamGame.gd` — a type-safe wrapper over exactly what you configured (see [Usage](#usage) below).
- Writes `SteamGameLoader.gd` next to it — a tiny bootstrap script.
- Creates `SteamGameAutoload.tscn` (if it doesn't already exist) with that script attached, and registers it as the `SteamGameLoader` AutoLoad in Project Settings automatically — **you never manually add an AutoLoad entry yourself**.

Regeneration also happens automatically every time you edit something in the dock, and Godot warns you (blocking Run with a one-click fix) if `SteamGame.gd` is ever out of sync with your settings — you shouldn't normally need to click the button more than once.

`SteamGameLoader.gd`'s node (the actual AutoLoad, visible in **Project Settings → AutoLoad** as `SteamGameLoader`) exposes three properties you can tweak directly in the Inspector:

| Property | Description |
|----------|-------------|
| `debug` | Print verbose init steps to the Godot console |
| `auto_logon` | Log on to Steam automatically |
| `auto_initialise` | Initialise Steam automatically on `_ready` (turn off if you want to gate Steam init behind, say, a menu button instead) |

Your App ID lives in `steam_settings.json` (edited via the dock), not as an Inspector property — one source of truth, not two things that can drift out of sync.

> **Manual/advanced alternative**: if you'd rather not use the dock/codegen at all, `FoundationAutoload.tscn` (the bare `SteamApi` node, with `AppId`/`Debug`/`AutoInitialise`/`AutoLogOn` as direct Inspector properties) is still there and still works — add it as an AutoLoad yourself the same way older versions of this README described. Nothing about the generated flow requires it; they're two independent ways to get `SteamApi` running.

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

> **Toolkit for Steamworks** — available to [GitHub Sponsors](https://github.com/sponsors/heathen-engineering) — extends Foundation's C# layer with strongly-typed `XxxDataExtensions` and `API.*` wrappers covering the ergonomic surface of the **full Steamworks SDK**: Lobbies, Inventory, Friends, Remote Play, Timeline, and more. Toolkit has no native SDK layer of its own — every call it makes goes through Foundation's `SteamApi` singleton, the same one your own code talks to. All Toolkit wrappers follow the same `UserData`, `StatData`, `AchievementData`, and `LeaderboardData` patterns found in Foundation.

### GDScript (recommended: generated `SteamGame` wrapper)

Once you've configured Achievements/Stats/Leaderboards in the dock and clicked **Generate Code** (see [Installation](#installation)), every entry is a real, typo-checked field — Godot's own script parser flags a mistyped name as a parse error, not a runtime string that silently does nothing:

```gdscript
# Achievements
SteamGame.Achievements.ACH_WIN_ONE_GAME.Unlock()
SteamGame.Achievements.ACH_WIN_ONE_GAME.Store()

# Stats
SteamGame.Stats.NumWins.SetIntValue(SteamGame.Stats.NumWins.GetIntValue() + 1)

# Leaderboards (download helpers only — score upload isn't ported to this
# GDExtension yet, see LeaderboardData.h)
SteamGame.Leaderboards.TopScores.DownloadGlobalEntries(1, 10, 0, func(entries):
    print("Top 10: ", entries))
```

No `SteamApi.OnReady` wiring needed either — the generated `SteamGameLoader` AutoLoad already calls `SteamGame.Initialise()` for you on `_ready()`.

### GDScript (direct `SteamApi`, no codegen)

Still fully supported if you're not using the dock/codegen — the same raw calls the generated wrapper itself is built on:

```gdscript
func _ready():
    SteamApi.OnReady.connect(_on_steam_ready)

func _on_steam_ready():
    print("Logged in as: ", SteamApi.GetPersonaName())

# Achievements
SteamApi.SetAchievement("ACH_WIN_ONE_GAME")
SteamApi.StoreStats()

# Leaderboards (resolved at startup via LeaderboardIds property)
SteamApi.UploadLeaderboardScore("Top Scores", 12345, func(result, error):
    if not error:
        print("New rank: ", result.global_rank_new))
```

### C#

```csharp
using Godot;
using Heathen.SteamworksIntegration;

public partial class MyNode : Node
{
    public override void _Ready()
    {
        SteamToolsInterface.WhenReady(OnSteamReady);
    }

    private void OnSteamReady()
    {
        GD.Print("Logged in as: " + UserData.Me.UserName);

        // Achievements
        var ach = AchievementData.Get("ACH_WIN_ONE_GAME");
        ach.IsAchieved = true;
        ach.Store();

        // Leaderboards (resolved at startup via LeaderboardIds property)
        var board = LeaderboardData.Get("Top Scores");
        board.UploadScore(12345, (entry, error) =>
        {
            if (!error)
                GD.Print("New rank: " + entry.Rank);
        });
    }
}
```

-----

## AutoLoad Order When Used with Toolkit

Only one AutoLoad is needed, whether or not Toolkit is installed — but which one depends on whether you're using the dock/codegen (recommended) or the manual node:

| Setup | AutoLoad Name | Scene | Owns `SteamApi`? |
|-------|---------------|-------|-------------------|
| Dock + Generate Code (recommended) | `SteamGameLoader` | `SteamGameAutoload.tscn` | Created as a runtime child by `SteamGame.Initialise()` — not the AutoLoad's own root |
| Manual (no codegen) | `SteamApi` | `FoundationAutoload.tscn` | Is the AutoLoad's own root |

Either way, Foundation is the sole owner of the Steam callback loop (`SteamAPI_Init`/`SteamAPI_RunCallbacks`) and all base types — the generated path just creates the same `SteamApi` node as a child instead of as the AutoLoad root itself, so it isn't something you need to see or configure directly in the AutoLoad list. Toolkit has no native singleton of its own and no init lifecycle to arbitrate — its C# `XxxDataExtensions`/`API.*` classes call straight into this same `SteamApi` singleton (wherever it lives in the tree), so there's nothing extra to configure when adding Toolkit to a project.

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
