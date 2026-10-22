#if defined _HOPECITY_ADMIN
    #endinput
#endif
#define _HOPECITY_ADMIN

// ==========================================================
//  HopeCity Roleplay - Admin System (gamemodes/command/admin.inc)
//  Level: 1 Helper, 2 Moderator, 3 Admin, 4 Head Admin, 5 Owner
// ==========================================================

#define ADMIN_MAX_LEVEL 5

#define ADMIN_COLOR_ERROR 0xFF6347FF
#define ADMIN_COLOR_OK 0x00FF00FF
#define ADMIN_COLOR_INFO 0xFFFF00FF
#define ADMIN_COLOR_CHAT 0x33CCFFFF
#define ADMIN_COLOR_BROADCAST 0xFF0000FF

new AdminDuty[MAX_PLAYERS];
new AdminHudOn[MAX_PLAYERS];
new AdminWelcomed[MAX_PLAYERS];
new PlayerText:AdminHudBox[MAX_PLAYERS];
new PlayerText:AdminHudText[MAX_PLAYERS];

forward Admin_HudTimer();

// ---------------------- helper ----------------------
stock Admin_LevelName(level, name[], size = sizeof name)
{
    switch (level)
    {
        case 1: format(name, size, "Helper");
        case 2: format(name, size, "Moderator");
        case 3: format(name, size, "Admin");
        case 4: format(name, size, "Head Admin");
        case 5: format(name, size, "Owner");
        default: format(name, size, "Player");
    }

    return 1;
}

stock Admin_IsNumeric(const text[])
{
    if (!text[0])
    {
        return 0;
    }

    for (new i = 0; text[i] != EOS; i++)
    {
        if (text[i] < '0' || text[i] > '9')
        {
            return 0;
        }
    }

    return 1;
}

stock Admin_Token(const text[], &pos, dest[], size = sizeof dest)
{
    new i = 0;

    while (text[pos] == ' ')
    {
        pos++;
    }

    while (text[pos] != EOS && text[pos] != ' ')
    {
        if (i < size - 1)
        {
            dest[i++] = text[pos];
        }

        pos++;
    }

    dest[i] = EOS;
    return i;
}

stock Admin_Rest(const text[], pos, dest[], size = sizeof dest)
{
    while (text[pos] == ' ')
    {
        pos++;
    }

    dest[0] = EOS;
    strmid(dest, text, pos, strlen(text), size);
    return strlen(dest);
}

stock Admin_SendToAdmins(color, const text[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && PlayerData[i][Player_Admin] > 0)
        {
            SendClientMessage(i, color, text);
        }
    }

    return 1;
}

// ---------------------- HUD ----------------------
stock Admin_ShowHud(playerid)
{
    new placeholder[] = "_";

    if (AdminHudOn[playerid])
    {
        return 0;
    }

    AdminHudBox[playerid] = CreatePlayerTextDraw(playerid, 28.0, 292.0, placeholder);
    PlayerTextDrawLetterSize(playerid, AdminHudBox[playerid], 0.0, 7.4);
    PlayerTextDrawTextSize(playerid, AdminHudBox[playerid], 178.0, 0.0);
    PlayerTextDrawUseBox(playerid, AdminHudBox[playerid], 1);
    PlayerTextDrawBoxColor(playerid, AdminHudBox[playerid], 0x000000AA);
    PlayerTextDrawShow(playerid, AdminHudBox[playerid]);

    AdminHudText[playerid] = CreatePlayerTextDraw(playerid, 34.0, 297.0, placeholder);
    PlayerTextDrawLetterSize(playerid, AdminHudText[playerid], 0.25, 1.1);
    PlayerTextDrawFont(playerid, AdminHudText[playerid], 1);
    PlayerTextDrawColor(playerid, AdminHudText[playerid], -1);
    PlayerTextDrawSetOutline(playerid, AdminHudText[playerid], 1);
    PlayerTextDrawSetProportional(playerid, AdminHudText[playerid], 1);
    PlayerTextDrawShow(playerid, AdminHudText[playerid]);

    AdminHudOn[playerid] = 1;
    return 1;
}

