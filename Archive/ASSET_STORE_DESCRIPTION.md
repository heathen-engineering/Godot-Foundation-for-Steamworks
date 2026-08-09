# Foundation for Steamworks

A direct, strongly-typed GDExtension wrapper for the Steamworks SDK, giving Godot 4 projects a clean singleton API and typed data classes for Steam. Fully functional as a standalone open-source addon, and the base layer for the commercial Toolkit for Steamworks extension.

Works from both GDScript and C#.

## What it does

A Steamworks settings dock (Project Settings, Plugins, enable Foundation for Steamworks) lets you configure your App ID, Achievements, Stats, Leaderboards, and DLC visually. A single "Generate Code" button then produces a type-safe SteamGame.gd wrapper and the AutoLoad that initializes it, so every Achievement or Stat name is a real, typo-checked field instead of a raw string.

Covers:

- App: App ID, DLC enumeration, install info, beta names, launch parameters
- User: local identity, persona name, avatar loading
- Friends: friend lists, clan and group membership, rich presence, overlays
- Stats and Achievements: read and write stats, unlock achievements, global percentages
- Leaderboards: find, upload scores, download global, friend, and user entries
- Lobby: create, join, query, and manage lobbies with typed query filters
- Inventory: item definitions, grants, exchange, instance details
- Utilities: IP helpers, Steam Deck detection, floating-point time
- Remote Play: session events, guest invite links
- Timeline: game phase, event markers, state tooltips (SDK 1.61+)
- Game Server: dedicated server initialization, VAC, anonymous logon
- Matchmaking Servers: server list requests, ping, player and rules queries

All Steam callback events are surfaced as Godot signals on the SteamApi node.

## Requirements

- Godot 4.6 or compatible
- A registered Steamworks developer account
- Steamworks SDK 1.63 (download separately from the Steamworks partner portal; not redistributed here)
- Pre-built binaries included for Windows, macOS, and Linux (Debug and Release). A C++ build environment is only needed if you want to build from source.
- Godot Game Framework, enabled in the consuming project. If it is missing, enabling this plugin walks you through fetching it automatically.

## Links

- GitHub: [https://github.com/heathen-engineering/Godot-Foundation-for-Steamworks](https://github.com/heathen-engineering/Godot-Foundation-for-Steamworks)
- Documentation: [https://heathen.group/kb/steam-welcome/](https://heathen.group/kb/steam-welcome/)
- Support and Discord: [https://discord.gg/xmtRNkW7hW](https://discord.gg/xmtRNkW7hW)
- License: Apache 2.0

## Want more?

Toolkit for Steamworks, available to GitHub Sponsors, extends Foundation with a high-level SteamTools singleton: auto-initialization, leaderboard management, lobby helpers, inventory, avatar loading, and game server browsing. Learn more at [https://heathen.group/kb/do-more/](https://heathen.group/kb/do-more/)
