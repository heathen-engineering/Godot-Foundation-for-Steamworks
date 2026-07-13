@tool
extends EditorPlugin

## Activates the Steamworks settings page — handed to the unified Project
## Settings > Subsystems tab (Godot-Game-Framework) instead of building its
## own bottom-panel dock. Enable via Project Settings > Plugins (this addon's
## plugin.cfg now points here).
##
## Gated: FoundationSteamworks.gdextension (the native library everything
## here ultimately depends on — SteamworksSubsystem, the Subsystem
## integration) ships inert until Extension Resolver confirms
## Godot-Game-Framework is actually installed (at a satisfying version —
## real version-guarding, not just presence, since the migration off
## heathen_gate.gd). See gate/extension_resolver_gate.gd and
## Godot-GameplayTags-Foundation's GameplayTagsEditorPlugin.gd (the
## reference implementation this mirrors) for the full mechanism and why.
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

const Gate = preload("res://addons/FoundationSteamworks/gate/extension_resolver_gate.gd")

var _export_plugin: SteamGameExportPlugin
var _stale_dialog: AcceptDialog

func _enter_tree() -> void:
	if Gate.ensure_unlocked(self, "FoundationSteamworks", _activate_tooling):
		_activate_tooling()

# NOT named _build() — EditorPlugin already declares a virtual _build() -> bool
# (asks whether a custom pre-run build step should block "Run Project"); naming
# this the same collides with it and breaks script parsing entirely.
func _activate_tooling() -> void:
	var bridge = Engine.get_singleton("SubsystemManagerBridge")
	if bridge != null:
		bridge.register_settings_panel("Steamworks", Callable(self, "_build_settings_panel"))
		bridge.register_start_mode_setter("Steamworks", Callable(self, "_set_start_mode"))
		bridge.register_build("Steamworks", Callable(self, "_build_status"), Callable(self, "_do_build"))

	_export_plugin = SteamGameExportPlugin.new()
	add_export_plugin(_export_plugin)

func _build_settings_panel() -> Control:
	return SteamworksSettingsDock.new()

## Persists the project-wide Automatic/On Demand/Disabled choice —
## SteamworksSubsystem::start_mode() and SteamApi::_ready() both read this
## exact same ProjectSettings entry back, so the Subsystems tab's dropdown,
## the framework's boot bookkeeping, and Steam's actual real init decision
## can never disagree.
func _set_start_mode(mode: int) -> void:
	ProjectSettings.set_setting("steamworks/start_mode", mode)
	ProjectSettings.set_initial_value("steamworks/start_mode", 2)
	ProjectSettings.save()

## The "Build" concept — SteamGame.gd (the type-safe generated wrapper) can
## drift out of date with the Achievements/Stats/Leaderboards/DLC settings
## edited in the settings panel. 0=Good/green (up to date), 1=NeedsAttention/
## amber (stale). Mirrors _build()'s own staleness check below (that one
## gates the Run button; this is the same check surfaced as a clickable
## status button on the Subsystems tab row instead).
func _build_status() -> int:
	var settings := SteamSettingsIO.load_settings()
	return 1 if SteamGameCodeGenerator.is_stale(settings) else 0

func _do_build() -> void:
	var generated_dir := SteamGameCodeGenerator.generate(SteamSettingsIO.load_settings())
	SteamAutoloadSetup.ensure_autoload_setup(generated_dir.path_join(SteamGameCodeGenerator.LOADER_FILENAME))

func _exit_tree() -> void:
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
