#if !defined _floatstr_fallback
#define _floatstr_fallback
stock Float:floatstr_fb(const string[])
{
new Float:result = 0.0;
new Float:frac = 0.1;
new i = 0;
new bool:neg = false;
new bool:afterDot = false;
if (string[0] == '-') { neg = true; i++; }
else if (string[0] == '+') { i++; }
for (; string[i] != EOS; i++)
{
if (string[i] == '.') { afterDot = true; continue; }
if (string[i] < '0' || string[i] > '9') break;
if (!afterDot) result = result * 10.0 + float(string[i] - '0');
else { result += float(string[i] - '0') * frac; frac /= 10.0; }
}
return neg ? -result : result;
}
#define floatstr floatstr_fb
#endif
#if !defined _floatstr_fallback
    #define _floatstr_fallback
    stock Float:floatstr_fb(const string[])
    {
        new Float:result = 0.0, Float:frac = 0.1, i = 0, bool:neg = false, bool:afterDot = false;
        if (string[0] == '-') { neg = true; i++; }
        else if (string[0] == '+') i++;
        for (; string[i] != EOS; i++)
        {
            if (string[i] == '.') { afterDot = true; continue; }
            if (string[i] < '0' || string[i] > '9') break;
            if (!afterDot) result = result * 10.0 + float(string[i] - '0');
            else { result += float(string[i] - '0') * frac; frac /= 10.0; }
        }
        return neg ? -result : result;
    }
    #define floatstr floatstr_fb
#endif
#if defined _HOPECITY_ADMIN_CMD
    #endinput
#endif
#define _HOPECITY_ADMIN_CMD

// ==========================================================
//  HopeCity Roleplay - Admin Commands Lanjutan
//  (gamemodes/command/admin_extra.inc)
//  Wajib di-include SETELAH admin.inc
// ==========================================================

// 1 = ban juga berdasarkan IP (hati-hati: IP seluler sering dipakai banyak orang)
#define ADMINX_BAN_BY_IP 0
#define ADMINX_WARN_LIMIT 3

new AdminMuted[MAX_PLAYERS];
new AdminWarns[MAX_PLAYERS];
new AdminSpecOn[MAX_PLAYERS];
new AdminSpecTarget[MAX_PLAYERS];
new AdminSpecReturn[MAX_PLAYERS];
new AdminSpecInterior[MAX_PLAYERS];
new AdminSpecWorld[MAX_PLAYERS];
new Float:AdminSpecData[MAX_PLAYERS][6];
new AdminVehicle[MAX_PLAYERS];
new AdminCmdHudOn[MAX_PLAYERS];

forward AdminX_Timer();
forward AdminX_OnBanCheck(playerid, session);
forward AdminX_OnUnban(playerid, name[]);

// ---------------------- helper ----------------------
stock AdminX_IsFloat(const text[])
{
    if (!text[0])
    {
        return 0;
    }

    new dots = 0;

    for (new i = 0; text[i] != EOS; i++)
    {
        if (i == 0 && text[i] == '-')
        {
            if (!text[1])
            {
                return 0;
            }

            continue;
        }

        if (text[i] == '.')
        {
            dots++;

            if (dots > 1)
            {
                return 0;
            }

            continue;
        }

        if (text[i] < '0' || text[i] > '9')
        {
            return 0;
        }
    }

    return 1;
}

// command teleport: level cukup + WAJIB sedang duty


// ---------------------- teleport ----------------------
new AdminTpTicket[MAX_PLAYERS];

forward AdminX_TpUnlock(playerid, ticket);

// command teleport: level cukup + WAJIB sedang duty
stock AdminX_CheckTP(playerid, level)
{
    new msg[96];

    if (PlayerData[playerid][Player_Admin] < level)
    {
        format(msg, sizeof msg, "Command ini butuh admin level %d (level kamu: %d).", level, PlayerData[playerid][Player_Admin]);
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, msg);
        printf("[ADMINX] TP ditolak (level kurang): %s level=%d butuh=%d", PlayerData[playerid][Player_Name], PlayerData[playerid][Player_Admin], level);
        return 0;
    }

    if (!AdminDuty[playerid])
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu harus ON DUTY (/aduty) dulu sebelum bisa teleport.");
        printf("[ADMINX] TP ditolak (belum duty): %s", PlayerData[playerid][Player_Name]);
        return 0;
    }

    return 1;
}

public AdminX_TpUnlock(playerid, ticket)
{
    if (!IsPlayerConnected(playerid) || AdminTpTicket[playerid] != ticket)
    {
        return 0;
    }

    TogglePlayerControllable(playerid, 1);
    return 1;
}

stock AdminX_TeleportTo(playerid, Float:x, Float:y, Float:z, interior, world)
{
    new vehicleid = GetPlayerVehicleID(playerid);
    new driving = (vehicleid != 0 && GetPlayerState(playerid) == PLAYER_STATE_DRIVER);

    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, world);

    if (driving)
    {
        LinkVehicleToInterior(vehicleid, interior);
        SetVehicleVirtualWorld(vehicleid, world);
        SetVehiclePos(vehicleid, x, y, z + 0.7);
    }
    else
    {
        SetPlayerPos(playerid, x, y, z + 0.7);

        // tahan sebentar supaya map sempat ke-load (client mobile sering jatuh/nyangkut)
        new timerFn[] = "AdminX_TpUnlock";

        TogglePlayerControllable(playerid, 0);
        AdminTpTicket[playerid]++;
        SetTimerEx(timerFn, 1500, false, "dd", playerid, AdminTpTicket[playerid]);
    }

    SetCameraBehindPlayer(playerid);

    printf("[ADMINX] TP id=%d -> %.1f, %.1f, %.1f (interior %d, vw %d)", playerid, x, y, z, interior, world);
    return 1;
}

