#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class SteamInitialisationResponse : public RefCounted {
    GDCLASS(SteamInitialisationResponse, RefCounted);

public:
    bool bSuccess = false;
    bool bShouldRestart = false;
    String message;

protected:
    static void _bind_methods();

public:
    bool GetSuccess() const { return bSuccess; }
    void SetSuccess(bool v) { bSuccess = v; }
    bool GetShouldRestart() const { return bShouldRestart; }
    void SetShouldRestart(bool v) { bShouldRestart = v; }
    String GetMessage() const { return message; }
    void SetMessage(const String &v) { message = v; }
};