stock Admin_HideHud(playerid)
{
    if (!AdminHudOn[playerid])
    {
        return 0;
    }

    PlayerTextDrawDestroy(playerid, AdminHudText[playerid]);
    PlayerTextDrawDestroy(playerid, AdminHudBox[playerid]);
    AdminHudOn[playerid] = 0;
    return 1;
}

stock Admin_UpdateHud(playerid)
{
    if (!AdminHudOn[playerid])
    {
        return 0;
    }

    new levelName[16];
    new text[256];
    new players = 0;
    new admins = 0;

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i))
        {
            continue;
        }

        players++;

        if (AdminDuty[i])
        {
            admins++;
        }
    }

    Admin_LevelName(PlayerData[playerid][Player_Admin], levelName);

    format(
        text,
        sizeof text,
        "~r~ADMIN DUTY~n~~w~%s ~y~(%s)~n~~w~Level: ~g~%d~n~~w~Player: ~y~%d~w~/%d~n~~w~Admin duty: ~y~%d~n~~w~Ping: ~y~%d",
        PlayerData[playerid][Player_Name],
        levelName,
        PlayerData[playerid][Player_Admin],
        players,
        GetMaxPlayers(),
        admins,
        GetPlayerPing(playerid)
    );

    PlayerTextDrawSetString(playerid, AdminHudText[playerid], text);
    return 1;
}

public Admin_HudTimer()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && AdminDuty[i])
        {
            Admin_UpdateHud(i);
        }
    }

    return 1;
}

// ---------------------- duty & akses ----------------------
stock Admin_SetDuty(playerid, enable)
{
    new msg[144];

    if (enable)
    {
        AdminDuty[playerid] = 1;
        Admin_ShowHud(playerid);
        Admin_UpdateHud(playerid);
        format(msg, sizeof msg, "[ADMIN] %s sekarang ON DUTY.", PlayerData[playerid][Player_Name]);
    }
    else
    {
        AdminDuty[playerid] = 0;
        Admin_HideHud(playerid);
        format(msg, sizeof msg, "[ADMIN] %s sekarang OFF DUTY.", PlayerData[playerid][Player_Name]);
    }

    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock Admin_Check(playerid, level, needDuty = 1)
{
    if (PlayerData[playerid][Player_Admin] < level)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak punya akses untuk command ini.");
        return 0;
    }

    if (needDuty && !AdminDuty[playerid])
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Aktifkan /aduty dulu sebelum memakai command admin.");
        return 0;
    }

    return 1;
}

stock Admin_GetTarget(playerid, const token[])
{
    if (!Admin_IsNumeric(token))
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "ID pemain harus berupa angka.");
        return INVALID_PLAYER_ID;
    }

    new target = strval(token);

    if (target < 0 || target >= MAX_PLAYERS || !IsPlayerConnected(target))
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Pemain tidak ditemukan.");
        return INVALID_PLAYER_ID;
    }

    if (!PlayerData[target][Player_LoggedIn])
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Pemain itu belum login.");
        return INVALID_PLAYER_ID;
    }

    return target;
}

stock Admin_CanTarget(playerid, target)
{
    if (target == playerid)
    {
        return 1;
    }

    if (PlayerData[target][Player_Admin] > 0 && PlayerData[target][Player_Admin] >= PlayerData[playerid][Player_Admin])
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak bisa melakukan itu ke admin selevel atau lebih tinggi.");
        return 0;
    }

    return 1;
}

stock Admin_SaveLevel(playerid)
{
    if (HopeCityDB == MySQL:0 || PlayerData[playerid][Player_ID] <= 0)
    {
        return 0;
    }

    new query[128];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "UPDATE hc_accounts SET admin = %d WHERE id = %d",
        PlayerData[playerid][Player_Admin],
        PlayerData[playerid][Player_ID]
    );

    mysql_tquery(HopeCityDB, query);
    return 1;
}

// ---------------------- command ----------------------