stock AdminX_CmdGotoPlayer(playerid, const cmdtext[], pos, bool:bring)
{
    if (!AdminX_CheckTP(playerid, 1))
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
        format(msg, sizeof msg, "Kamu tidak bisa memilih dirimu sendiri (ID kamu: %d). Sendirian? Pakai /gotolv /gotosf /gotoco.", playerid);
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, msg);
        return 1;
    }

    if (bring)
    {
        if (!Admin_CanTarget(playerid, target))
        {
            return 1;
        }

        GetPlayerPos(playerid, x, y, z);
        AdminX_TeleportTo(target, x + 1.0, y, z, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

        format(msg, sizeof msg, "[ADMIN] Kamu ditarik oleh %s.", PlayerData[playerid][Player_Name]);
        SendClientMessage(target, ADMIN_COLOR_INFO, msg);
        format(msg, sizeof msg, "[ADMIN] Kamu menarik %s ke posisimu.", PlayerData[target][Player_Name]);
        SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
        return 1;
    }

    GetPlayerPos(target, x, y, z);
    AdminX_TeleportTo(playerid, x + 1.0, y, z, GetPlayerInterior(target), GetPlayerVirtualWorld(target));

    format(msg, sizeof msg, "[ADMIN] Kamu teleport ke %s.", PlayerData[target][Player_Name]);
    SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    return 1;
}
stock AdminX_CmdGotoPlace(playerid, place)
{
    if (!AdminX_CheckTP(playerid, 1))
    {
        return 1;
    }

    new Float:x, Float:y, Float:z;
    new name[24];
    new msg[64];

    switch (place)
    {
        case 0:
        {
            x = 1529.6;
            y = -1691.2;
            z = 13.4;
            format(name, sizeof name, "Los Santos");
        }
        case 1:
        {
            x = 2031.1;
            y = 1008.2;
            z = 10.8;
            format(name, sizeof name, "Las Venturas");
        }
        default:
        {
            x = -1988.0;
            y = 138.0;
            z = 27.7;
            format(name, sizeof name, "San Fierro");
        }
    }

    AdminX_TeleportTo(playerid, x, y, z, 0, 0);

    format(msg, sizeof msg, "[ADMIN] Teleport ke %s.", name);
    SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    return 1;
}

stock AdminX_CmdGotoCo(playerid, const cmdtext[], pos)
{
    if (!AdminX_CheckTP(playerid, 1))
    {
        return 1;
    }

    new tx[16], ty[16], tz[16], ti[8];
    new msg[96];

    Admin_Token(cmdtext, pos, tx);
    Admin_Token(cmdtext, pos, ty);
    Admin_Token(cmdtext, pos, tz);
    Admin_Token(cmdtext, pos, ti);

    if (!AdminX_IsFloat(tx) || !AdminX_IsFloat(ty) || !AdminX_IsFloat(tz))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /gotoco [x] [y] [z] [interior opsional]");
        return 1;
    }

    new Float:x = floatstr(tx);
    new Float:y = floatstr(ty);
    new Float:z = floatstr(tz);
    new interior = 0;

    if (ti[0])
    {
        if (!Admin_IsNumeric(ti))
        {
            SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /gotoco [x] [y] [z] [interior opsional]");
            return 1;
        }

        interior = strval(ti);
    }

    if (floatabs(x) > 20000.0 || floatabs(y) > 20000.0 || floatabs(z) > 20000.0)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Koordinat di luar batas (maksimal 20000).");
        return 1;
    }

    AdminX_TeleportTo(playerid, x, y, z, interior, 0);

    format(msg, sizeof msg, "[ADMIN] Teleport ke %.1f, %.1f, %.1f (interior %d).", x, y, z, interior);
    SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    return 1;
}

