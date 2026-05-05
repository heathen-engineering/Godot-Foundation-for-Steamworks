#include "SteamTimedTrial.h"

void SteamTimedTrial::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("IsActive"), &SteamTimedTrial::IsActive);
    ClassDB::bind_method(D_METHOD("GetAllowed"), &SteamTimedTrial::GetAllowed);
    ClassDB::bind_method(D_METHOD("GetPlayed"), &SteamTimedTrial::GetPlayed);
    ClassDB::bind_static_method("SteamTimedTrial", D_METHOD("GetSteamTimedTrial", "allowed", "played"), &SteamTimedTrial::GetSteamTimedTrial);
}

bool SteamTimedTrial::IsActive() const { return is_active; }
int SteamTimedTrial::GetAllowed() const { return allowed; }
int SteamTimedTrial::GetPlayed() const { return played; }

Ref<SteamTimedTrial> SteamTimedTrial::GetSteamTimedTrial(const int allowed, const int played)
{
    return Ref<SteamTimedTrial>(memnew(SteamTimedTrial(allowed, played)));
}
