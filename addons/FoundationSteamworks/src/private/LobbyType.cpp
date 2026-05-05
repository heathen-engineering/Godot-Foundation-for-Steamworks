#include "LobbyType.h"

void LobbyType::_bind_methods()
{
    BIND_ENUM_CONSTANT(Private);
    BIND_ENUM_CONSTANT(FriendsOnly);
    BIND_ENUM_CONSTANT(Public);
    BIND_ENUM_CONSTANT(Invisible);
}

void LobbyUseHint::_bind_methods()
{
    BIND_ENUM_CONSTANT(General);
    BIND_ENUM_CONSTANT(Session);
    BIND_ENUM_CONSTANT(Party);
}

void LobbyEnterResponse::_bind_methods()
{
    BIND_ENUM_CONSTANT(Success);
    BIND_ENUM_CONSTANT(DoesntExist);
    BIND_ENUM_CONSTANT(NotAllowed);
    BIND_ENUM_CONSTANT(Full);
    BIND_ENUM_CONSTANT(Error);
    BIND_ENUM_CONSTANT(Banned);
    BIND_ENUM_CONSTANT(Limited);
    BIND_ENUM_CONSTANT(ClanDisabled);
    BIND_ENUM_CONSTANT(CommunityBan);
    BIND_ENUM_CONSTANT(MemberBlockedYou);
    BIND_ENUM_CONSTANT(YouBlockedMember);
    BIND_ENUM_CONSTANT(RateLimitExceeded);
}