stock AdminX_CmdSendTo(playerid, const cmdtext[], pos)
{
    if (!AdminX_CheckTP(playerid, 2))
    {
        return 1;
    }

    new tokenA[16], tokenB[16];
    new msg[144];
    new Float:x, Float:y, Float:z;

    Admin_Token(cmdtext, pos, tokenA);
    Admin_Token(cmdtext, pos, tokenB);

    if (!tokenA[0] || !tokenB[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /sendto [id yang dikirim] [id tujuan]");
        return 1;
    }

    new first = Admin_GetTarget(playerid, tokenA);

    if (first == INVALID_PLAYER_ID)
    {
        return 1;
    }

    new second = Admin_GetTarget(playerid, tokenB);

    if (second == INVALID_PLAYER_ID)
    {
        return 1;
    }

    if (first == second)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Pemain asal dan tujuan tidak boleh sama.");
        return 1;
    }

    if (!Admin_CanTarget(playerid, first))
    {
        return 1;
    }

    GetPlayerPos(second, x, y, z);
    AdminX_TeleportTo(first, x + 1.0, y, z, GetPlayerInterior(second), GetPlayerVirtualWorld(second));

    format(msg, sizeof msg, "[ADMIN] %s mengirim %s ke %s.", PlayerData[playerid][Player_Name], PlayerData[first][Player_Name], PlayerData[second][Player_Name]);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

// ---------------------- spectate ----------------------
stock AdminX_StopSpec(playerid)
{
    if (!AdminSpecOn[playerid])
    {
        return 0;
    }

    AdminSpecOn[playerid] = 0;
    AdminSpecReturn[playerid] = 1;
    TogglePlayerSpectating(playerid, 0);
    return 1;
}

stock AdminX_CmdSpec(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1))
    {
        return 1;
    }

    new token[16];

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /spec [id]  (berhenti: /specoff)");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    if (target == playerid)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak bisa spec dirimu sendiri.");
        return 1;
    }

    if (!AdminSpecOn[playerid])
    {
        new Float:x, Float:y, Float:z, Float:angle, Float:health, Float:armor;

        GetPlayerPos(playerid, x, y, z);
        GetPlayerFacingAngle(playerid, angle);
        GetPlayerHealth(playerid, health);
        GetPlayerArmour(playerid, armor);

        AdminSpecData[playerid][0] = x;
        AdminSpecData[playerid][1] = y;
        AdminSpecData[playerid][2] = z;
        AdminSpecData[playerid][3] = angle;
        AdminSpecData[playerid][4] = health;
        AdminSpecData[playerid][5] = armor;
        AdminSpecInterior[playerid] = GetPlayerInterior(playerid);
        AdminSpecWorld[playerid] = GetPlayerVirtualWorld(playerid);

        TogglePlayerSpectating(playerid, 1);
        AdminSpecOn[playerid] = 1;
    }

    AdminSpecTarget[playerid] = target;
    SetPlayerInterior(playerid, GetPlayerInterior(target));
    SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(target));

    if (IsPlayerInAnyVehicle(target))
    {
        PlayerSpectateVehicle(playerid, GetPlayerVehicleID(target));
    }
    else
    {
        PlayerSpectatePlayer(playerid, target);
    }

    new msg[96];
    format(msg, sizeof msg, "[ADMIN] Kamu men-spec %s. Ketik /specoff untuk berhenti.", PlayerData[target][Player_Name]);
    SendClientMessage(playerid, ADMIN_COLOR_OK, msg);
    return 1;
}

stock AdminX_CmdSpecOff(playerid)
{
    if (!Admin_Check(playerid, 1, 0))
    {
        return 1;
    }

    if (!AdminX_StopSpec(playerid))
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu sedang tidak spec siapa pun.");
        return 1;
    }

    SendClientMessage(playerid, ADMIN_COLOR_OK, "[ADMIN] Spec dihentikan.");
    return 1;
}

// ---------------------- moderasi ----------------------
stock AdminX_CmdMute(playerid, const cmdtext[], pos, mute)
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
        SendClientMessage(playerid, ADMIN_COLOR_INFO, mute ? ("Gunakan: /mute [id]") : ("Gunakan: /unmute [id]"));
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID || !Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    AdminMuted[target] = mute;

    format(
        msg,
        sizeof msg,
        "[ADMIN] %s %s oleh %s.",
        PlayerData[target][Player_Name],
        mute ? ("di-mute") : ("di-unmute"),
        PlayerData[playerid][Player_Name]
    );
    SendClientMessageToAll(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock AdminX_CmdWarn(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1))
    {
        return 1;
    }

    new token[16];
    new reason[80];
    new msg[144];

    Admin_Token(cmdtext, pos, token);
    Admin_Rest(cmdtext, pos, reason);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /warn [id] [alasan]");
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

    AdminWarns[target]++;

    format(
        msg,
        sizeof msg,
        "[ADMIN] %s di-warn oleh %s (%d/%d). Alasan: %s",
        PlayerData[target][Player_Name],
        PlayerData[playerid][Player_Name],
        AdminWarns[target],
        ADMINX_WARN_LIMIT,
        reason
    );
    SendClientMessageToAll(ADMIN_COLOR_BROADCAST, msg);

    if (AdminWarns[target] >= ADMINX_WARN_LIMIT)
    {
        AdminWarns[target] = 0;
        SendClientMessage(target, ADMIN_COLOR_ERROR, "Kamu mencapai batas warn dan di-kick.");
        Auth_StartTimer("Auth_KickPlayer", 1500, target, AuthSession[target]);
    }

    return 1;
}

stock AdminX_CmdSlap(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 2))
    {
        return 1;
    }

    new token[16];
    new msg[144];
    new Float:x, Float:y, Float:z;

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /slap [id]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID || !Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    GetPlayerPos(target, x, y, z);
    SetPlayerPos(target, x, y, z + 6.0);

    format(msg, sizeof msg, "[ADMIN] %s men-slap %s.", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name]);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock AdminX_CmdSpawn(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 2))
    {
        return 1;
    }

    new token[16];
    new msg[144];

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /spawn [id]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID || !Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    SpawnPlayer(target);

    format(msg, sizeof msg, "[ADMIN] %s me-respawn %s.", PlayerData[playerid][Player_Name], PlayerData[target][Player_Name]);
    Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock AdminX_CmdClearChat(playerid)
{
    if (!Admin_Check(playerid, 2))
    {
        return 1;
    }

    new msg[96];

    for (new i = 0; i < 20; i++)
    {
        SendClientMessageToAll(-1, " ");
    }

    format(msg, sizeof msg, "[ADMIN] Chat dibersihkan oleh %s.", PlayerData[playerid][Player_Name]);
    SendClientMessageToAll(ADMIN_COLOR_CHAT, msg);
    return 1;
}

