#if defined _HOPECITY_PLAYER
    #endinput
#endif
#define _HOPECITY_PLAYER

stock Player_Init()
{
    print("[PLAYER] Player system initialized.");
    return 1;
}

stock Player_OnConnect(playerid)
{
    SendClientMessage(playerid, 0x00FF00FF, "Selamat datang di HopeCity Roleplay!");
    SendClientMessage(playerid, 0xFFFFFFFF, "Development Build - Server sedang dalam tahap pengembangan.");
    return 1;
}

stock Player_OnSpawn(playerid)
{
    #pragma unused playerid
    return 1;
}

stock Player_OnDisconnect(playerid, reason)
{
    #pragma unused playerid
    #pragma unused reason
    return 1;
}
