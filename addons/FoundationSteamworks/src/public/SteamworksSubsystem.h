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

#pragma once

#include <gameframework/Subsystem.h>

/// <summary>
/// Global framework Subsystem exposing SteamApi's state to Godot-Game-
/// Framework's Subsystems tab and to other gems' depends_on() ordering.
///
/// Also owns the one thing SteamApi used to do for itself: pumping Steam's
/// callback queue every frame. SteamApi previously toggled Node::set_process()
/// on/off around its own readiness and drove SteamAPI_RunCallbacks()/
/// SteamGameServer_RunCallbacks() (plus queued leaderboard find/download
/// requests) from its own _process(double) override — now extracted into
/// SteamApi::pump_callbacks(double), called from tick() below instead, so
/// there's one tick mechanism for the whole framework (SubsystemTicker),
/// not SteamApi self-ticking in parallel with it. wants_tick() reports true
/// only once SteamApi::GetIsReady() is true — same "don't pump before
/// Initialise*() has completed" guard pump_callbacks() itself already had,
/// just checked once up front instead of on every call.
/// </summary>
class SteamworksSubsystem : public gameframework::Subsystem
{
public:
    std::string display_name() const override;
    std::vector<std::pair<std::string, std::string>> debug_info() const override;

    bool wants_tick() const override;
    void tick(double delta) override;

    /// Reads the "steamworks/start_mode" ProjectSettings entry (persisted
    /// by SubsystemManagerBridge::set_global_subsystem_start_mode(), wired
    /// from the Subsystems settings tab's dropdown — see
    /// FoundationSteamworksEditorPlugin.gd) — 0=Disabled, 1=OnDemand,
    /// 2=Automatic, defaulting to Automatic if unset. The one subsystem in
    /// this project that's genuinely useful to run OnDemand: a dedicated
    /// server build might not want Steam initialised at all until its own
    /// startup sequence decides to, and SteamApi::InitialiseClient()/
    /// InitialiseServer() are already public static entry points for
    /// exactly that manual call.
    gameframework::Subsystem::StartMode start_mode() const override;
};
