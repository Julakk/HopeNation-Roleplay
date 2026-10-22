#if defined _HOPECITY_AUTH
    #endinput
#endif
#define _HOPECITY_AUTH

#include <async_crypto>

#define DIALOG_AUTH 1000
#define DIALOG_REGISTER 1001
#define DIALOG_LOGIN 1002

new AuthHashRequest[MAX_PLAYERS];
new AuthVerifyRequest[MAX_PLAYERS];
new AuthLoginAttempts[MAX_PLAYERS];

new AuthPendingName[MAX_PLAYERS][MAX_PLAYER_NAME];
new AuthPendingPassword[MAX_PLAYERS][128];

forward Auth_OnPasswordHashed(requestid, status, const result[]);
forward Auth_OnPasswordVerified(requestid, status, const result[]);

stock Auth_Init()
{
    print("[AUTH] Authentication system initialized.");
    return 1;
}

stock Auth_ResetPlayer(playerid)
{
    AuthLoginAttempts[playerid] = 0;
    AuthHashRequest[playerid] = 0;
    AuthVerifyRequest[playerid] = 0;
    AuthPendingName[playerid][0] = EOS;
    AuthPendingPassword[playerid][0] = EOS;
    return 1;
}

stock Auth_ShowRegister(playerid)
{
    ShowPlayerDialog(
        playerid,
        DIALOG_REGISTER,
        DIALOG_STYLE_PASSWORD,
        "HopeCity Roleplay - Register",
        "Buat password untuk akun kamu:",
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

stock Auth_FindHashPlayer(requestid)
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (AuthHashRequest[playerid] == requestid)
        {
            return playerid;
        }
    }

    return INVALID_PLAYER_ID;
}

stock Auth_FindVerifyPlayer(requestid)
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (AuthVerifyRequest[playerid] == requestid)
        {
            return playerid;
        }
    }

    return INVALID_PLAYER_ID;
}

public Auth_OnPasswordHashed(requestid, status, const result[])
{
    new playerid = Auth_FindHashPlayer(requestid);

    if (playerid == INVALID_PLAYER_ID)
    {
        return 1;
    }

    AuthHashRequest[playerid] = 0;

    if (status != CRYPTO_STATUS_OK)
    {
        SendClientMessage(playerid, 0xFF0000FF, "Gagal membuat password hash. Silakan coba lagi.");
        Auth_ShowRegister(playerid);
        return 1;
    }

    if (!Database_SaveAccount(AuthPendingName[playerid], result))
    {
        SendClientMessage(playerid, 0xFF0000FF, "Gagal menyimpan akun. Silakan coba lagi.");
        Auth_ShowRegister(playerid);
        return 1;
    }

    SendClientMessage(playerid, 0x00FF00FF, "Registrasi berhasil!");
    SendClientMessage(playerid, 0xFFFFFFFF, "Silakan login menggunakan password yang baru dibuat.");

    AuthPendingName[playerid][0] = EOS;
    Auth_ShowLogin(playerid);

    return 1;
}

public Auth_OnPasswordVerified(requestid, status, const result[])
{
    new playerid = Auth_FindVerifyPlayer(requestid);

    if (playerid == INVALID_PLAYER_ID)
    {
        return 1;
    }

    AuthVerifyRequest[playerid] = 0;
    AuthPendingPassword[playerid][0] = EOS;

    if (status != CRYPTO_STATUS_OK)
    {
        SendClientMessage(playerid, 0xFF0000FF, "Gagal memverifikasi password. Silakan coba lagi.");
        Auth_ShowLogin(playerid);
        return 1;
    }

    if (!strcmp(result, "1", true))
    {
        PlayerData[playerid][Player_LoggedIn] = 1;

        SendClientMessage(playerid, 0x00FF00FF, "Login berhasil!");
        SendClientMessage(playerid, 0xFFFFFFFF, "Selamat bermain di HopeCity Roleplay.");

        SetSpawnInfo(playerid, 0, 0, 1481.0, -1771.0, 18.8, 90.0, 0, 0, 0, 0, 0, 0);
        SpawnPlayer(playerid);
    }
    else
    {
        if (++AuthLoginAttempts[playerid] >= 3)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Terlalu banyak percobaan. Kamu di-kick.");
            Kick(playerid);
            return 1;
        }

        SendClientMessage(playerid, 0xFF0000FF, "Password salah.");
        Auth_ShowLogin(playerid);
    }

    return 1;
}

stock Auth_OnDialogResponse(playerid, dialogid, response, const inputtext[])
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

        if (strlen(inputtext) < 4)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password minimal 4 karakter.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        if (Database_AccountExists(PlayerData[playerid][Player_Name]))
        {
            SendClientMessage(playerid, 0xFF0000FF, "Akun dengan nama tersebut sudah terdaftar.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        format(
            AuthPendingName[playerid],
            MAX_PLAYER_NAME,
            "%s",
            PlayerData[playerid][Player_Name]
        );

        AuthHashRequest[playerid] = Crypto_Argon2Hash(
            inputtext,
            "Auth_OnPasswordHashed"
        );

        if (AuthHashRequest[playerid] <= 0)
        {
            AuthHashRequest[playerid] = 0;
            SendClientMessage(playerid, 0xFF0000FF, "Gagal memulai proses registrasi.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        SendClientMessage(playerid, 0xFFFFFFFF, "Password sedang diproses dengan aman...");
        return 1;
    }

    if (dialogid == DIALOG_LOGIN)
    {
        if (!response)
        {
            Kick(playerid);
            return 1;
        }

        if (strlen(inputtext) < 4)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password tidak valid.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        new password_hash[256];

        if (!Database_GetPasswordHash(
            PlayerData[playerid][Player_Name],
            password_hash,
            sizeof password_hash
        ))
        {
            SendClientMessage(playerid, 0xFF0000FF, "Akun belum terdaftar.");
            Auth_ShowRegister(playerid);
            return 1;
        }

        format(
            AuthPendingPassword[playerid],
            sizeof AuthPendingPassword[],
            "%s",
            inputtext
        );

        AuthVerifyRequest[playerid] = Crypto_Argon2Verify(
            inputtext,
            password_hash,
            "Auth_OnPasswordVerified"
        );

        if (AuthVerifyRequest[playerid] <= 0)
        {
            AuthVerifyRequest[playerid] = 0;
            AuthPendingPassword[playerid][0] = EOS;

            SendClientMessage(playerid, 0xFF0000FF, "Gagal memulai verifikasi password.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        SendClientMessage(playerid, 0xFFFFFFFF, "Password sedang diverifikasi...");
        return 1;
    }

    return 0;
}