// ---------------------- ban / unban ----------------------
stock AdminX_CmdBan(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 3))
    {
        return 1;
    }

    new token[16];
    new reason[90];
    new msg[144];
    new ip[16];
    new query[400];

    Admin_Token(cmdtext, pos, token);
    Admin_Rest(cmdtext, pos, reason);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /ban [id] [alasan]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    if (target == playerid)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak bisa mem-ban dirimu sendiri.");
        return 1;
    }

    if (!Admin_CanTarget(playerid, target))
    {
        return 1;
    }

    if (HopeCityDB == MySQL:0)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Database tidak tersambung, ban dibatalkan.");
        return 1;
    }

    if (!reason[0])
    {
        format(reason, sizeof reason, "Tidak ada alasan");
    }

    GetPlayerIp(target, ip, sizeof ip);

    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "INSERT INTO hc_bans (name, ip, reason, banned_by) VALUES ('%e', '%e', '%e', '%e')",
        PlayerData[target][Player_Name],
        ip,
        reason,
        PlayerData[playerid][Player_Name]
    );
    mysql_tquery(HopeCityDB, query);

    format(
        msg,
        sizeof msg,
        "[ADMIN] %s di-ban oleh %s. Alasan: %s",
        PlayerData[target][Player_Name],
        PlayerData[playerid][Player_Name],
        reason
    );
    SendClientMessageToAll(ADMIN_COLOR_BROADCAST, msg);

    Auth_StartTimer("Auth_KickPlayer", 1500, target, AuthSession[target]);
    return 1;
}

stock AdminX_CmdUnban(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 4))
    {
        return 1;
    }

    new name[MAX_PLAYER_NAME + 1];
    new query[160];

    Admin_Token(cmdtext, pos, name);

    if (!name[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /unban [nama akun]");
        return 1;
    }

    if (HopeCityDB == MySQL:0)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Database tidak tersambung.");
        return 1;
    }

    mysql_format(HopeCityDB, query, sizeof query, "DELETE FROM hc_bans WHERE name = '%e'", name);
    mysql_tquery(HopeCityDB, query, "AdminX_OnUnban", "ds", playerid, name);
    return 1;
}

public AdminX_OnUnban(playerid, name[])
{
    if (!IsPlayerConnected(playerid))
    {
        return 0;
    }

    new msg[144];

    if (cache_affected_rows() > 0)
    {
        format(msg, sizeof msg, "[ADMIN] %s di-unban oleh %s.", name, PlayerData[playerid][Player_Name]);
        Admin_SendToAdmins(ADMIN_COLOR_CHAT, msg);
    }
    else
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Nama itu tidak ada di daftar ban.");
    }

    return 1;
}

public AdminX_OnBanCheck(playerid, session)
{
    if (!IsPlayerConnected(playerid) || AuthSession[playerid] != session)
    {
        return 0;
    }

    new rows;
    cache_get_row_count(rows);

    if (rows <= 0)
    {
        return 1;
    }

    new reason[96];
    new bannedBy[MAX_PLAYER_NAME + 1];
    new msg[144];

    cache_get_value_name(0, "reason", reason, sizeof reason);
    cache_get_value_name(0, "banned_by", bannedBy, sizeof bannedBy);

    format(msg, sizeof msg, "Kamu diblokir dari server. Alasan: %s (oleh %s).", reason, bannedBy);
    SendClientMessage(playerid, 0xFF0000FF, msg);

    AuthBusy[playerid] = 1;
    Auth_StartTimer("Auth_KickPlayer", 2000, playerid, AuthSession[playerid]);
    return 1;
}

// ---------------------- dunia & kendaraan ----------------------
// mode: 0 = cuaca, 1 = jam
stock AdminX_CmdWorld(playerid, const cmdtext[], pos, mode)
{
    if (!Admin_Check(playerid, 3))
    {
        return 1;
    }

    new valueText[8];
    new msg[144];

    Admin_Token(cmdtext, pos, valueText);

    if (!Admin_IsNumeric(valueText))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, mode == 0 ? ("Gunakan: /setweather [0-45]") : ("Gunakan: /settime [0-23]"));
        return 1;
    }

    new value = strval(valueText);

    if (mode == 0)
    {
        if (value > 45)
        {
            SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Cuaca harus 0 sampai 45.");
            return 1;
        }

        SetWeather(value);
        format(msg, sizeof msg, "[ADMIN] %s mengubah cuaca menjadi %d.", PlayerData[playerid][Player_Name], value);
    }
    else
    {
        if (value > 23)
        {
            SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Jam harus 0 sampai 23.");
            return 1;
        }

        SetWorldTime(value);
        format(msg, sizeof msg, "[ADMIN] %s mengubah jam server menjadi %02d:00.", PlayerData[playerid][Player_Name], value);
    }

    SendClientMessageToAll(ADMIN_COLOR_CHAT, msg);
    return 1;
}

