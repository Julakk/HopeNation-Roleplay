# HopeNation-Roleplay

Modern & Lightweight Roleplay Gamemode (open.mp)

## Fitur
- Register dan login (SHA256 + salt unik per akun, maksimal 3 kali salah lalu di-kick)
- Data pemain tersimpan di MySQL: uang, skin, interior, health, armor, posisi
- Auto-save tiap 5 menit, saat disconnect, dan saat server mati
- Struktur modular di `gamemodes/modules`

## Kebutuhan
- Server open.mp 32-bit (Linux)
- Database MySQL/MariaDB
- Plugin MySQL pBlueG R41-4 (build Linux 32-bit): `mysql.so` dan `log-core.so`

## Setup
1. Salin `mysql.ini.example` menjadi `mysql.ini` di root server, lalu isi kredensial database.
2. Taruh `mysql.so` di `plugins/`, dan `log-core.so` di root server dan `plugins/`.
3. Di `config.json` set `"legacy_plugins": ["mysql"]` dan `"main_scripts": ["hopecity 1"]`.
4. Compile dengan `bash tools/build.sh` (hasil: `gamemodes/hopecity.amx`).
5. Jalankan server. Tabel `hc_accounts` dibuat otomatis.

## Struktur
- `gamemodes/` : source gamemode
- `gamemodes/modules/` : auth, database, player, commands, core, utils
- `deploy/` : catatan deploy
- `tools/` : compiler dan script build
- `server.cfg.example` : template config

## Tidak ikut di-commit
`mysql.ini`, `server.cfg`, `*.amx`, `*.so`, `scriptfiles/` (lihat `.gitignore`).

Riwayat perubahan ada di [CHANGELOG.md](CHANGELOG.md).
