#if defined _HOPECITY_AUTH
    #endinput
#endif
#define _HOPECITY_AUTH

#define DIALOG_AUTH 1000
#define DIALOG_REGISTER 1001
#define DIALOG_LOGIN 1002

#define AUTH_SALT_LEN 16
#define AUTH_MAX_ATTEMPTS 3
#define AUTH_MIN_PASSWORD 6
#define AUTH_MAX_PASSWORD 32

#define AUTH_LOCK_SECONDS 120
#define AUTH_LOCK_SLOTS 64
#define AUTH_KICK_DELAY 1500

new AuthLoginAttempts[MAX_PLAYERS];
new AuthSession[MAX_PLAYERS];
new AuthBusy[MAX_PLAYERS];

new AuthLockIP[AUTH_LOCK_SLOTS][16];
new AuthLockName[AUTH_LOCK_SLOTS][MAX_PLAYER_NAME + 1];
new AuthLockUntil[AUTH_LOCK_SLOTS];
new AuthLockNext;

forward Auth_ShowWelcome(playerid, session);
forward Auth_KickPlayer(playerid, session);

stock Auth_StartTimer(const func[], delay, playerid, session)
{
    new name[32];
    format(name, sizeof name, "%s", func);
    return SetTimerEx(name, delay, false, "dd", playerid, session);
}

stock Auth_Init()
{
    print("[AUTH] Authentication system initialized.");
    return 1;
}

stock Auth_ResetPlayer(playerid)
{
    AuthLoginAttempts[playerid] = 0;
    AuthBusy[playerid] = 0;
    AuthSession[playerid]++;
    return 1;
}

stock Auth_MakeSalt(salt[])
{
    static const chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (new i = 0; i < AUTH_SALT_LEN; i++)
    {
        salt[i] = chars[random(sizeof chars - 1)];
    }

    salt[AUTH_SALT_LEN] = EOS;
    return 1;
}

// ---------- lock sementara setelah salah password ----------
stock Auth_LockRemaining(playerid)
{
    new ip[16];
    new now = gettime();

    GetPlayerIp(playerid, ip, sizeof ip);

    for (new i = 0; i < AUTH_LOCK_SLOTS; i++)
    {
        if (AuthLockUntil[i] <= now)
        {
            continue;
        }

        if (!strcmp(AuthLockIP[i], ip) && !strcmp(AuthLockName[i], PlayerData[playerid][Player_Name], true))
        {
            return AuthLockUntil[i] - now;
        }
    }

    return 0;
}

stock Auth_LockPlayer(playerid)
{
    new ip[16];
    new now = gettime();
    new slot = -1;

    GetPlayerIp(playerid, ip, sizeof ip);

    for (new i = 0; i < AUTH_LOCK_SLOTS; i++)
    {
        if (AuthLockUntil[i] <= now)
        {
            slot = i;
            break;
        }

        if (!strcmp(AuthLockIP[i], ip) && !strcmp(AuthLockName[i], PlayerData[playerid][Player_Name], true))
        {
            slot = i;
            break;
        }
    }

    if (slot == -1)
    {
        slot = AuthLockNext;
        AuthLockNext = (AuthLockNext + 1) % AUTH_LOCK_SLOTS;
    }

    format(AuthLockIP[slot], 16, "%s", ip);
    format(AuthLockName[slot], MAX_PLAYER_NAME + 1, "%s", PlayerData[playerid][Player_Name]);
    AuthLockUntil[slot] = now + AUTH_LOCK_SECONDS;
    return 1;
}

public Auth_KickPlayer(playerid, session)
{
    if (!IsPlayerConnected(playerid) || AuthSession[playerid] != session)
    {
        return 0;
    }

    Kick(playerid);
    return 1;
}

// ---------- dialog ----------
stock Auth_ShowRegister(playerid)
{
    ShowPlayerDialog(
        playerid,
        DIALOG_REGISTER,
        DIALOG_STYLE_PASSWORD,
        "HopeCity Roleplay - Register",
        "Buat password untuk akun kamu (6-32 karakter):",
        "Register",
        "Keluar"
    );

    return 1;
}