stock AdminX_CmdVeh(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 3))
    {
        return 1;
    }

    new modelText[8], color1Text[8], color2Text[8];
    new Float:x, Float:y, Float:z, Float:angle;

    Admin_Token(cmdtext, pos, modelText);
    Admin_Token(cmdtext, pos, color1Text);
    Admin_Token(cmdtext, pos, color2Text);

    if (!Admin_IsNumeric(modelText))
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /veh [model 400-611] [warna1] [warna2]");
        return 1;
    }

    new model = strval(modelText);

    if (model < 400 || model > 611)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Model kendaraan harus 400 sampai 611.");
        return 1;
    }

    new color1 = -1;
    new color2 = -1;

    if (Admin_IsNumeric(color1Text))
    {
        color1 = strval(color1Text);

        if (color1 > 255)
        {
            color1 = 255;
        }
    }

    if (Admin_IsNumeric(color2Text))
    {
        color2 = strval(color2Text);

        if (color2 > 255)
        {
            color2 = 255;
        }
    }

    if (AdminVehicle[playerid] != 0)
    {
        DestroyVehicle(AdminVehicle[playerid]);
        AdminVehicle[playerid] = 0;
    }

    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);

    new vehicleid = CreateVehicle(model, x, y, z + 0.5, angle, color1, color2, -1);

    if (vehicleid == INVALID_VEHICLE_ID)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Gagal membuat kendaraan.");
        return 1;
    }

    LinkVehicleToInterior(vehicleid, GetPlayerInterior(playerid));
    SetVehicleVirtualWorld(vehicleid, GetPlayerVirtualWorld(playerid));
    PutPlayerInVehicle(playerid, vehicleid, 0);
    AdminVehicle[playerid] = vehicleid;

    SendClientMessage(playerid, ADMIN_COLOR_OK, "[ADMIN] Kendaraan dibuat. Kendaraan sebelumnya (kalau ada) dihapus.");
    return 1;
}

stock AdminX_CmdFixVeh(playerid)
{
    if (!Admin_Check(playerid, 3))
    {
        return 1;
    }

    new vehicleid = GetPlayerVehicleID(playerid);

    if (vehicleid == 0)
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu tidak sedang di dalam kendaraan.");
        return 1;
    }

    RepairVehicle(vehicleid);
    SetVehicleHealth(vehicleid, 1000.0);
    SendClientMessage(playerid, ADMIN_COLOR_OK, "[ADMIN] Kendaraan diperbaiki.");
    return 1;
}

// ---------------------- info ----------------------
stock AdminX_CmdPlayers(playerid)
{
    if (!Admin_Check(playerid, 1, 0))
    {
        return 1;
    }

    new msg[144];
    new entry[40];
    new count = 0;

    msg[0] = EOS;
    SendClientMessage(playerid, ADMIN_COLOR_INFO, "===== PLAYER ONLINE =====");

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i))
        {
            continue;
        }

        format(entry, sizeof entry, "[%d]%s  ", i, PlayerData[i][Player_Name]);

        if (strlen(msg) + strlen(entry) > 120)
        {
            SendClientMessage(playerid, -1, msg);
            msg[0] = EOS;
        }

        strcat(msg, entry, sizeof msg);
        count++;
    }

    if (msg[0])
    {
        SendClientMessage(playerid, -1, msg);
    }

    format(msg, sizeof msg, "Total: %d pemain.", count);
    SendClientMessage(playerid, ADMIN_COLOR_INFO, msg);
    return 1;
}

stock AdminX_CmdACheck(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1, 0))
    {
        return 1;
    }

    new token[16];
    new msg[144];
    new ip[16];
    new Float:health, Float:armor;

    Admin_Token(cmdtext, pos, token);

    if (!token[0])
    {
        SendClientMessage(playerid, ADMIN_COLOR_INFO, "Gunakan: /acheck [id]");
        return 1;
    }

    new target = Admin_GetTarget(playerid, token);

    if (target == INVALID_PLAYER_ID)
    {
        return 1;
    }

    GetPlayerHealth(target, health);
    GetPlayerArmour(target, armor);

    format(
        msg,
        sizeof msg,
        "[%d] %s | Admin Lv:%d | $%d | HP:%.0f Armor:%.0f | Ping:%d | Warn:%d%s",
        target,
        PlayerData[target][Player_Name],
        PlayerData[target][Player_Admin],
        GetPlayerMoney(target),
        health,
        armor,
        GetPlayerPing(target),
        AdminWarns[target],
        AdminMuted[target] ? (" | MUTED") : ("")
    );
    SendClientMessage(playerid, ADMIN_COLOR_INFO, msg);

    if (PlayerData[playerid][Player_Admin] >= 3)
    {
        GetPlayerIp(target, ip, sizeof ip);
        format(msg, sizeof msg, "    Account ID: %d | IP: %s | Interior: %d", PlayerData[target][Player_ID], ip, GetPlayerInterior(target));
        SendClientMessage(playerid, ADMIN_COLOR_INFO, msg);
    }

    return 1;
}


// ---------------------- timer & hook ----------------------
new PlayerText:AdminHelpBox[MAX_PLAYERS];
new PlayerText:AdminHelpText[MAX_PLAYERS];
new PlayerText:AdminHelpBtn[MAX_PLAYERS][3];
new AdminHelpBtnOn[MAX_PLAYERS][3];
new AdminHelpPage[MAX_PLAYERS];
new AdminHelpSelecting[MAX_PLAYERS];

stock AdminX_HelpPages(level)
{
    new total = 2;

    if (level >= 2) total++;
    if (level >= 3) total++;
    if (level >= 4) total++;

    return total;
}

stock AdminX_HelpLines(const text[])
{
    new lines = 1;

    for (new i = 0; text[i] != EOS; i++)
    {
        if (text[i] == '~' && text[i + 1] == 'n' && text[i + 2] == '~')
        {
            lines++;
        }
    }

    return lines;
}