stock Admin_CmdDuty(playerid)
{
    if (PlayerData[playerid][Player_Admin] < 1)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu bukan admin.");
        return 1;
    }

    Admin_SetDuty(playerid, !AdminDuty[playerid]);
    return 1;
}

stock Admin_CmdChat(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1, 0))
    {
        return 1;
    }

    new text[110];
    new levelName[16];
    new msg[144];

    if (!Admin_Rest(cmdtext, pos, text))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /a [pesan]");
        return 1;
    }

    Admin_LevelName(PlayerData[playerid][Player_Admin], levelName);
    format(msg, sizeof msg, "[ADMIN CHAT] %s %s: %s", levelName, PlayerData[playerid][Player_Name], text);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock Admin_CmdGoto(playerid, const cmdtext[], pos, bool:bring)
{
    if (!Admin_Check(playerid, 1))
    {
        return 1;
    }

    new token[16];
    new msg[144];
    new Float:x, Float:y, Float:z;

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, bring ? ("Gunakan: /gethere [id]") : ("Gunakan: /goto [id]"));
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    if (target == playerid)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak bisa memilih dirimu sendiri.");
        return 1;
    }

    if (bring)
    {
        if (!Admin_CanTarget(playerid, target))
        {
            return 1;
        }

        GetPlayerPos(playerid, x, y, z);
        SetPlayerPos(target, x + 1.0, y, z);
        SetPlayerInterior(target, GetPlayerInterior(playerid));
        SetPlayerVirtualWorld(target, GetPlayerVirtualWorld(playerid));

        format(msg, sizeof msg, "[ADMIN] Kamu ditarik oleh %s.", PlayerData[playerid][Player_Name]);
        SendClientMessage(target, ADMIN_COLOR_INFO, msg);
        format(msg, sizeof msg, "[ADMIN] Kamu menarik %s ke posisimu.", PlayerData[target][Player_Name]);
        SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
        return 1;
    }

    GetPlayerPos(target, x, y, z);
    SetPlayerPos(playerid, x + 1.0, y, z);
    SetPlayerInterior(playerid, GetPlayerInterior(target));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(target));

    format(msg, sizeof msg, "[ADMIN] Kamu teleport ke %s.", PlayerData[target][Player_Name]);
    SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    return 1;
}

stock Admin_CmdFreeze(playerid, const cmdtext[], pos, bool:freeze)
{
    if (!Admin_Check(playerid, 1))
    {
        return 1;
    }

    new token[16];
    new msg[144];

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, freeze ? ("Gunakan: /freeze [id]") : ("Gunakan: /unfreeze [id]"));
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID || !Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    TogglePlayerControllable(target, freeze ? 0 : 1);

    format(
        msg,
        sizeof msg,
        "[ADMIN] %s %s %s.",
        PlayerData[playerid][Player_Name],
        freeze ? ("membekukan") : ("melepas"),
        PlayerData[target][Player_Name]
    );
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);

    SendClientMessage(target, ADMIN_COLOR_INFO, freeze ? ("[ADMIN] Kamu dibekukan oleh admin.") : ("[ADMIN] Kamu dilepas dari bekuan."));
    return 1;
}

stock Admin_CmdKick(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1))
    {
        return 1;
    }

    new token[16];
    new reason[96];
    new msg[160];

    Admin_Token(cmdtext, pos, token);
    Admin_Rest(cmdtext, pos, reason);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /kick [id] [alasan]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID || !Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    if (!reason[0])
    {
        format(reason, sizeof reason, "Tidak ada alasan");
    }

    format(
        msg,
        sizeof msg,
        "[ADMIN] %s di-kick oleh %s. Alasan: %s",
        PlayerData[target][Player_Name],
        PlayerData[playerid][Player_Name],
        reason
    );
    SendClientMessageToAll(ADMIN_COLOR_BROADCAST, msg);

    Auth_StartTimer("Auth_KickPlayer", 1000, target, AuthSession[target]);
    return 1;
}

