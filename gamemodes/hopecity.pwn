#pragma rational Float

#include <a_samp>

#include "modules/core.pwn"
#include "modules/player.pwn"
#include "modules/commands.pwn"
#include "modules/utils.pwn"
#include "modules/auth.pwn"

main()
{
    print("====================================");
    print("      HOPECITY ROLEPLAY");
    print("      Development Build");
    print("====================================");
}

public OnGameModeInit()
{
    Core_Init();
    Player_Init();
    Commands_Init();
    Auth_Init();

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

public OnPlayerSpawn(playerid)
{
    Player_OnSpawn(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    Player_OnDisconnect(playerid, reason);
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    return Commands_OnText(playerid, cmdtext);
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    #pragma unused listitem
    return Auth_OnDialogResponse(playerid, dialogid, response, inputtext);
}
