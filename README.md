# Beat It

Rhythm game 2D dibuat dengan **C++** dan **SFML**. Lawan musuh dengan pukulan dan tendangan tepat waktu mengikuti beat lagu

![Status](https://img.shields.io/badge/status-in%20development-yellow)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![SFML](https://img.shields.io/badge/SFML-2.5-green)

## Gameplay

- **A** → Pukul musuh tipe **A** (lingkaran neon)
- **D** → Tendang musuh tipe **B** (kotak neon)
- **Hold note**: tahan tombol selama ekor neon masih memanjang
- **Esc** → Kembali ke menu / batal
- Timing menentukan judgement: **Perfect → Good → Bad → Miss**, dengan sistem combo dan scoring

## Fitur

- Beatmap kustom untuk lagu **"Monitoring"** (Hatsune Miku, 132 BPM) dengan 3 tingkat kesulitan: **Easy / Medium / Hard**
- Sistem hit-judgement berbasis waktu (Perfect/Good/Bad/Miss) dengan threshold yang bisa diatur
- Hold note — tekan dan tahan untuk skor maksimal
- Sistem combo & floating judgement text
- Menu utama dengan animasi neon, partikel, dan title bergaya glow
- Settings lengkap: Volume, Enemy Speed, Enemy Colors (custom RGB), Difficulty
- Result screen menampilkan score, max combo, akurasi, dan rincian Perfect/Good/Bad/Miss
-  Background infinite-scrolling parallax

## Struktur Proyek

```
BeatIT/
├── assets/
│   ├── audio/          # musik & sound effects (.ogg)
│   ├── background/     # gambar background stage & menu
│   └── fonts/           # font UI & neon title
├── include/             # header (.hpp)
├── src/                 # implementasi (.cpp)
├── CMakeLists.txt
└── README.md
```

### Modul utama

| File | Tanggung jawab |
|---|---|
| `menu.*` | Menu utama, popup settings (volume/speed/colors/difficulty) |
| `stage.*` | Loop gameplay utama: background, musik, integrasi semua sistem |
| `player.*` | Karakter pemain, animasi lari/pukul/tendang |
| `enemy.*` | Musuh (tap & hold note), beatmap per difficulty, sistem judgement |
| `hud.*` | Score, combo, floating text, statistik per judgement |
| `result_screen.*` | Tampilan ringkasan setelah stage selesai |

## Build & Run

### Dependencies

- CMake ≥ 3.10
- Compiler dengan dukungan C++17
- [SFML 2.5](https://www.sfml-dev.org/) (graphics, window, system, audio)

### Linux (Ubuntu/Debian)

```bash
sudo apt install libsfml-dev
```

### Compile

```bash
mkdir -p build
cd build
cmake ..
make
```

### Jalankan

```bash
./BeatIT
```

> Path asset di-resolve otomatis lewat macro `ASSET_DIR` yang di-generate CMake dari lokasi project — tidak perlu menjalankan dari folder tertentu.

## Kontrol Settings

| Setting | Deskripsi |
|---|---|
| Volume | Volume musik menu & stage |
| Enemy Speed | Kecepatan musuh bergerak (px/detik) |
| Enemy Colors | Warna custom untuk musuh tipe A & B (slider RGB) |
| Difficulty | Easy / Medium / Hard — menentukan kepadatan beatmap |

## Roadmap

- [ ] Tambah lagu & beatmap baru
- [ ] Leaderboard lokal
- [ ] Efek hit-feedback tambahan (screen shake, particle burst)
- [ ] Dukungan custom beatmap dari file eksternal

## Lisensi

Proyek pribadi untuk pembelajaran. Aset musik & gambar tunduk pada lisensi masing-masing sumber.