// mode: 0 = sethp, 1 = setarmor, 2 = setskin
stock Admin_CmdSetValue(playerid, const cmdtext[], pos, mode)
{
    if (!Admin_Check(playerid, mode == 2 ? 3 : 2))
    {
        return 1;
    }

    new token[16];
    new valueText[16];
    new msg[144];

    Admin_Token(cmdtext, pos, token);
    Admin_Token(cmdtext, pos, valueText);

    if (!token[0] || !Admin_IsNumeric(valueText))
    {
        switch (mode)
        {
            case 0: SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /sethp [id] [0-100]");
            case 1: SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /setarmor [id] [0-100]");
            default: SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /setskin [id] [0-311]");
        }

        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    new value = strval(valueText);

    switch (mode)
    {
        case 0:
        {
            if (value > 100)
            {
                value = 100;
            }

            SetPlayerHealth(target, float(value));
            format(msg, sizeof msg, "[ADMIN] %s mengatur HP %s menjadi %d.", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name], value);
        }
        case 1:
        {
            if (value > 100)
            {
                value = 100;
            }

            SetPlayerArmour(target, float(value));
            format(msg, sizeof msg, "[ADMIN] %s mengatur armor %s menjadi %d.", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name], value);
        }
        default:
        {
            if (value > 311 || value == 74)
            {
                SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Skin ID tidak valid (0-311, bukan 74).");
                return 1;
            }

            SetPlayerSkin(target, value);
            format(msg, sizeof msg, "[ADMIN] %s mengubah skin %s menjadi %d.", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name], value);
        }
    }

    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock Admin_CmdGiveMoney(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 3))
    {
        return 1;
    }

    new token[16];
    new amountText[16];
    new msg[144];

    Admin_Token(cmdtext, pos, token);
    Admin_Token(cmdtext, pos, amountText);

    if (!token[0] || !Admin_IsNumeric(amountText))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /givemoney [id] [jumlah]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    new amount = strval(amountText);

    if (amount < 1 || amount > 10000000)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Jumlah harus antara 1 sampai 10.000.000.");
        return 1;
    }

    GivePlayerMoney(target, amount);

    format(msg, sizeof msg, "[ADMIN] %s memberi $%d ke %s.", PlayerData[playerid][Player_Name], amount, PlayerData[target][Player_Name]);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);

    format(msg, sizeof msg, "[ADMIN] Kamu menerima $%d dari admin.", amount);
    SendClientMessage(target, ADMIN_COLOR_OK, msg);
    return 1;
}

stock Admin_CmdAnnounce(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 2))
    {
        return 1;
    }

    new text[110];
    new msg[200];

    if (!Admin_Rest(cmdtext, pos, text))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /announce [pesan]");
        return 1;
    }

    for (new i = 0; text[i] != EOS; i++)
    {
        if (text[i] == '~')
        {
            text[i] = '-';
        }
    }

    format(msg, sizeof msg, "[PENGUMUMAN] %s", text);
    SendClientMessageToAll(ADMIN_COLOR_BROADCAST, msg);

    format(msg, sizeof msg, "~y~PENGUMUMAN~n~~w~%s", text);
    GameTextForAll(msg, 6000, 3);
    return 1;
}