stock AdminX_HelpBuild(page, total, level, text[], size)
{
    format(text, size, "~r~ADMIN PANEL ~w~(%d/%d)~n~", page, total);

    switch (page)
    {
        case 1:
        {
            strcat(text, "~y~Umum~n~~w~/aduty /a /ahelp /players /acheck~n~", size);
            strcat(text, "~y~Teleport (wajib duty)~n~~w~/goto /gethere /gotoco~n~~w~/gotols /gotolv /gotosf", size);
        }
        case 2:
        {
            strcat(text, "~y~Moderasi~n~~w~/kick /mute /unmute /warn~n~~w~/freeze /unfreeze~n~~w~/spec /specoff", size);
        }
        case 3:
        {
            strcat(text, "~y~Level 2~n~~w~/sethp /setarmor /slap /spawn~n~~w~/sendto /announce /clearchat~n~~w~(/sendto wajib duty)", size);
        }
        case 4:
        {
            strcat(text, "~y~Level 3~n~~w~/givemoney /setskin /veh /fixveh~n~~w~/ban /setweather /settime", size);
        }
        default:
        {
            strcat(text, "~y~Level 4~n~~w~/unban", size);

            if (level >= 5)
            {
                strcat(text, "~n~~y~Level 5 / RCON~n~~w~/setadmin", size);
            }
        }
    }

    return 1;
}

stock AdminX_HelpHide(playerid, cancel = 1)
{
    if (!AdminCmdHudOn[playerid])
    {
        return 0;
    }

    PlayerTextDrawDestroy(playerid, AdminHelpText[playerid]);
    PlayerTextDrawDestroy(playerid, AdminHelpBox[playerid]);

    for (new i = 0; i < 3; i++)
    {
        if (AdminHelpBtnOn[playerid][i])
        {
            PlayerTextDrawDestroy(playerid, AdminHelpBtn[playerid][i]);
            AdminHelpBtnOn[playerid][i] = 0;
        }
    }

    AdminCmdHudOn[playerid] = 0;

    if (cancel && AdminHelpSelecting[playerid])
    {
        AdminHelpSelecting[playerid] = 0;
        CancelSelectTextDraw(playerid);
    }

    return 1;
}

stock PlayerText:AdminX_MakeButton(playerid, Float:x, Float:y, const label[], color, Float:right)
{
    new buffer[8];

    format(buffer, sizeof buffer, "%s", label);

    new PlayerText:td = CreatePlayerTextDraw(playerid, x, y, buffer);
    PlayerTextDrawLetterSize(playerid, td, 0.45, 1.5);
    PlayerTextDrawTextSize(playerid, td, right, 14.0);
    PlayerTextDrawFont(playerid, td, 1);
    PlayerTextDrawColor(playerid, td, color);
    PlayerTextDrawSetOutline(playerid, td, 1);
    PlayerTextDrawSetProportional(playerid, td, 1);
    PlayerTextDrawSetSelectable(playerid, td, 1);
    PlayerTextDrawShow(playerid, td);
    return td;
}

stock AdminX_HelpShow(playerid, page)
{
    new level = PlayerData[playerid][Player_Admin];
    new total = AdminX_HelpPages(level);
    new placeholder[] = "_";
    new text[256];

    if (page < 1) page = 1;
    if (page > total) page = total;

    if (!AdminCmdHudOn[playerid])
    {
        AdminHelpSelecting[playerid] = 0;
    }

    AdminX_HelpHide(playerid, 0);

    AdminHelpBtnOn[playerid][0] = 0;
    AdminHelpBtnOn[playerid][1] = 0;
    AdminHelpBtnOn[playerid][2] = 0;

    AdminX_HelpBuild(page, total, level, text, sizeof text);

    // 1 unit tinggi = sekitar 10.4 piksel (layar virtual 640x480), panel diposisikan di tengah
    new Float:textUnits = float(AdminX_HelpLines(text)) * 1.1;
    new Float:boxUnits = textUnits + 0.9 + 1.5;
    new Float:top = 240.0 - (boxUnits * 10.4) / 2.0;
    new Float:buttonY = top + 4.0 + textUnits * 10.4 + 2.0;

    printf("[ADMINX] /ahelp: %s halaman %d/%d (%d karakter)", PlayerData[playerid][Player_Name], page, total, strlen(text));

    AdminHelpBox[playerid] = CreatePlayerTextDraw(playerid, 210.0, top, placeholder);
    PlayerTextDrawLetterSize(playerid, AdminHelpBox[playerid], 0.0, boxUnits);
    PlayerTextDrawTextSize(playerid, AdminHelpBox[playerid], 430.0, 0.0);
    PlayerTextDrawUseBox(playerid, AdminHelpBox[playerid], 1);
    PlayerTextDrawBoxColor(playerid, AdminHelpBox[playerid], 0x000000CC);
    PlayerTextDrawShow(playerid, AdminHelpBox[playerid]);

    AdminHelpText[playerid] = CreatePlayerTextDraw(playerid, 218.0, top + 4.0, text);
    PlayerTextDrawLetterSize(playerid, AdminHelpText[playerid], 0.25, 1.1);
    PlayerTextDrawFont(playerid, AdminHelpText[playerid], 1);
    PlayerTextDrawColor(playerid, AdminHelpText[playerid], -1);
    PlayerTextDrawSetOutline(playerid, AdminHelpText[playerid], 1);
    PlayerTextDrawSetProportional(playerid, AdminHelpText[playerid], 1);
    PlayerTextDrawShow(playerid, AdminHelpText[playerid]);

    // tombol X (tutup)
    AdminHelpBtn[playerid][0] = AdminX_MakeButton(playerid, 410.0, top + 2.0, "X", 0xFF3333FF, 428.0);
    AdminHelpBtnOn[playerid][0] = 1;

    // tombol halaman sebelumnya
    if (page > 1)
    {
        AdminHelpBtn[playerid][1] = AdminX_MakeButton(playerid, 218.0, buttonY, "<<", 0x33FF33FF, 246.0);
        AdminHelpBtnOn[playerid][1] = 1;
    }

    // tombol halaman berikutnya
    if (page < total)
    {
        AdminHelpBtn[playerid][2] = AdminX_MakeButton(playerid, 398.0, buttonY, ">>", 0x33FF33FF, 426.0);
        AdminHelpBtnOn[playerid][2] = 1;
    }

    AdminCmdHudOn[playerid] = 1;
    AdminHelpPage[playerid] = page;

    if (!AdminHelpSelecting[playerid])
    {
        AdminHelpSelecting[playerid] = 1;
        SelectTextDraw(playerid, 0xFF4040FF);
    }

    return 1;
}

