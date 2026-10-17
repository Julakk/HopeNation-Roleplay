#if defined _HOPECITY_AUTH
    #endinput
#endif
#define _HOPECITY_AUTH

#define DIALOG_AUTH 1000
#define DIALOG_REGISTER 1001
#define DIALOG_LOGIN 1002

stock Auth_Init()
{
    print("[AUTH] Authentication system initialized.");
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

    ShowPlayerDialog(
        playerid,
        DIALOG_AUTH,
        DIALOG_STYLE_MSGBOX,
        "HopeCity Roleplay - Authentication",
        "Selamat datang di HopeCity Roleplay!\\n\\nSilakan pilih Login jika sudah memiliki akun atau Register untuk membuat akun baru.",
        "Login",
        "Register"
    );

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

        SendClientMessage(playerid, 0x00FF00FF, "Registrasi berhasil. Silakan login.");
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

        if (strlen(inputtext) < 4)
        {
            SendClientMessage(playerid, 0xFF0000FF, "Password tidak valid.");
            Auth_ShowLogin(playerid);
            return 1;
        }

        PlayerData[playerid][Player_LoggedIn] = 1;

        SendClientMessage(playerid, 0x00FF00FF, "Login berhasil!");
        SendClientMessage(playerid, 0xFFFFFFFF, "Selamat bermain di HopeCity Roleplay.");
        return 1;
    }

    return 0;
}
