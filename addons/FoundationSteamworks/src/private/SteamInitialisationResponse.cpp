#include "SteamInitialisationResponse.h"

void SteamInitialisationResponse::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("GetSuccess"), &SteamInitialisationResponse::GetSuccess);
    ClassDB::bind_method(D_METHOD("SetSuccess", "v"), &SteamInitialisationResponse::SetSuccess);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "Success"), "SetSuccess", "GetSuccess");

    ClassDB::bind_method(D_METHOD("GetShouldRestart"), &SteamInitialisationResponse::GetShouldRestart);
    ClassDB::bind_method(D_METHOD("SetShouldRestart", "v"), &SteamInitialisationResponse::SetShouldRestart);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ShouldRestart"), "SetShouldRestart", "GetShouldRestart");

    ClassDB::bind_method(D_METHOD("GetMessage"), &SteamInitialisationResponse::GetMessage);
    ClassDB::bind_method(D_METHOD("SetMessage", "v"), &SteamInitialisationResponse::SetMessage);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "Message"), "SetMessage", "GetMessage");
}
