@tool
extends Control
class_name SteamworksSettingsDock

## Bottom-panel dock editing Steamworks settings (App ID, achievements, stats,
## leaderboards, DLC). The Godot analog of Unity's SteamToolsSettingsProvider
## (Project Settings > Subsystems > Steamworks). Persisted at
## res://steam_settings.json via SteamSettingsIO — plain JSON, not a Resource
## (see SteamSettingsIO's doc comment for why), read/written directly the same
## way OghamManifestIO handles .ogham files.
##
## Demo/playtest apps (main_app vs. demo_app/playtest_apps in the old
## SteamToolSettings Resource) still aren't exposed here — this dock only ever
## covered a single real App ID for TheBarrow; extend it if/when a demo or
## playtest build actually needs configuring.
##
## Every edit also regenerates SteamGame.gd immediately (see
## SteamGameCodeGenerator) — safe to do eagerly here since re-parsing a changed
## GDScript is a cheap, near-instant operation in Godot, unlike the full C#
## domain reload Unity's own auto-regen-on-edit had to be designed around.

var _settings: Dictionary
var _app_id_spin: SpinBox
var _achievements_list: ItemList
var _stats_list: ItemList
var _leaderboards_list: ItemList
var _dlc_list: ItemList
var _staleness_label: Label

func _ready() -> void:
	name = "Steamworks"
	# See GameplayTagsDock._ready()'s doc comment — the bottom panel sizes its
	# tabs via size_flags, not anchors, so this control itself (not just the
	# inner root_vbox below) needs expand-fill or it renders at ~0 height.
	size_flags_horizontal = Control.SIZE_EXPAND_FILL
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	_settings = SteamSettingsIO.load_settings()

	var root_vbox := VBoxContainer.new()
	root_vbox.set_anchors_preset(Control.PRESET_FULL_RECT)
	add_child(root_vbox)

	var toolbar := HBoxContainer.new()
	var generate_btn := Button.new()
	generate_btn.text = "Generate Code"
	generate_btn.tooltip_text = "Regenerate SteamGame.gd from the current settings (also happens automatically on every edit here)."
	generate_btn.pressed.connect(_on_generate_pressed)
	toolbar.add_child(generate_btn)
	_staleness_label = Label.new()
	toolbar.add_child(_staleness_label)
	root_vbox.add_child(toolbar)
	_refresh_staleness_label()

	var app_id_row := HBoxContainer.new()
	var app_id_label := Label.new()
	app_id_label.text = "App ID:"
	app_id_row.add_child(app_id_label)
	_app_id_spin = SpinBox.new()
	_app_id_spin.min_value = 0
	_app_id_spin.max_value = 4294967295
	_app_id_spin.custom_minimum_size = Vector2(140, 0)
	_app_id_spin.value = _settings.get("app_id", 0)
	_app_id_spin.value_changed.connect(_on_app_id_changed)
	app_id_row.add_child(_app_id_spin)
	root_vbox.add_child(app_id_row)

	var tabs := TabContainer.new()
	tabs.size_flags_vertical = Control.SIZE_EXPAND_FILL
	root_vbox.add_child(tabs)

	_achievements_list = _make_string_list_tab(tabs, "Achievements", _settings.get("achievements", []))
	_stats_list = _make_string_list_tab(tabs, "Stats", _settings.get("stats", []))
	_dlc_list = _make_string_list_tab(tabs, "DLC (App IDs)", _dlc_to_strings(_settings.get("dlc", [])))

	var leaderboard_names: Array = []
	for lb in _settings.get("leaderboards", []):
		if typeof(lb) == TYPE_DICTIONARY and lb.has("name"):
			leaderboard_names.append(lb["name"])
	_leaderboards_list = _make_string_list_tab(tabs, "Leaderboards", leaderboard_names)

func _make_string_list_tab(tabs: TabContainer, tab_name: String, initial: Array) -> ItemList:
	var vbox := VBoxContainer.new()
	vbox.name = tab_name
	tabs.add_child(vbox)

	var list := ItemList.new()
	list.size_flags_vertical = Control.SIZE_EXPAND_FILL
	for entry in initial:
		list.add_item(str(entry))
	vbox.add_child(list)

	var add_row := HBoxContainer.new()
	var edit := LineEdit.new()
	edit.placeholder_text = "New entry..."
	edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	edit.text_submitted.connect(func(_t): _add_entry(list, edit, tab_name))
	add_row.add_child(edit)
	var add_btn := Button.new()
	add_btn.text = "Add"
	add_btn.pressed.connect(func(): _add_entry(list, edit, tab_name))
	add_row.add_child(add_btn)
	var delete_btn := Button.new()
	delete_btn.text = "Delete Selected"
	delete_btn.pressed.connect(func(): _delete_selected(list, tab_name))
	add_row.add_child(delete_btn)
	vbox.add_child(add_row)

	return list

func _add_entry(list: ItemList, edit: LineEdit, tab_name: String) -> void:
	var value := edit.text.strip_edges()
	if value.is_empty():
		return
	list.add_item(value)
	edit.text = ""
	_sync_list_to_settings(list, tab_name)

func _delete_selected(list: ItemList, tab_name: String) -> void:
	var selected := list.get_selected_items()
	for i in range(selected.size() - 1, -1, -1):
		list.remove_item(selected[i])
	_sync_list_to_settings(list, tab_name)

func _sync_list_to_settings(list: ItemList, tab_name: String) -> void:
	var values: Array = []
	for i in list.item_count:
		values.append(list.get_item_text(i))

	match tab_name:
		"Achievements":
			_settings["achievements"] = values
		"Stats":
			_settings["stats"] = values
		"DLC (App IDs)":
			var ints: Array = []
			for v in values:
				ints.append(int(v))
			_settings["dlc"] = ints
		"Leaderboards":
			var leaderboards: Array = []
			for v in values:
				leaderboards.append({"name": v})
			_settings["leaderboards"] = leaderboards

	_save_settings()

func _dlc_to_strings(dlc: Array) -> Array:
	var result: Array = []
	for d in dlc:
		result.append(str(d))
	return result

func _on_app_id_changed(value: float) -> void:
	_settings["app_id"] = int(value)
	_save_settings()

func _on_generate_pressed() -> void:
	_generate_code()

## Everything "Generate Code" hands the dev for free, beyond just the
## SteamGame.gd wrapper itself: also makes sure the SteamGame autoload scene
## exists and is registered in Project Settings (see SteamAutoloadSetup) —
## the goal being that a dev never has to manually wire up the autoload
## themselves, same as they never have to hand-type an achievement's API name.
func _generate_code() -> void:
	var generated_dir := SteamGameCodeGenerator.generate(_settings)
	var loader_path := generated_dir.path_join(SteamGameCodeGenerator.LOADER_FILENAME)
	SteamAutoloadSetup.ensure_autoload_setup(loader_path)
	_refresh_staleness_label()

func _refresh_staleness_label() -> void:
	if SteamGameCodeGenerator.is_stale(_settings):
		_staleness_label.text = "  Generated SteamGame.gd is stale — click Generate Code."
		_staleness_label.add_theme_color_override("font_color", Color(0.9, 0.7, 0.2))
	else:
		_staleness_label.text = "  Generated SteamGame.gd is up to date."
		_staleness_label.add_theme_color_override("font_color", Color(0.5, 0.8, 0.5))

func _save_settings() -> void:
	var err := SteamSettingsIO.save_settings(_settings)
	if err != OK:
		push_warning("SteamworksSettingsDock: failed to save %s (error %d)" % [SteamSettingsIO.SETTINGS_PATH, err])
	_generate_code()