stock Auth_ShowLogin(playerid)
{
    ShowPlayerDialog(
        playerid,
        DIALOG_LOGIN,
        DIALOG_STYLE_PASSWORD,
        "HopeCity Roleplay - Login",
        "Masukkan password akun kamu:",
        "Login",
        "Keluar"
    );

    return 1;
}

public Auth_ShowWelcome(playerid, session)
{
    if (!IsPlayerConnected(playerid) || AuthSession[playerid] != session)
    {
        return 0;
    }

    ShowPlayerDialog(
        playerid,
        DIALOG_AUTH,
        DIALOG_STYLE_MSGBOX,
        "HopeCity Roleplay - Authentication",
        "Selamat datang di HopeCity Roleplay!\n\nSilakan pilih Login jika sudah memiliki akun atau Register untuk membuat akun baru.",
        "Login",
        "Register"
    );

    return 1;
}

stock Auth_OnConnect(playerid)
{
    PlayerData[playerid][Player_LoggedIn] = 0;
    Auth_ResetPlayer(playerid);

    new remaining = Auth_LockRemaining(playerid);

    if (remaining > 0)
    {
        new msg[144];
        format(msg, sizeof msg, "Akun ini dikunci sementara karena terlalu banyak salah password. Coba lagi dalam %d detik.", remaining);
        SendClientMessage(playerid, 0xFF0000FF, msg);

        AuthBusy[playerid] = 1;
        Auth_StartTimer("Auth_KickPlayer", AUTH_KICK_DELAY, playerid, AuthSession[playerid]);
        return 1;
    }

    // delay sedikit supaya dialog pasti muncul di client
    Auth_StartTimer("Auth_ShowWelcome", 800, playerid, AuthSession[playerid]);
    return 1;
}

stock Auth_OnDisconnect(playerid)
{
    Auth_ResetPlayer(playerid);
    return 1;
}

// ---------- callback async: hasil register ----------
public OnAccountRegister(playerid, session)
{
    if (!IsPlayerConnected(playerid) || AuthSession[playerid] != session)
    {
        return 0;
    }

    AuthBusy[playerid] = 0;

    if (cache_insert_id() <= 0)
    {
        SendClientMessage(playerid, 0xFF0000FF, "Gagal membuat akun. Nama kamu mungkin sudah terdaftar.");
        Auth_ShowLogin(playerid);
        return 1;
    }

    SendClientMessage(playerid, 0x00FF00FF, "Registrasi berhasil!");
    SendClientMessage(playerid, 0xFFFFFFFF, "Silakan login menggunakan password yang baru dibuat.");
    Auth_ShowLogin(playerid);
    return 1;
}

