@tool
extends EditorExportPlugin
class_name SteamGameExportPlugin

## Guards against SteamGame.gd shipping out of date with the current
## Steamworks settings — the Godot analog of Unity's SettingsBuildPreprocessor
## (IPreprocessBuildWithReport, which hard-fails the build via
## BuildFailedException if a SourceCode generator is stale).
##
## Godot's EditorExportPlugin has no equivalent "abort the whole export"
## signal — _export_begin() customizes what's included, it doesn't return a
## pass/fail verdict the exporter checks. Rather than pretend to hard-fail an
## export that would actually continue anyway, this instead auto-regenerates
## right before export starts (regeneration is fully deterministic and cheap —
## see SteamGameCodeGenerator), so a stale file is fixed in place instead of
## silently shipping outdated Achievement/Stat/Leaderboard names. A warning is
## still logged so the dev knows it happened and can review the diff.

func _get_name() -> String:
	return "SteamGameExportPlugin"

func _export_begin(_features: PackedStringArray, _is_debug: bool, _path: String, _flags: int) -> void:
	var settings := SteamSettingsIO.load_settings()
	var generated_dir: String
	if SteamGameCodeGenerator.is_stale(settings):
		push_warning("SteamGameExportPlugin: SteamGame.gd was stale at export time — regenerating from steam_settings.json before packaging.")
		generated_dir = SteamGameCodeGenerator.generate(settings)
	else:
		generated_dir = SteamGameCodeGenerator._find_existing_script().get_base_dir()
	SteamAutoloadSetup.ensure_autoload_setup(generated_dir.path_join(SteamGameCodeGenerator.LOADER_FILENAME))
