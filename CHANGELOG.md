# Changelog

Semua perubahan penting pada HopeCity Roleplay dicatat di sini.

## [0.4.0] - 2026-10-22

### Diubah
- Database dipindah dari SQLite ke MySQL memakai plugin pBlueG R41-4 (`mysql_connect_file`, kredensial di `mysql.ini`).
- Tabel akun sekarang `hc_accounts` (dibuat otomatis saat server start).
- Semua query memakai `mysql_format` dengan escape `%e`.
- Login/register memakai query langsung, sedangkan penyimpanan data pemain memakai query threaded.

### Ditambahkan
- `mysql.ini.example` sebagai template konfigurasi database.
- Konfigurasi `legacy_plugins` untuk plugin `mysql` di `config.json`.

### Keamanan
- `mysql.ini` dan `*.so` masuk `.gitignore` supaya kredensial dan binary tidak ikut ke repo.

## [0.3.0] - 2026-10-22

### Ditambahkan
- Simpan dan muat data pemain: uang, skin, interior, health, armor, posisi, dan arah hadap.
- Auto-save tiap 5 menit, saat pemain disconnect, dan saat gamemode berhenti.
- Pemain baru mendapat uang awal 500 dan spawn di titik default.
- Migrasi kolom otomatis untuk tabel akun yang sudah ada.

## [0.2.0] - 2026-10-22

### Diubah
- Hash password diganti dari Argon2 (plugin `async_crypto`) ke `SHA256_PassHash` bawaan open.mp dengan salt acak 16 karakter per akun, sehingga tidak butuh plugin dan cocok dengan server 32-bit x86.
- Password register minimal 6 karakter.

### Diperbaiki
- Cek akun duplikat saat register memakai nama yang salah (nama kosong), sekarang memakai nama pemain.
- `Auth_OnDisconnect` tidak pernah dipanggil dari `OnPlayerDisconnect`.
- Teks dialog selamat datang menampilkan `\n` literal, sekarang baris baru.
- Pemain bisa spawn dan memakai command sebelum login: ditambahkan guard di `OnPlayerRequestSpawn` dan `OnPlayerCommandText`.
- Pemain sekarang di-spawn otomatis setelah login berhasil.

### Ditambahkan
- Batas 3 kali percobaan login salah, lalu pemain di-kick.

### Dihapus
- Plugin `async_crypto` dan include-nya dari repo.

## [0.1.0] - Awal

### Ditambahkan
- Kerangka gamemode modular: `core`, `player`, `commands`, `utils`, `database`, `auth`.
- Sistem register dan login berbasis dialog.
- Command dasar `/help` dan `/credits`.
- Script build `tools/build.sh` untuk Termux dan catatan deploy Pterodactyl.

### Keamanan
- `server.cfg` dikeluarkan dari tracking dan diganti `server.cfg.example`.
- `.gitignore` untuk file hasil build (`*.amx`, `*.bak`, `scriptfiles/`, `*.db`).

## 2026-10-22
- Login/register dan load data pemain diubah ke async (`mysql_tquery`), server tidak nge-block saat query.
- Tambah `Player_ID`, save/update sekarang pakai `WHERE id`.
- Tambah kolom `last_login` dan `last_ip` (migrasi otomatis untuk tabel lama).
- Dialog auth diberi delay 0.8 detik agar pasti muncul; ada guard double-klik dan session check saat disconnect.
- Save saat server mati dibuat sync supaya data tidak hilang sebelum koneksi ditutup.

## 2026-10-22 (lanjutan)
- Fix loop reconnect setelah salah password 3x: akun dikunci sementara 120 detik per IP + nama, kick ditunda 1,5 detik supaya pesan terbaca.
- Hapus warning compile (239) pada pemanggilan SetTimer/SetTimerEx.

## 2026-10-22 (admin)
- Sistem admin: command/admin.inc (duty, HUD, kick, freeze, set HP/armor/skin, give money, announce, setadmin).
- command/admin_extra.inc: HUD panel /acmd, TP wajib duty (/gotoco /gotols /gotolv /gotosf /sendto), /spec, /mute, /warn, /slap, /spawn, /clearchat, /veh, /fixveh, /ban, /unban, /setweather, /settime, /players, /acheck.
- Tabel baru hc_bans, kolom admin di hc_accounts.

## 2026-10-22 (ahelp panel)
- /acmd dihapus; /ahelp sekarang menampilkan panel HUD berhalaman (/ahelp [hal], /ahelp tutup), otomatis tertutup setelah 20 detik.

## 2026-10-22 (ahelp center)
- /ahelp tunggal: panel HUD di tengah layar, tombol X untuk menutup, tombol << >> untuk ganti halaman, tanpa auto close.
- Hapus /ahelp versi chat dari admin.inc.
