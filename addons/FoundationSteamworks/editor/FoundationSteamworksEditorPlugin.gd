@tool
extends EditorPlugin

## Activates the Steamworks settings dock. Enable via Project Settings > Plugins
## (this addon's plugin.cfg now points here).
##
## Lives in Foundation, not Toolkit: Achievements/Stats/Leaderboards/App(DLC) —
## everything this dock and the generated SteamGame.gd cover — are fully
## Foundation-owned per Foundation's own plugin.cfg description ("Foundation
## fully owns User, Achievements, Stats, Leaderboards, and App/Utilities").
## A dev using Foundation standalone, with no Toolkit installed at all, should
## still get the type-safe generated wrapper for the parts Foundation owns —
## that's the whole reason this moved out of ToolkitSteamworks (which is now
## just an empty placeholder plugin; see its own doc comment). Toolkit, when
## present, is expected to extend the SAME generated file with its own
## sections (DLC extensions, Input, Inventory, ...) rather than own a
## competing generator.
##
## Also gates the Run button on SteamGame.gd being up to date with the current
## Steamworks settings — the Godot analog of Unity's SettingsPlayModeGuard
## (playModeStateChanged, cancelling ExitingEditMode with a Build & Play/
## Cancel/Play Anyway dialog). Godot's _build() is a single synchronous
## true/false decision with no equivalent to Unity's "pause the transition,
## resume after an async dialog answers" — so unlike Unity, this can't offer a
## true "Run Anyway" (that would require _build() to already have returned
## true before the dev's choice is known). Instead: not stale -> Play proceeds
## silently (the common case); stale -> Play is blocked for this attempt and a
## dialog with a one-click "Generate Code" button appears, after which
## pressing Run again immediately succeeds.

var _dock: SteamworksSettingsDock
var _export_plugin: SteamGameExportPlugin
var _stale_dialog: AcceptDialog

func _enter_tree() -> void:
	_dock = SteamworksSettingsDock.new()
	add_control_to_bottom_panel(_dock, "Steamworks")
	_export_plugin = SteamGameExportPlugin.new()
	add_export_plugin(_export_plugin)

func _exit_tree() -> void:
	if _dock != null:
		remove_control_from_bottom_panel(_dock)
		_dock.queue_free()
		_dock = null
	if _export_plugin != null:
		remove_export_plugin(_export_plugin)
		_export_plugin = null
	if _stale_dialog != null:
		_stale_dialog.queue_free()
		_stale_dialog = null

func _build() -> bool:
	var settings := SteamSettingsIO.load_settings()
	if not SteamGameCodeGenerator.is_stale(settings):
		return true

	if _stale_dialog == null:
		_stale_dialog = AcceptDialog.new()
		_stale_dialog.title = "Steamworks: generated code is stale"
		_stale_dialog.add_button("Generate Code", true, "generate")
		_stale_dialog.custom_action.connect(func(action):
			if action == "generate":
				var generated_dir := SteamGameCodeGenerator.generate(SteamSettingsIO.load_settings())
				SteamAutoloadSetup.ensure_autoload_setup(generated_dir.path_join(SteamGameCodeGenerator.LOADER_FILENAME))
			_stale_dialog.hide())
		get_editor_interface().get_base_control().add_child(_stale_dialog)
	_stale_dialog.dialog_text = "SteamGame.gd doesn't match the current Steamworks settings (Achievements/Stats/Leaderboards). Generate it, then press Run again."
	_stale_dialog.popup_centered()
	return false
