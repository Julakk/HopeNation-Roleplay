#if defined _HOPECITY_DATABASE
    #endinput
#endif
#define _HOPECITY_DATABASE

#include <a_sampdb>

new DB:HopeCityDB;

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
