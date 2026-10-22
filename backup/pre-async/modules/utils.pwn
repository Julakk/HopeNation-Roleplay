#if defined _HOPECITY_UTILS
    #endinput
#endif
#define _HOPECITY_UTILS

stock Utils_SendServerMessage(const message[])
{
    SendClientMessageToAll(0xFFFFFFFF, message);
    return 1;
}

stock Utils_SendWelcome(playerid)
{
    SendClientMessage(playerid, 0xFFFFFFFF, "Gunakan /help untuk melihat command yang tersedia.");
    return 1;
}
