#if defined _HOPECITY_PLAYER
    #endinput
#endif
#define _HOPECITY_PLAYER

enum E_PLAYER_DATA
{
    Player_Name[MAX_PLAYER_NAME],
    Player_LoggedIn,
    Player_Spawned,
    Player_Money,
    Player_Skin,
    Player_Interior,
    Float:Player_Health,
    Float:Player_Armor,
    Float:Player_PosX,
    Float:Player_PosY,
    Float:Player_PosZ,
    Float:Player_Angle
};

new PlayerData[MAX_PLAYERS][E_PLAYER_DATA];

stock Player_Init()
{
    print("[PLAYER] Player system initialized.");
    return 1;
}

stock Player_SetDefaults(playerid)
{
    PlayerData[playerid][Player_Money] = 500;
    PlayerData[playerid][Player_Skin] = 0;
    PlayerData[playerid][Player_Interior] = 0;
    PlayerData[playerid][Player_Health] = 100.0;
    PlayerData[playerid][Player_Armor] = 0.0;
    PlayerData[playerid][Player_PosX] = 1481.0;
    PlayerData[playerid][Player_PosY] = -1771.0;
    PlayerData[playerid][Player_PosZ] = 18.8;
    PlayerData[playerid][Player_Angle] = 90.0;
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
    if (!PlayerData[playerid][Player_Spawned] && PlayerData[playerid][Player_LoggedIn])
    {
        SetPlayerInterior(playerid, PlayerData[playerid][Player_Interior]);
        SetPlayerHealth(playerid, PlayerData[playerid][Player_Health]);
        SetPlayerArmour(playerid, PlayerData[playerid][Player_Armor]);
        ResetPlayerMoney(playerid);
        GivePlayerMoney(playerid, PlayerData[playerid][Player_Money]);
    }

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
