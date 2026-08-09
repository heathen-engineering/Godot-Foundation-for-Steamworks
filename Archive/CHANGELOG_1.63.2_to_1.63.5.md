# Changelog: v1.63.2 to v1.63.5

## Highlights

- Pre-built binaries are now included for **Windows, macOS, and Linux** (Debug and Release). Previously only Linux binaries were provided, so most developers no longer need to compile anything to use the plugin.
- Automatic dependency management: the plugin now depends on [Godot Game Framework](https://github.com/heathen-engineering/Godot-Game-Framework) and is resolved through [Extension Resolver for Godot](https://github.com/heathen-engineering/Godot-Extension-Resolver). If a dependency is missing, enabling the plugin walks you through fetching it automatically instead of failing silently.
- Steam's status is now visible in the Subsystems dock, alongside every other Foundation-tier system, instead of being managed separately.

## Fixes

- **Start Mode "Disabled" now actually prevents Steam from initializing.** Previously, setting Start Mode to Disabled only stopped the Subsystem from ticking, but `SteamAPI_Init()` still ran regardless, leaving Steam initialized with its callback queue never pumped.
- **Leaderboard score upload is now callable from GDScript.** `SteamGame.Leaderboards.<YourBoard>.UploadScore(score, callback)` and `UploadScoreWithDetails(...)` are now bound and working. The underlying Steamworks calls were already implemented; they just were not exposed on the generated leaderboard wrapper.
- **Fixed a crash on editor boot** affecting certain builds, caused by hex-ID formatting in `UserData` and `LobbyData`.

## Removed

- Removed the DLC settings tab from the Foundation settings dock. It captured data that nothing in the plugin ever read back. Live DLC data (installed/owned status, download progress) is still fully available through `SteamApi`/`DlcData`, populated directly from Steam, and is unaffected by this change.

## Other

- Publisher metadata standardized, and documentation/support links updated to point at the Heathen Group Knowledge Base and Discord.
