#if defined _HOPECITY_COMMANDS
    #endinput
#endif
#define _HOPECITY_COMMANDS

stock Commands_Init()
{
    print("[COMMANDS] Command system initialized.");
    return 1;
}

stock Commands_OnText(playerid, const cmdtext[])
{
    if (!strcmp(cmdtext, "/help", true))
    {
        SendClientMessage(playerid, 0xFFFFFFFF, "=== HopeCity Roleplay ===");
        SendClientMessage(playerid, 0xFFFFFFFF, "/help - Menampilkan bantuan.");
        SendClientMessage(playerid, 0xFFFFFFFF, "/credits - Informasi developer.");
        return 1;
    }

    if (!strcmp(cmdtext, "/credits", true))
    {
        SendClientMessage(playerid, 0xFFFFFFFF, "HopeCity Roleplay");
        SendClientMessage(playerid, 0xFFFFFFFF, "Development Build");
        return 1;
    }

    return 0;
}
