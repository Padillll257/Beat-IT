#ifndef ENEMY_HPP
#define ENEMY_HPP
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

//DIFFICULTY
enum class Difficulty { Easy, Medium, Hard };
//Judgement 
enum class Judgement { Perfect, Good, Bad, Miss, None };
struct HitResult {
    Judgement   judgement = Judgement::None;
    bool        correctType = false;   // true kalau jenis serangan memang cocok
    sf::Vector2f position;             // posisi musuh saat kena (utk floating text)
    bool        hasPosition = false;   // false kalau Judgement::None (tidak ada posisi)
    bool        isHoldNote  = false;   // true kalau ini hasil dari hold note (bukan tap)
};

class Enemy {
public:
    enum class Type { A, B };   // A = lingkaran neon (pukul), B = kotak neon (tendang)
    // State internal untuk hold note. Tap note langsung dianggap Idle selamanya sampai kena/passed (tidak pernah masuk Holding/HoldDone)
    enum class HoldState { Idle, Holding, HoldSuccess, HoldFailed };
    Enemy(sf::RenderWindow& window, Type type, float spawnX, float groundY,
          float targetHitTime, float holdDuration = 0.f);
    void update(float dt);
    void draw();
    sf::FloatRect getBounds()  const;
    Type          getType()    const { return m_type; }
    bool          isDead()     const { return m_dead; }
    bool          isPassed()   const { return m_passed; }
    bool          isHoldNote() const { return m_holdDuration > 0.f; }
    // Waktu (detik sejak stage mulai) musuh ini SEHARUSNYA berada tepat di tengah hit zone — dipakai untuk menghitung selisih timing saat dipukul
    float getTargetHitTime()  const { return m_targetHitTime; }
    float getHoldDuration()   const { return m_holdDuration; }
    HoldState getHoldState()  const { return m_holdState; }

    void kill();

    //Hold Note Control (dipanggil dari EnemyManager)
    //Mulai menahan note ini (dipanggil saat tombol pertama ditekan & kena)
    void startHold(float stageTimerNow);
    //Dipanggil tiap frame selama tombol masih ditahan. Mengembalikan true
    //Kalau hold sudah selesai (holdDuration tercapai) sambil tombol masih ditahan.
    bool updateHold(float dt, float stageTimerNow);
    // Tombol dilepas sebelum waktunya. Mengembalikan rasio progress (0..1)
    // seberapa jauh hold sempat berjalan sebelum dilepas.
    float releaseHold();

    //SETTERS (dipanggil saat spawn, sesuai settings dari menu) 
    void setSpeed(float speed) { m_speed = speed; }
    void setColor(sf::Color color);

private:
    sf::RenderWindow& m_window;
    Type              m_type;
    //Shape neon (lingkaran untuk A, kotak untuk B)
    sf::CircleShape    m_circle;   //dipakai jika Type::A
    sf::RectangleShape m_box;      //dipakai jika Type::B

    //Ekor hold note (garis memanjang ke arah spawn / kanan)
    sf::RectangleShape m_tail;
    sf::Color          m_tailBaseColor = sf::Color(255, 255, 255, 90);

    float m_size = 56.f;   // diameter lingkaran / sisi kotak

    //parameter spawn & hit zone
    float m_pulseTimer = 0.f;
    float m_speed = 220.f;   // pixel per detik, musuh bergerak ke kiri
    bool  m_dead   = false;
    bool  m_passed = false;
    float m_groundY = 0.f;
    float m_targetHitTime = 0.f;

    // Hold note state
    float     m_holdDuration  = 0.f;     // 0 = bukan hold note
    HoldState m_holdState     = HoldState::Idle;
    float     m_holdElapsed   = 0.f;     // detik sejak startHold() dipanggil
    float     m_holdStartTime = 0.f;     // m_stageTimer saat startHold() dipanggil

    void setupShape();
    void updateGlow(float dt);
    void updateTail();   // posisikan & resize ekor sesuai sisa progress hold
};
//BEAT NOTE — data satu musuh di beatmap
struct BeatNote {
    float        spawnTime;            // detik sejak stage mulai → musuh di-spawn
    Enemy::Type  type;                 // Enemy::Type::A (lingkaran) atau Enemy::Type::B (kotak)
    float        holdDuration = 0.f;   // 0 = tap note biasa. >0 = hold note, tahan sekian detik.
};