stock Admin_CmdSetAdmin(playerid, const cmdtext[], pos)
{
    if (!IsPlayerAdmin(playerid) && PlayerData[playerid][Player_Admin] < ADMIN_MAX_LEVEL)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Hanya Owner atau RCON admin yang bisa memakai /setadmin.");
        return 1;
    }

    new token[16];
    new levelText[16];
    new levelName[16];
    new msg[144];

    Admin_Token(cmdtext, pos, token);
    Admin_Token(cmdtext, pos, levelText);

    if (!token[0] || !Admin_IsNumeric(levelText))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /setadmin [id] [0-5]");
        return 1;
    }

    new level = strval(levelText);

    if (level < 0 || level > ADMIN_MAX_LEVEL)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Level harus antara 0 sampai 5.");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    PlayerData[target][Player_Admin] = level;

    if (level == 0 && AdminDuty[target])
    {
        Admin_SetDuty(target, 0);
    }

    Admin_SaveLevel(target);
    Admin_LevelName(level, levelName);

    format(msg, sizeof msg, "[ADMIN] %s mengatur level admin %s menjadi %d (%s).", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name], level, levelName);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);

    if (level > 0)
    {
        format(msg, sizeof msg, "[ADMIN] Level admin kamu sekarang %d (%s). Ketik /ahelp.", level, levelName);
    }
    else
    {
        format(msg, sizeof msg, "[ADMIN] Status admin kamu dicabut.");
    }

    SendClientMessage(target, ADMIN_COLOR_INFO, msg);

    if (target != playerid)
    {
        format(msg, sizeof msg, "[ADMIN] Level %s diubah menjadi %d.", PlayerData[target][Player_Name], level);
        SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    }

    return 1;
}

// ---------------------- hook (dipanggil dari hopecity.pwn) ----------------------
stock Admin_Init()
{
    new timerFn[] = "Admin_HudTimer";
    SetTimer(timerFn, 1000, true);

    print("[ADMIN] Admin system initialized.");
    return 1;
}

stock Admin_OnSpawn(playerid)
{
    if (PlayerData[playerid][Player_Admin] > 0 && !AdminWelcomed[playerid])
    {
        new levelName[16];
        new msg[144];

        Admin_LevelName(PlayerData[playerid][Player_Admin], levelName);
        format(msg, sizeof msg, "[ADMIN] Kamu login sebagai %s (level %d). Ketik /ahelp untuk daftar command.", levelName, PlayerData[playerid][Player_Admin]);
        SendClientMessage(playerid, ADMIN_COLOR_INFO, msg);
        AdminWelcomed[playerid] = 1;
    }

    return 1;
}

stock Admin_OnDisconnect(playerid)
{
    AdminDuty[playerid] = 0;
    AdminHudOn[playerid] = 0;
    AdminWelcomed[playerid] = 0;
    return 1;
}

// return 1 kalau command dikenali & ditangani, 0 kalau bukan command admin
stock Admin_OnCommandText(playerid, const cmdtext[])
{
    if (PlayerData[playerid][Player_Admin] < 1 && !IsPlayerAdmin(playerid))
    {
        return 0;
    }

    new cmd[24];
    new pos = 1;

    Admin_Token(cmdtext, pos, cmd);

    if (!strcmp(cmd, "aduty", true)) return Admin_CmdDuty(playerid);
    if (!strcmp(cmd, "a", true)) return Admin_CmdChat(playerid, cmdtext, pos);
    if (!strcmp(cmd, "goto", true)) return Admin_CmdGoto(playerid, cmdtext, pos, false);
    if (!strcmp(cmd, "gethere", true)) return Admin_CmdGoto(playerid, cmdtext, pos, true);
    if (!strcmp(cmd, "freeze", true)) return Admin_CmdFreeze(playerid, cmdtext, pos, true);
    if (!strcmp(cmd, "unfreeze", true)) return Admin_CmdFreeze(playerid, cmdtext, pos, false);
    if (!strcmp(cmd, "kick", true)) return Admin_CmdKick(playerid, cmdtext, pos);
    if (!strcmp(cmd, "sethp", true)) return Admin_CmdSetValue(playerid, cmdtext, pos, 0);
    if (!strcmp(cmd, "setarmor", true)) return Admin_CmdSetValue(playerid, cmdtext, pos, 1);
    if (!strcmp(cmd, "setskin", true)) return Admin_CmdSetValue(playerid, cmdtext, pos, 2);
    if (!strcmp(cmd, "givemoney", true)) return Admin_CmdGiveMoney(playerid, cmdtext, pos);
    if (!strcmp(cmd, "announce", true)) return Admin_CmdAnnounce(playerid, cmdtext, pos);
    if (!strcmp(cmd, "setadmin", true)) return Admin_CmdSetAdmin(playerid, cmdtext, pos);

    return 0;
}
