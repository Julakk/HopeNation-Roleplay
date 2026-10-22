#if defined _HOPECITY_DATABASE
    #endinput
#endif
#define _HOPECITY_DATABASE

#include <a_mysql>

new MySQL:HopeCityDB;

forward OnAccountLogin(playerid, session, password[]);
forward OnAccountRegister(playerid, session);

stock Database_Exec(const query[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    mysql_query(HopeCityDB, query, false);
    return (mysql_errno(HopeCityDB) == 0);
}

stock Database_ColumnExists(const table[], const column[])
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
        "SELECT COUNT(*) AS c FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = '%e' AND COLUMN_NAME = '%e'",
        table,
        column
    );

    new Cache:result = mysql_query(HopeCityDB, query);

    if (result == Cache:0)
    {
        return 0;
    }

    new count = 0;
    cache_get_value_name_int(0, "c", count);
    cache_delete(result);

    return (count > 0);
}

stock Database_EnsureColumn(const table[], const column[], const definition[])
{
    if (Database_ColumnExists(table, column))
    {
        return 1;
    }

    new query[256];
    format(query, sizeof query, "ALTER TABLE %s ADD COLUMN %s %s", table, column, definition);

    if (!Database_Exec(query))
    {
        printf("[DATABASE] Gagal menambah kolom %s.%s", table, column);
        return 0;
    }

    printf("[DATABASE] Kolom %s.%s ditambahkan.", table, column);
    return 1;
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

    new query[1280];
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
    strcat(query, "last_login DATETIME NULL DEFAULT NULL, ", sizeof query);
    strcat(query, "last_ip VARCHAR(16) NULL DEFAULT NULL, ", sizeof query);
    strcat(query, "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, ", sizeof query);
    strcat(query, "PRIMARY KEY (id), UNIQUE KEY uq_hc_name (name)", sizeof query);
    strcat(query, ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", sizeof query);

    if (!Database_Exec(query))
    {
        print("[DATABASE] Gagal membuat tabel hc_accounts.");
        return 0;
    }

    // migrasi untuk tabel lama (aman dijalankan berulang)
    Database_EnsureColumn("hc_accounts", "last_login", "DATETIME NULL DEFAULT NULL");
    Database_EnsureColumn("hc_accounts", "last_ip", "VARCHAR(16) NULL DEFAULT NULL");
    Database_EnsureColumn("hc_accounts", "admin", "TINYINT UNSIGNED NOT NULL DEFAULT 0");

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

// ---------- async: login ----------
stock Database_LoginQuery(playerid, session, password[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new query[384];
    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "SELECT id, password_hash, admin, money, skin, interior, health, armor, pos_x, pos_y, pos_z, angle FROM hc_accounts WHERE name = '%e' LIMIT 1",
        PlayerData[playerid][Player_Name]
    );

    mysql_tquery(HopeCityDB, query, "OnAccountLogin", "dds", playerid, session, password);
    return 1;
}

// dipanggil DI DALAM callback OnAccountLogin (cache aktif)
stock Database_ReadHash(output[], output_size)
{
    cache_get_value_name(0, "password_hash", output, output_size);
    return 1;
}

// dipanggil DI DALAM callback OnAccountLogin setelah password cocok
stock Database_ReadPlayerRow(playerid)
{
    Player_SetDefaults(playerid);

    new value;
    new Float:fvalue;

    cache_get_value_name_int(0, "id", value);
    PlayerData[playerid][Player_ID] = value;
    cache_get_value_name_int(0, "admin", value);
    PlayerData[playerid][Player_Admin] = value;

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

    if (PlayerData[playerid][Player_Health] < 1.0)
    {
        PlayerData[playerid][Player_Health] = 100.0;
    }

    return 1;
}

stock Database_TouchLogin(playerid)
{
    if (HopeCityDB == MySQL:0 || PlayerData[playerid][Player_ID] <= 0)
    {
        return 0;
    }

    new ip[16], query[192];
    GetPlayerIp(playerid, ip, sizeof ip);

    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "UPDATE hc_accounts SET last_login = NOW(), last_ip = '%e' WHERE id = %d",
        ip,
        PlayerData[playerid][Player_ID]
    );

    mysql_tquery(HopeCityDB, query);
    return 1;
}

// ---------- async: register ----------
stock Database_RegisterQuery(playerid, session, const password_hash[])
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    new ip[16], query[320];
    GetPlayerIp(playerid, ip, sizeof ip);

    mysql_format(
        HopeCityDB,
        query,
        sizeof query,
        "INSERT INTO hc_accounts (name, password_hash, last_ip) VALUES ('%e', '%e', '%e')",
        PlayerData[playerid][Player_Name],
        password_hash,
        ip
    );

    mysql_tquery(HopeCityDB, query, "OnAccountRegister", "dd", playerid, session);
    return 1;
}

// ---------- save ----------
// sync = 1 dipakai saat server mati supaya data pasti tertulis sebelum koneksi ditutup
stock Database_SavePlayer(playerid, sync = 0)
{
    if (HopeCityDB == MySQL:0)
    {
        return 0;
    }

    if (!PlayerData[playerid][Player_LoggedIn] || !PlayerData[playerid][Player_Spawned] || PlayerData[playerid][Player_ID] <= 0)
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
        "UPDATE hc_accounts SET money = %d, skin = %d, interior = %d, health = %.1f, armor = %.1f, pos_x = %.3f, pos_y = %.3f, pos_z = %.3f, angle = %.1f WHERE id = %d",
        GetPlayerMoney(playerid),
        GetPlayerSkin(playerid),
        GetPlayerInterior(playerid),
        health,
        armor,
        x,
        y,
        z,
        angle,
        PlayerData[playerid][Player_ID]
    );

    if (sync)
    {
        return Database_Exec(query);
    }

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