// ---------- callback async: hasil login ----------
public OnAccountLogin(playerid, session, password[])
{
    if (!IsPlayerConnected(playerid) || AuthSession[playerid] != session)
    {
        return 0;
    }

    AuthBusy[playerid] = 0;

    new rows;
    cache_get_row_count(rows);

    if (rows <= 0)
    {
        SendClientMessage(playerid, 0xFF0000FF, "Akun belum terdaftar.");
        Auth_ShowRegister(playerid);
        return 1;
    }

    new stored[96];
    Database_ReadHash(stored, sizeof stored);

    new sep = strfind(stored, "$");

    if (sep != AUTH_SALT_LEN)
    {
        SendClientMessage(playerid, 0xFF0000FF, "Data akun tidak valid. Hubungi admin.");
        AuthBusy[playerid] = 1;
        Auth_StartTimer("Auth_KickPlayer", AUTH_KICK_DELAY, playerid, AuthSession[playerid]);
        return 1;
    }

    new salt[AUTH_SALT_LEN + 1];
    new hash[65];
    new check[65];

    strmid(salt, stored, 0, sep, sizeof salt);
    strmid(hash, stored, sep + 1, strlen(stored), sizeof hash);
    SHA256_PassHash(password, salt, check, sizeof check);

    if (!strcmp(check, hash))
    {
        Database_ReadPlayerRow(playerid);

        PlayerData[playerid][Player_LoggedIn] = 1;
        AuthLoginAttempts[playerid] = 0;

        Database_TouchLogin(playerid);

        SendClientMessage(playerid, 0x00FF00FF, "Login berhasil!");
        SendClientMessage(playerid, 0xFFFFFFFF, "Selamat bermain di HopeCity Roleplay.");

        SetSpawnInfo(
            playerid,
            0,
            PlayerData[playerid][Player_Skin],
            PlayerData[playerid][Player_PosX],
            PlayerData[playerid][Player_PosY],
            PlayerData[playerid][Player_PosZ],
            PlayerData[playerid][Player_Angle],
            0, 0, 0, 0, 0, 0
        );
        SpawnPlayer(playerid);
        return 1;
    }

    if (++AuthLoginAttempts[playerid] >= AUTH_MAX_ATTEMPTS)
    {
        Auth_LockPlayer(playerid);

        new msg[144];
        format(msg, sizeof msg, "Terlalu banyak percobaan. Kamu di-kick dan akun dikunci %d detik.", AUTH_LOCK_SECONDS);
        SendClientMessage(playerid, 0xFF0000FF, msg);

        AuthBusy[playerid] = 1;
        Auth_StartTimer("Auth_KickPlayer", AUTH_KICK_DELAY, playerid, AuthSession[playerid]);
        return 1;
    }

    SendClientMessage(playerid, 0xFF0000FF, "Password salah.");
    Auth_ShowLogin(playerid);
    return 1;
}

// ---------- dialog response ----------
stock Auth_OnDialogResponse(playerid, dialogid, response, inputtext[])
{
    if (dialogid == DIALOG_AUTH)
    {
        if (AuthBusy[playerid])
        {
            return 1;
        }

        if (response)
        {
            Auth_ShowLogin(playerid);
        }
        else
        {
            Auth_ShowRegister(playerid);
        }

        return 1;
    }

    if (dialogid == DIALOG_REGISTER)
    {
        if (!response)
        {
            Kick(playerid);
            return 1;
        }

        if (AuthBusy[playerid])
        {
            return 1;
        }

        new len = strlen(inputtext);

        if (len < AUTH_MIN_PASSWORD || len > AUTH_MAX_PASSWORD)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password harus 6-32 karakter.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        new salt[AUTH_SALT_LEN + 1];
        new hash[65];
        new stored[96];

        Auth_MakeSalt(salt);
        SHA256_PassHash(inputtext, salt, hash, sizeof hash);
        format(stored, sizeof stored, "%s$%s", salt, hash);

        AuthBusy[playerid] = 1;

        if (!Database_RegisterQuery(playerid, AuthSession[playerid], stored))
        {
            AuthBusy[playerid] = 0;
            SendClientMessage(playerid, 0xFF0000FF, "Database sedang bermasalah. Coba lagi nanti.");
            Auth_ShowRegister(playerid);
        }

        return 1;
    }

    if (dialogid == DIALOG_LOGIN)
    {
        if (!response)
        {
            Kick(playerid);
            return 1;
        }

        if (AuthBusy[playerid])
        {
            return 1;
        }

        if (strlen(inputtext) < 1 || strlen(inputtext) > AUTH_MAX_PASSWORD)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password tidak valid.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        AuthBusy[playerid] = 1;

        if (!Database_LoginQuery(playerid, AuthSession[playerid], inputtext))
        {
            AuthBusy[playerid] = 0;
            SendClientMessage(playerid, 0xFF0000FF, "Database sedang bermasalah. Coba lagi nanti.");
            Auth_ShowLogin(playerid);
        }

        return 1;
    }

    return 0;
}
