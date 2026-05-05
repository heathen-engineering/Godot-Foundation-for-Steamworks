#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class SteamTimedTrial : public RefCounted
{
    GDCLASS(SteamTimedTrial, RefCounted);

protected:
    static void _bind_methods();
    bool is_active = false;
    int allowed = 0;
    int played = 0;

public:
    SteamTimedTrial() = default;
    SteamTimedTrial(const int allowed, const int played) : is_active(true), allowed(allowed), played(played) {}

    bool IsActive() const;
    int GetAllowed() const;
    int GetPlayed() const;

    static Ref<SteamTimedTrial> GetSteamTimedTrial(const int allowed, const int played);
};
