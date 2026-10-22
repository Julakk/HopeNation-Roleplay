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

new AuthLoginAttempts[MAX_PLAYERS];

stock Auth_Init()
{
    print("[AUTH] Authentication system initialized.");
    return 1;
}

stock Auth_ResetPlayer(playerid)
{
    AuthLoginAttempts[playerid] = 0;
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

stock Auth_ShowRegister(playerid)
{
    ShowPlayerDialog(
        playerid,
        DIALOG_REGISTER,
        DIALOG_STYLE_PASSWORD,
        "HopeCity Roleplay - Register",
        "Buat password untuk akun kamu (minimal 6 karakter):",
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

stock Auth_OnConnect(playerid)
{
    PlayerData[playerid][Player_LoggedIn] = 0;
    Auth_ResetPlayer(playerid);

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

stock Auth_OnDisconnect(playerid)
{
    Auth_ResetPlayer(playerid);
    return 1;
}

stock Auth_OnDialogResponse(playerid, dialogid, response, inputtext[])
{
    if (dialogid == DIALOG_AUTH)
    {
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

        if (strlen(inputtext) < AUTH_MIN_PASSWORD)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password minimal 6 karakter.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        if (Database_AccountExists(PlayerData[playerid][Player_Name]))
        {
            SendClientMessage(playerid, 0xFF0000FF, "Akun dengan nama tersebut sudah terdaftar.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        new salt[AUTH_SALT_LEN + 1];
        new hash[65];
        new stored[96];

        Auth_MakeSalt(salt);
        SHA256_PassHash(inputtext, salt, hash, sizeof hash);
        format(stored, sizeof stored, "%s$%s", salt, hash);

        if (!Database_SaveAccount(PlayerData[playerid][Player_Name], stored))
        {
            SendClientMessage(playerid, 0xFF0000FF, "Gagal menyimpan akun. Silakan coba lagi.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        SendClientMessage(playerid, 0x00FF00FF, "Registrasi berhasil!");
        SendClientMessage(playerid, 0xFFFFFFFF, "Silakan login menggunakan password yang baru dibuat.");
        Auth_ShowLogin(playerid);
        return 1;
    }

    if (dialogid == DIALOG_LOGIN)
    {
        if (!response)
        {
            Kick(playerid);
            return 1;
        }

        new stored[96];

        if (!Database_GetPasswordHash(
            PlayerData[playerid][Player_Name],
            stored,
            sizeof stored
        ))
        {
            SendClientMessage(playerid, 0xFF0000FF, "Akun belum terdaftar.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        new sep = strfind(stored, "$");

        if (sep != AUTH_SALT_LEN)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Data akun tidak valid. Hubungi admin.");
            Kick(playerid);
            return 1;
        }

        new salt[AUTH_SALT_LEN + 1];
        new hash[65];
        new check[65];

        strmid(salt, stored, 0, sep, sizeof salt);
        strmid(hash, stored, sep + 1, strlen(stored), sizeof hash);
        SHA256_PassHash(inputtext, salt, check, sizeof check);

        if (!strcmp(check, hash))
        {
            PlayerData[playerid][Player_LoggedIn] = 1;
            AuthLoginAttempts[playerid] = 0;

            SendClientMessage(playerid, 0x00FF00FF, "Login berhasil!");
            SendClientMessage(playerid, 0xFFFFFFFF, "Selamat bermain di HopeCity Roleplay.");

            Database_LoadPlayer(playerid);

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
            SendClientMessage(playerid, 0xFF0000FF, "Terlalu banyak percobaan. Kamu di-kick.");
            Kick(playerid);
            return 1;
        }

        SendClientMessage(playerid, 0xFF0000FF, "Password salah.");
        Auth_ShowLogin(playerid);
        return 1;
    }

    return 0;
}
