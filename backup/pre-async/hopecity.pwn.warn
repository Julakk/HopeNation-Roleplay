#pragma rational Float

#include <a_samp>

#include "modules/core.pwn"
#include "modules/player.pwn"
#include "modules/commands.pwn"
#include "modules/utils.pwn"
#include "modules/database.pwn"
#include "modules/auth.pwn"

main()
{
    print("====================================");
    print("      HOPECITY ROLEPLAY");
    print("      Development Build");
    print("====================================");
}

forward Player_AutoSave();
public Player_AutoSave()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i))
        {
            Database_SavePlayer(i);
        }
    }

    return 1;
}

public OnGameModeInit()
{
    Core_Init();
    Player_Init();
    Commands_Init();
    Auth_Init();
    Database_Init();
    SetTimer("Player_AutoSave", 300000, true);

    AddPlayerClass(
        0,
        1481.0, -1771.0, 18.8,
        90.0,
        0, 0,
        0, 0,
        0, 0
    );

    print("[HOPECITY] Gamemode initialized.");
    return 1;
}

public OnPlayerConnect(playerid)
{
    Player_OnConnect(playerid);
    Auth_OnConnect(playerid);
    Utils_SendWelcome(playerid);
    return 1;
}

public OnPlayerRequestSpawn(playerid)
{
    if (!PlayerData[playerid][Player_LoggedIn])
    {
        SendClientMessage(playerid, 0xFF0000FF, "Login dulu sebelum spawn.");
        return 0;
    }

    return 1;
}

public OnPlayerSpawn(playerid)
{
    Player_OnSpawn(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    Database_SavePlayer(playerid);
    Player_OnDisconnect(playerid, reason);
    Auth_OnDisconnect(playerid);
    return 1;
}

public OnGameModeExit()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i))
        {
            Database_SavePlayer(i, 1);
        }
    }

    Database_Close();
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!PlayerData[playerid][Player_LoggedIn])
    {
        SendClientMessage(playerid, 0xFF0000FF, "Login dulu untuk memakai command.");
        return 1;
    }

    return Commands_OnText(playerid, cmdtext);
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused listitem
    return Auth_OnDialogResponse(playerid, dialogid, response, inputtext);
}
