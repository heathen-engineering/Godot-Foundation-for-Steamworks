@tool
extends RefCounted
class_name SteamSettingsIO

## Reads/writes res://steam_settings.json directly — no Resource round-trip,
## matching the plain-JSON convention already established by Ogham
## (OghamManifestIO) and Lexicon in this repo, rather than the Godot-only
## SteamToolSettings Resource this replaces. Plain JSON is what makes the
## settings portable to a future engine (or back to Unity) without needing a
## Godot-specific Resource loader to read them — the whole point the JSON
## migration exists to serve.
##
## Schema mirrors AppSettings — a cross-addon reference for shape only, not a
## dependency (addons/ToolkitSteamworks/src/public/SteamToolsSettings.h) —
## exactly, only main_app's fields (dock doesn't expose demo_app/playtest_apps/input
## yet, same scope as before this migration):
## {
##   "app_id": int,
##   "dlc": [int, ...],
##   "stats": [String, ...],
##   "achievements": [String, ...],
##   "leaderboards": [{"name": String}, ...]
## }

const SETTINGS_PATH := "res://steam_settings.json"

static func default_settings() -> Dictionary:
	return {
		"app_id": 0,
		"dlc": [],
		"stats": [],
		"achievements": [],
		"leaderboards": [],
	}

## Missing file -> default_settings(), same as OghamManifestIO.load_manifest's
## missing/unreadable-file behavior (empty/default rather than an error).
static func load_settings() -> Dictionary:
	var file := FileAccess.open(SETTINGS_PATH, FileAccess.READ)
	if file == null:
		return default_settings()
	var parsed = JSON.parse_string(file.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		return default_settings()
	var settings := default_settings()
	for key in settings.keys():
		if parsed.has(key):
			settings[key] = parsed[key]
	return settings

static func save_settings(settings: Dictionary) -> Error:
	var file := FileAccess.open(SETTINGS_PATH, FileAccess.WRITE)
	if file == null:
		return ERR_CANT_OPEN
	file.store_string(JSON.stringify(settings, "\t"))
	return OK