stock AdminX_CmdHelp(playerid, const cmdtext[], pos)
{
    if (!Admin_Check(playerid, 1, 0))
    {
        return 1;
    }

    new token[8];

    Admin_Token(cmdtext, pos, token);

    if (Admin_IsNumeric(token))
    {
        AdminX_HelpShow(playerid, strval(token));
        return 1;
    }

    if (AdminCmdHudOn[playerid])
    {
        AdminX_HelpHide(playerid, 1);
        return 1;
    }

    AdminX_HelpShow(playerid, 1);
    return 1;
}

// pemain menekan ESC / back saat mode kursor aktif
stock AdminX_OnClickTextDraw(playerid, Text:clickedid)
{
    if (clickedid == Text:INVALID_TEXT_DRAW && AdminCmdHudOn[playerid])
    {
        AdminHelpSelecting[playerid] = 0;
        AdminX_HelpHide(playerid, 0);
        return 1;
    }

    return 0;
}

// pemain menekan tombol X / << / >>
stock AdminX_OnClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    if (!AdminCmdHudOn[playerid])
    {
        return 0;
    }

    if (AdminHelpBtnOn[playerid][0] && playertextid == AdminHelpBtn[playerid][0])
    {
        AdminX_HelpHide(playerid, 1);
        return 1;
    }

    if (AdminHelpBtnOn[playerid][1] && playertextid == AdminHelpBtn[playerid][1])
    {
        AdminX_HelpShow(playerid, AdminHelpPage[playerid] - 1);
        return 1;
    }

    if (AdminHelpBtnOn[playerid][2] && playertextid == AdminHelpBtn[playerid][2])
    {
        AdminX_HelpShow(playerid, AdminHelpPage[playerid] + 1);
        return 1;
    }

    return 0;
}













// isi tiap section panel, return jumlah baris (0 = section tidak ditampilkan untuk level ini)




public AdminX_Timer()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !AdminSpecOn[i])
        {
            continue;
        }

        new target = AdminSpecTarget[i];

        if (target < 0 || target >= MAX_PLAYERS || !IsPlayerConnected(target) || !PlayerData[target][Player_LoggedIn])
        {
            SendClientMessage(i, ADMIN_COLOR_INFO, "[ADMIN] Pemain yang kamu spec sudah keluar, spec dihentikan.");
            AdminX_StopSpec(i);
        }
    }

    return 1;
}

