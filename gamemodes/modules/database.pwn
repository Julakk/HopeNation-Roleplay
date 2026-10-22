#if defined _HOPECITY_DATABASE
    #endinput
#endif
#define _HOPECITY_DATABASE

#include <a_sampdb>

new DB:HopeCityDB;

stock Database_Exec(const query[])
{
    if (HopeCityDB == DB:0)
    {
        return 0;
    }

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    db_free_result(result);
    return 1;
}

stock Database_ColumnExists(const table[], const column[])
{
    new query[96];
    new name[32];
    new found = 0;

    format(query, sizeof query, "PRAGMA table_info(%s)", table);

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    if (db_num_rows(result) > 0)
    {
        do
        {
            db_get_field(result, 1, name, sizeof name);

            if (!strcmp(name, column))
            {
                found = 1;
                break;
            }
        }
        while (db_next_row(result));
    }

    db_free_result(result);
    return found;
}

stock Database_Migrate()
{
    if (Database_ColumnExists("accounts", "money"))
    {
        return 1;
    }

    Database_Exec("ALTER TABLE accounts ADD COLUMN money INTEGER NOT NULL DEFAULT 500");
    Database_Exec("ALTER TABLE accounts ADD COLUMN skin INTEGER NOT NULL DEFAULT 0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN interior INTEGER NOT NULL DEFAULT 0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN health REAL NOT NULL DEFAULT 100.0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN armor REAL NOT NULL DEFAULT 0.0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN pos_x REAL NOT NULL DEFAULT 1481.0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN pos_y REAL NOT NULL DEFAULT -1771.0");
    Database_Exec("ALTER TABLE accounts ADD COLUMN pos_z REAL NOT NULL DEFAULT 18.8");
    Database_Exec("ALTER TABLE accounts ADD COLUMN angle REAL NOT NULL DEFAULT 90.0");

    print("[DATABASE] Player data columns added.");
    return 1;
}

stock Database_LoadPlayer(playerid)
{
    Player_SetDefaults(playerid);

    if (HopeCityDB == DB:0)
    {
        return 0;
    }

    new query[256];
    format(
        query,
        sizeof query,
        "SELECT money, skin, interior, health, armor, pos_x, pos_y, pos_z, angle FROM accounts WHERE name = '%s' LIMIT 1",
        PlayerData[playerid][Player_Name]
    );

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    if (db_num_rows(result) > 0)
    {
        PlayerData[playerid][Player_Money] = db_get_field_int(result, 0);
        PlayerData[playerid][Player_Skin] = db_get_field_int(result, 1);
        PlayerData[playerid][Player_Interior] = db_get_field_int(result, 2);
        PlayerData[playerid][Player_Health] = db_get_field_float(result, 3);
        PlayerData[playerid][Player_Armor] = db_get_field_float(result, 4);
        PlayerData[playerid][Player_PosX] = db_get_field_float(result, 5);
        PlayerData[playerid][Player_PosY] = db_get_field_float(result, 6);
        PlayerData[playerid][Player_PosZ] = db_get_field_float(result, 7);
        PlayerData[playerid][Player_Angle] = db_get_field_float(result, 8);
    }

    db_free_result(result);

    if (PlayerData[playerid][Player_Health] < 1.0)
    {
        PlayerData[playerid][Player_Health] = 100.0;
    }

    return 1;
}

stock Database_SavePlayer(playerid)
{
    if (HopeCityDB == DB:0)
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
    format(
        query,
        sizeof query,
        "UPDATE accounts SET money = %d, skin = %d, interior = %d, health = %.1f, armor = %.1f, pos_x = %.3f, pos_y = %.3f, pos_z = %.3f, angle = %.1f WHERE name = '%s'",
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

    return Database_Exec(query);
}

stock Database_Init()
{
    HopeCityDB = db_open("hopecity.db");

    if (HopeCityDB == DB:0)
    {
        print("[DATABASE] Failed to open SQLite database.");
        return 0;
    }

    print("[DATABASE] SQLite database opened.");

    new DBResult:result = db_query(
        HopeCityDB,
        "CREATE TABLE IF NOT EXISTS accounts (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, password_hash TEXT NOT NULL, created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')))"
    );

    if (result)
    {
        db_free_result(result);
    }

    Database_Migrate();

    print("[DATABASE] Accounts table ready.");
    return 1;
}

stock Database_AccountExists(const name[])
{
    if (HopeCityDB == DB:0)
    {
        return 0;
    }

    new query[256];
    format(
        query,
        sizeof query,
        "SELECT id FROM accounts WHERE name = '%s' LIMIT 1",
        name
    );

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    new exists = db_num_rows(result) > 0;

    db_free_result(result);
    return exists;
}

stock Database_GetPasswordHash(const name[], output[], output_size)
{
    if (HopeCityDB == DB:0)
    {
        return 0;
    }

    new query[256];
    format(
        query,
        sizeof query,
        "SELECT password_hash FROM accounts WHERE name = '%s' LIMIT 1",
        name
    );

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    if (db_num_rows(result) <= 0)
    {
        db_free_result(result);
        return 0;
    }

    db_get_field(result, 0, output, output_size);
    db_free_result(result);

    return 1;
}

stock Database_SaveAccount(const name[], const password_hash[])
{
    if (HopeCityDB == DB:0)
    {
        return 0;
    }

    new query[512];
    format(
        query,
        sizeof query,
        "INSERT INTO accounts (name, password_hash) VALUES ('%s', '%s')",
        name,
        password_hash
    );

    new DBResult:result = db_query(HopeCityDB, query);

    if (!result)
    {
        return 0;
    }

    db_free_result(result);
    return 1;
}

stock Database_Close()
{
    if (HopeCityDB != DB:0)
    {
        db_close(HopeCityDB);
        HopeCityDB = DB:0;
    }

    print("[DATABASE] SQLite database closed.");
    return 1;
}
