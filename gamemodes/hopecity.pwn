#pragma rational Float
#include <a_samp>

main()
{
    print("====================================");
    print("      HOPECITY ROLEPLAY");
    print("      Development Build");
    print("====================================");
}

public OnGameModeInit()
{
    SetGameModeText("HopeCity RP");

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
    SendClientMessage(playerid, 0x00FF00FF, "Selamat datang di HopeCity Roleplay!");
    SendClientMessage(playerid, 0xFFFFFFFF, "Development Build - Server sedang dalam tahap pengembangan.");
    return 1;
}
