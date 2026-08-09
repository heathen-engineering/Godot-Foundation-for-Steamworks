@tool
extends RefCounted
class_name SteamAutoloadSetup

## Ensures the SteamGameLoader autoload scene + Project Settings registration
## exist — called from "Generate Code" (see SteamworksSettingsDock) so a dev
## never has to manually create the loader scene or wire up the autoload
## themselves.
##
## The autoload's root is a plain Node running the GENERATED SteamGameLoader.gd
## (see SteamGameCodeGenerator) — not the native SteamApi class directly. That
## class still exists and still does real work (its _process() pumps
## SteamAPI_RunCallbacks() every frame, which needs a live tree node), it's
## just created as a runtime child by SteamGame.Initialise() instead of being
## the thing a dev sees sitting in the Autoload list.
##
## The autoload is registered under the key "SteamGameLoader", NOT "SteamGame"
## — confirmed the hard way (Godot refused to parse SteamGame.gd at all:
## "Class SteamGame hides an autoload singleton") that an autoload's
## project.godot key becomes a global singleton identifier, which collides
## with class_name SteamGame on the generated static wrapper the exact same
## way "SteamApi" the old autoload name once collided with the native SteamApi
## class (see the now-retired SteamSettingsLoader.gd's doc comment for that
## first occurrence of this exact mistake). The autoload's own name is never
## referenced directly by dev code (only SteamGame.Achievements.Foo.Unlock()
## etc. is), so it can safely be anything that doesn't collide.
##
## Idempotent and non-destructive: if the scene already exists, it's left
## alone entirely (a dev may have customized it) — only the Project Settings
## autoload entry is checked/repaired, in case that got lost independently of
## the scene file itself.

const SCENE_PATH := "res://SteamGameAutoload.tscn"
const AUTOLOAD_SETTING := "autoload/SteamGameLoader"
## The old architecture's autoload — removed here if still present, so a
## project doesn't end up with both the old SteamApi-rooted autoload and the
## new one registered at once.
const OLD_AUTOLOAD_SETTING := "autoload/SteamApi"

## 'loader_script_path' is wherever SteamGameCodeGenerator.generate() actually
## wrote SteamGameLoader.gd this run (next to SteamGame.gd, which may have
## been relocated by the dev — see its own marker-search doc comment).
static func ensure_autoload_setup(loader_script_path: String) -> void:
	var needs_save := false

	if ProjectSettings.has_setting(OLD_AUTOLOAD_SETTING):
		ProjectSettings.set_setting(OLD_AUTOLOAD_SETTING, null)
		needs_save = true

	if not ResourceLoader.exists(SCENE_PATH):
		_create_autoload_scene(loader_script_path)

	var desired := "*%s" % SCENE_PATH
	if ProjectSettings.get_setting(AUTOLOAD_SETTING, "") != desired:
		ProjectSettings.set_setting(AUTOLOAD_SETTING, desired)
		needs_save = true

	# Only touch project.godot on disk when something actually changed — this
	# runs on every dock edit (see SteamworksSettingsDock's auto-regen), so
	# rewriting it unconditionally would churn a shared, version-controlled
	# file on every single achievement/stat/leaderboard tweak.
	if needs_save:
		ProjectSettings.save()

## Builds the scene via live Godot APIs (PackedScene.pack) rather than
## hand-writing .tscn text — avoids getting UID/ext_resource formatting subtly
## wrong, and stays correct automatically if Godot's .tscn format ever changes.
static func _create_autoload_scene(loader_script_path: String) -> void:
	var root := Node.new()
	root.name = "SteamGameLoader"
	root.set_script(load(loader_script_path))
	var packed := PackedScene.new()
	packed.pack(root)
	var err := ResourceSaver.save(packed, SCENE_PATH)
	if err != OK:
		push_warning("SteamAutoloadSetup: failed to save %s (error %d)" % [SCENE_PATH, err])
	root.free()
