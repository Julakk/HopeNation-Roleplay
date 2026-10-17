#if defined _HOPECITY_PLAYER
    #endinput
#endif
#define _HOPECITY_PLAYER

enum E_PLAYER_DATA
{
    Player_Name[MAX_PLAYER_NAME],
    Player_LoggedIn,
    Player_Spawned
};

new PlayerData[MAX_PLAYERS][E_PLAYER_DATA];

stock Player_Init()
{
    print("[PLAYER] Player system initialized.");
    return 1;
}

stock Player_OnConnect(playerid)
{
    GetPlayerName(playerid, PlayerData[playerid][Player_Name], MAX_PLAYER_NAME);

    PlayerData[playerid][Player_LoggedIn] = 0;
    PlayerData[playerid][Player_Spawned] = 0;

    SendClientMessage(playerid, 0x00FF00FF, "Selamat datang di HopeCity Roleplay!");
    SendClientMessage(playerid, 0xFFFFFFFF, "Development Build - Server sedang dalam tahap pengembangan.");

    return 1;
}

stock Player_OnSpawn(playerid)
{
    PlayerData[playerid][Player_Spawned] = 1;
    return 1;
}

stock Player_OnDisconnect(playerid, reason)
{
    #pragma unused reason

    PlayerData[playerid][Player_LoggedIn] = 0;
    PlayerData[playerid][Player_Spawned] = 0;

    return 1;
}