//  BEATMAPS — 3 versi kepadatan untuk lagu "Monitoring" (Hatsune Miku)
//  132 BPM, durasi 3:01. 1 beat = 60/132 = 0.4545s
//  Editable sesuai preferensi, tetapi saya buat versi sederhananya aja, karena tuning pastinya makan waktu lama. Kalau mau versi lebih kompleks, bisa bikin beatmap sendiri di luar program ini.
namespace Beatmaps {
    std::vector<BeatNote> getEasy();
    std::vector<BeatNote> getMedium();
    std::vector<BeatNote> getHard();

    // Helper: ambil beatmap sesuai Difficulty yang dipilih
    std::vector<BeatNote> getByDifficulty(Difficulty diff);
}

//ENEMY MANAGER — mengatur semua musuh di stage, termasuk spawn, update, draw, dan hit detection
class EnemyManager {
public:
    EnemyManager(sf::RenderWindow& window, float groundY);

    void update(float dt);
    void draw();
    void drawTargets();   // gambar target diam (lingkaran/kotak buram) di hit zone
    bool isFinished() const {
        return m_nextNote >= int(m_beatmap.size()) && m_enemies.empty();
    }
    HitResult onPunch();   // tombol A ditekan
    HitResult onKick();    // tombol D ditekan
    HitResult onPunchRelease();
    HitResult onKickRelease();

    // Dipanggil tiap frame (dari Stage::update) untuk note hold yang SUKSES
    void updateHolds(float dt, bool isPunchHeld, bool isKickHeld,
                      HitResult& outPunchResult, HitResult& outKickResult);
    void reset();

    void setSpeed(float speed)                       { m_defaultSpeed = speed; }
    void setColors(sf::Color colorA, sf::Color colorB) {
        m_defaultColorA = colorA;
        m_defaultColorB = colorB;
    }

    // Pilih beatmap sesuai difficulty.
    void setDifficulty(Difficulty diff);

    // ── THRESHOLD TIMING (detik) — dipakai untuk tap note & komponen "start" hold note
    static constexpr float kPerfectWindow = 0.08f;
    static constexpr float kGoodWindow    = 0.18f;
    static constexpr float kBadWindow     = 0.35f;

private:
    sf::RenderWindow&  m_window;
    float              m_groundY;

    std::vector<std::unique_ptr<Enemy>> m_enemies;
    float              m_stageTimer = 0.f;
    int                m_nextNote   = 0;

    // Beatmap aktif (dipilih via setDifficulty)
    std::vector<BeatNote> m_beatmap;

    // Default yang diterapkan ke musuh baru saat spawn (dari Menu Settings)
    float     m_defaultSpeed  = 220.f;
    sf::Color m_defaultColorA = sf::Color(0, 220, 255);    // cyan default
    sf::Color m_defaultColorB = sf::Color(255, 0, 200);    // magenta default

    float spawnX() const {
        return float(m_window.getSize().x) + 80.f;
    }

    // Hit zone di sekitar posisi player
    float m_hitZoneLeft   = 0.f;
    float m_hitZoneRight  = 0.f;
    float m_hitZoneCenter = 0.f;

    // Target diam (penanda visual di hit zone) — digambar terpisah dari musuh
    sf::CircleShape    m_targetCircle;   // penanda untuk Type::A (pukul)
    sf::RectangleShape m_targetBox;      // penanda untuk Type::B (tendang)

    // Hold note yang sedang aktif ditahan player, per tipe (nullptr = tidak ada)
    Enemy* m_activeHoldA = nullptr;   
    Enemy* m_activeHoldB = nullptr;   

    // Helper bersama untuk onPunch()/onKick()
    HitResult resolveHit(Enemy::Type expectedType, Enemy** activeHoldSlot);
    HitResult resolveRelease(Enemy::Type expectedType, Enemy** activeHoldSlot);
    Judgement judgeTiming(float deltaSeconds) const; 

    // Gabungkan judgement "start" dengan komponen "hold"
    Judgement combineJudgement(Judgement startJ, float holdRatio) const;
};

#endif