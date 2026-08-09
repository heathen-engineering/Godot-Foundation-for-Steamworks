#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class LobbyType : public RefCounted {
    GDCLASS(LobbyType, RefCounted);

public:
    enum Type {
        Private = 0,
        FriendsOnly = 1,
        Public = 2,
        Invisible = 3,
    };

protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(LobbyType::Type);

class LobbyUseHint : public RefCounted {
    GDCLASS(LobbyUseHint, RefCounted);

public:
    enum Hint {
        General = 0,
        Session = 1,
        Party = 2,
    };

protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(LobbyUseHint::Hint);

class LobbyEnterResponse : public RefCounted {
    GDCLASS(LobbyEnterResponse, RefCounted);

public:
    enum EnterResponse {
        Success = 1,
        DoesntExist = 2,
        NotAllowed = 3,
        Full = 4,
        Error = 5,
        Banned = 6,
        Limited = 7,
        ClanDisabled = 8,
        CommunityBan = 9,
        MemberBlockedYou = 10,
        YouBlockedMember = 11,
        RateLimitExceeded = 12,
    };

protected:
    static void _bind_methods();
};

VARIANT_ENUM_CAST(LobbyEnterResponse::EnterResponse);