stock AdminX_Init()
{
    new timerFn[] = "AdminX_Timer";
    SetTimer(timerFn, 1000, true);

    Database_Exec("CREATE TABLE IF NOT EXISTS hc_bans (id INT UNSIGNED NOT NULL AUTO_INCREMENT, name VARCHAR(24) NOT NULL, ip VARCHAR(16) NOT NULL, reason VARCHAR(96) NOT NULL, banned_by VARCHAR(24) NOT NULL, created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (id), KEY idx_ban_name (name), KEY idx_ban_ip (ip)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    print("[ADMIN] Admin commands (extra) initialized.");
    return 1;
}

stock AdminX_OnConnect(playerid)
{
    AdminMuted[playerid] = 0;
    AdminWarns[playerid] = 0;
    AdminSpecOn[playerid] = 0;
    AdminSpecReturn[playerid] = 0;
    AdminCmdHudOn[playerid] = 0;
    AdminVehicle[playerid] = 0;

    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new ip[16];
    new query[256];

    GetPlayerIp(playerid, ip, sizeof ip);

    #if ADMINX_BAN_BY_IP
        mysql_format(
            HopeCityDB,
            query,
            sizeof query,
            "SELECT reason, banned_by FROM hc_bans WHERE name = '%e' OR ip = '%e' LIMIT 1",
            PlayerData[playerid][Player_Name],
            ip
        );
    #else
        mysql_format(
            HopeCityDB,
            query,
            sizeof query,
            "SELECT reason, banned_by FROM hc_bans WHERE name = '%e' LIMIT 1",
            PlayerData[playerid][Player_Name]
        );
    #endif

    mysql_tquery(HopeCityDB, query, "AdminX_OnBanCheck", "dd", playerid, AuthSession[playerid]);
    return 1;
}

stock AdminX_OnSpawn(playerid)
{
    if (AdminSpecReturn[playerid])
    {
        AdminSpecReturn[playerid] = 0;

        SetPlayerPos(playerid, AdminSpecData[playerid][0], AdminSpecData[playerid][1], AdminSpecData[playerid][2]);
        SetPlayerFacingAngle(playerid, AdminSpecData[playerid][3]);
        SetPlayerInterior(playerid, AdminSpecInterior[playerid]);
        SetPlayerVirtualWorld(playerid, AdminSpecWorld[playerid]);
        SetPlayerHealth(playerid, AdminSpecData[playerid][4]);
        SetPlayerArmour(playerid, AdminSpecData[playerid][5]);
    }

    return 1;
}

// dipanggil SEBELUM Database_SavePlayer supaya data yang tersimpan benar
stock AdminX_OnDisconnect(playerid)
{
    if (AdminSpecOn[playerid])
    {
        SetPlayerInterior(playerid, AdminSpecInterior[playerid]);
        SetPlayerVirtualWorld(playerid, AdminSpecWorld[playerid]);
    }

    if (AdminVehicle[playerid] != 0)
    {
        DestroyVehicle(AdminVehicle[playerid]);
        AdminVehicle[playerid] = 0;
    }

    AdminMuted[playerid] = 0;
    AdminWarns[playerid] = 0;
    AdminSpecOn[playerid] = 0;
    AdminSpecReturn[playerid] = 0;
    AdminCmdHudOn[playerid] = 0;
    return 1;
}

// return 0 = chat diblok
stock AdminX_OnText(playerid)
{
    if (AdminMuted[playerid])
    {
        SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Kamu sedang di-mute oleh admin.");
        return 0;
    }

    return 1;
}

// return 1 kalau command dikenali & ditangani, 0 kalau bukan (lanjut ke Admin_OnCommandText)
stock AdminX_OnCommandText(playerid, const cmdtext[])
{
    if (PlayerData[playerid][Player_Admin] < 1 && !IsPlayerAdmin(playerid))
    {
        return 0;
    }

    new cmd[24];
    new pos = 1;

    Admin_Token(cmdtext, pos, cmd);

    if (!strcmp(cmd, "ahelp", true)) return AdminX_CmdHelp(playerid, cmdtext, pos);
    if (!strcmp(cmd, "players", true)) return AdminX_CmdPlayers(playerid);
    if (!strcmp(cmd, "acheck", true)) return AdminX_CmdACheck(playerid, cmdtext, pos);
    if (!strcmp(cmd, "goto", true)) return AdminX_CmdGotoPlayer(playerid, cmdtext, pos, false);
    if (!strcmp(cmd, "gethere", true)) return AdminX_CmdGotoPlayer(playerid, cmdtext, pos, true);
    if (!strcmp(cmd, "gotols", true)) return AdminX_CmdGotoPlace(playerid, 0);
    if (!strcmp(cmd, "gotolv", true)) return AdminX_CmdGotoPlace(playerid, 1);
    if (!strcmp(cmd, "gotosf", true)) return AdminX_CmdGotoPlace(playerid, 2);
    if (!strcmp(cmd, "gotoco", true)) return AdminX_CmdGotoCo(playerid, cmdtext, pos);
    if (!strcmp(cmd, "sendto", true)) return AdminX_CmdSendTo(playerid, cmdtext, pos);
    if (!strcmp(cmd, "spec", true)) return AdminX_CmdSpec(playerid, cmdtext, pos);
    if (!strcmp(cmd, "specoff", true)) return AdminX_CmdSpecOff(playerid);
    if (!strcmp(cmd, "mute", true)) return AdminX_CmdMute(playerid, cmdtext, pos, 1);
    if (!strcmp(cmd, "unmute", true)) return AdminX_CmdMute(playerid, cmdtext, pos, 0);
    if (!strcmp(cmd, "warn", true)) return AdminX_CmdWarn(playerid, cmdtext, pos);
    if (!strcmp(cmd, "slap", true)) return AdminX_CmdSlap(playerid, cmdtext, pos);
    if (!strcmp(cmd, "spawn", true)) return AdminX_CmdSpawn(playerid, cmdtext, pos);
    if (!strcmp(cmd, "clearchat", true)) return AdminX_CmdClearChat(playerid);
    if (!strcmp(cmd, "ban", true)) return AdminX_CmdBan(playerid, cmdtext, pos);
    if (!strcmp(cmd, "unban", true)) return AdminX_CmdUnban(playerid, cmdtext, pos);
    if (!strcmp(cmd, "setweather", true)) return AdminX_CmdWorld(playerid, cmdtext, pos, 0);
    if (!strcmp(cmd, "settime", true)) return AdminX_CmdWorld(playerid, cmdtext, pos, 1);
    if (!strcmp(cmd, "veh", true)) return AdminX_CmdVeh(playerid, cmdtext, pos);
    if (!strcmp(cmd, "fixveh", true)) return AdminX_CmdFixVeh(playerid);

    return 0;
}

// ---------------------- TP lewat peta (wajib duty) ----------------------
public OnPlayerClickMap(playerid, Float:fX, Float:fY, Float:fZ)
{
#pragma unused fZ
if (PlayerData[playerid][Player_Admin] < 1)
{
return 1;
}
if (!AdminX_CheckTP(playerid, 1))
{
return 1;
}
if (GetPlayerInterior(playerid) != 0)
{
SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Tidak bisa TP peta saat di dalam interior.");
return 1;
}
if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
{
SendClientMessage(playerid, ADMIN_COLOR_ERROR, "Turun dari kendaraan dulu sebelum TP peta.");
return 1;
}
SetPlayerPosFindZ(playerid, fX, fY, 999.0);
SendClientMessage(playerid, ADMIN_COLOR_INFO, "[ADMIN] Teleport lewat peta berhasil.");
return 1;
}
