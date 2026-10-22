#if defined _HOPECITY_DATABASE
    #endinput
#endif
#define _HOPECITY_DATABASE

#include <a_mysql>

new MySQL:HopeCityDB;

stock Database_Exec(const query[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    mysql_query(HopeCityDB, query, false);
    return (mysql_errno(HopeCityDB) == 0);
}

stock Database_Init()
{
    HopeCityDB = mysql_connect_file();

    if (HopeCityDB == MySQL:0 || mysql_errno(HopeCityDB) != 0)
    {
        print("[DATABASE] Gagal konek MySQL. Cek mysql.ini, plugin mysql, dan log-core.");
        HopeCityDB = MySQL:0;
        return 0;
    }

    print("[DATABASE] MySQL connected.");

    new query[1024];
    query[0] = EOS;

    strcat(query, "CREATE TABLE IF NOT EXISTS hc_accounts (", sizeof query);
    strcat(query, "id INT UNSIGNED NOT NULL AUTO_INCREMENT, ", sizeof query);
    strcat(query, "name VARCHAR(24) NOT NULL, ", sizeof query);
    strcat(query, "password_hash VARCHAR(96) NOT NULL, ", sizeof query);
    strcat(query, "money INT NOT NULL DEFAULT 500, ", sizeof query);
    strcat(query, "skin SMALLINT NOT NULL DEFAULT 0, ", sizeof query);
    strcat(query, "interior SMALLINT NOT NULL DEFAULT 0, ", sizeof query);
    strcat(query, "health FLOAT NOT NULL DEFAULT 100, ", sizeof query);
    strcat(query, "armor FLOAT NOT NULL DEFAULT 0, ", sizeof query);
    strcat(query, "pos_x FLOAT NOT NULL DEFAULT 1481, ", sizeof query);
    strcat(query, "pos_y FLOAT NOT NULL DEFAULT -1771, ", sizeof query);
    strcat(query, "pos_z FLOAT NOT NULL DEFAULT 18.8, ", sizeof query);
    strcat(query, "angle FLOAT NOT NULL DEFAULT 90, ", sizeof query);
    strcat(query, "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, ", sizeof query);
    strcat(query, "PRIMARY KEY (id), UNIQUE KEY uq_hc_name (name)", sizeof query);
    strcat(query, ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", sizeof query);

    if (!Database_Exec(query))
    {
        print("[DATABASE] Gagal membuat tabel hc_accounts.");
        return 0;
    }

    print("[DATABASE] Accounts table ready.");
    return 1;
}

stock Database_AccountExists(const name[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new query[160];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "SELECT id FROM hc_accounts WHERE name = '%e' LIMIT 1",
        name
    );

    new Cache:result = mysql_query(HopeCityDB, query);

    if (result == Cache:0)
    {
        return 0;
    }

    new rows;
    cache_get_row_count(rows);
    cache_delete(result);

    return (rows > 0);
}

stock Database_GetPasswordHash(const name[], output[], output_size)
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new query[160];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "SELECT password_hash FROM hc_accounts WHERE name = '%e' LIMIT 1",
        name
    );

    new Cache:result = mysql_query(HopeCityDB, query);

    if (result == Cache:0)
    {
        return 0;
    }

    new rows;
    cache_get_row_count(rows);

    if (rows <= 0)
    {
        cache_delete(result);
        return 0;
    }

    cache_get_value_name(0, "password_hash", output, output_size);
    cache_delete(result);

    return 1;
}

stock Database_SaveAccount(const name[], const password_hash[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new query[256];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "INSERT INTO hc_accounts (name, password_hash) VALUES ('%e', '%e')",
        name,
        password_hash
    );

    return Database_Exec(query);
}

stock Database_LoadPlayer(playerid)
{
    Player_SetDefaults(playerid);

    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new query[256];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "SELECT money, skin, interior, health, armor, pos_x, pos_y, pos_z, angle FROM hc_accounts WHERE name = '%e' LIMIT 1",
        PlayerData[playerid][Player_Name]
    );

    new Cache:result = mysql_query(HopeCityDB, query);

    if (result == Cache:0)
    {
        return 0;
    }

    new rows;
    cache_get_row_count(rows);

    if (rows > 0)
    {
        new value;
        new Float:fvalue;

        cache_get_value_name_int(0, "money", value);
        PlayerData[playerid][Player_Money] = value;

        cache_get_value_name_int(0, "skin", value);
        PlayerData[playerid][Player_Skin] = value;

        cache_get_value_name_int(0, "interior", value);
        PlayerData[playerid][Player_Interior] = value;

        cache_get_value_name_float(0, "health", fvalue);
        PlayerData[playerid][Player_Health] = fvalue;

        cache_get_value_name_float(0, "armor", fvalue);
        PlayerData[playerid][Player_Armor] = fvalue;

        cache_get_value_name_float(0, "pos_x", fvalue);
        PlayerData[playerid][Player_PosX] = fvalue;

        cache_get_value_name_float(0, "pos_y", fvalue);
        PlayerData[playerid][Player_PosY] = fvalue;

        cache_get_value_name_float(0, "pos_z", fvalue);
        PlayerData[playerid][Player_PosZ] = fvalue;

        cache_get_value_name_float(0, "angle", fvalue);
        PlayerData[playerid][Player_Angle] = fvalue;
    }

    cache_delete(result);

    if (PlayerData[playerid][Player_Health] < 1.0)
    {
        PlayerData[playerid][Player_Health] = 100.0;
    }

    return 1;
}

stock Database_SavePlayer(playerid)
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    if (!PlayerData[playerid][Player_LoggedIn] || !PlayerData[playerid][Player_Spawned])
    {
        return 0;
    }

    new Float:x, Float:y, Float:z, Float:angle, Float:health, Float:armor;

    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, angle);
    GetPlayerHealth(playerid, health);
    GetPlayerArmour(playerid, armor);

    new query[512];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "UPDATE hc_accounts SET money = %d, skin = %d, interior = %d, health = %.1f, armor = %.1f, pos_x = %.3f, pos_y = %.3f, pos_z = %.3f, angle = %.1f WHERE name = '%e'",
        GetPlayerMoney(playerid),
        GetPlayerSkin(playerid),
        GetPlayerInterior(playerid),
        health,
        armor,
        x,
        y,
        z,
        angle,
        PlayerData[playerid][Player_Name]
    );

    mysql_tquery(HopeCityDB, query);
    return 1;
}

stock Database_Close()
{
    if (HopeCityDB != MySQL:0)
    {
        mysql_close(HopeCityDB);
        HopeCityDB = MySQL:0;
    }

    print("[DATABASE] MySQL connection closed.");
    return 1;
}
