/*
 * Copyright (c) 2026 Heathen Engineering Limited
 * Irish Registered Company #556277
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SteamworksSubsystem.h"

#include "SteamApi.h"

#include <godot_cpp/classes/project_settings.hpp>

std::string SteamworksSubsystem::display_name() const
{
    return "Steamworks";
}

std::vector<std::pair<std::string, std::string>> SteamworksSubsystem::debug_info() const
{
    std::vector<std::pair<std::string, std::string>> result;

    SteamApi *api = SteamApi::GetSingleton();
    if (api == nullptr)
    {
        result.emplace_back("SteamApi", "not constructed");
        return result;
    }

    result.emplace_back("Ready", SteamApi::GetIsReady() ? "true" : "false");
    return result;
}

bool SteamworksSubsystem::wants_tick() const
{
    return SteamApi::GetIsReady();
}

void SteamworksSubsystem::tick(double delta)
{
    SteamApi *api = SteamApi::GetSingleton();
    if (api != nullptr)
        api->pump_callbacks(delta);
}

gameframework::Subsystem::StartMode SteamworksSubsystem::start_mode() const
{
    using godot::ProjectSettings;
    using godot::String;

    // Function-local static, not file-scope — godot::String's constructor
    // calls into the GDExtension API, which isn't bound yet during this
    // shared library's static-initialization pass at dlopen() time. See
    // OghamKeyLabelsNative.cpp's identical comment for the confirmed
    // dlopen-time segfault this avoids.
    static const String setting_path = "steamworks/start_mode";

    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps == nullptr || !ps->has_setting(setting_path))
        return gameframework::Subsystem::StartMode::Automatic;

    int mode = int(ps->get_setting(setting_path));
    if (mode < 0 || mode > 2)
        return gameframework::Subsystem::StartMode::Automatic;
    return gameframework::Subsystem::StartMode(mode);
}
