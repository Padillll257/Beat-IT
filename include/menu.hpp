#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <functional>
#include "enemy.hpp"   // untuk enum Difficulty

class Menu {
public:
    Menu(sf::RenderWindow& window);
    ~Menu();

    void handleEvent(const sf::Event& event);
    void update(sf::Time dt);
    void draw();

    std::function<void()> onPlay;
    std::function<void()> onQuit;

    //MUSIC CONTROL (dipanggil dari main.cpp saat ganti state)
    void stopMusic();
    void resumeMusic();

    //SETTINGS GETTERS (dipanggil dari main.cpp untuk pass ke Stage)
    float      getEnemySpeed()  const { return m_enemySpeed; }
    sf::Color  getEnemyColorA() const { return m_enemyColorA; }
    sf::Color  getEnemyColorB() const { return m_enemyColorB; }
    Difficulty getDifficulty()  const { return m_difficulty; }

private:
    sf::RenderWindow& m_window;
    // resources
    sf::Texture m_bgTex;
    sf::Sprite  m_bgSprite;
    sf::Font    m_font;
    sf::Font    m_fontNeon;

    // Neon glow layers (dari luar ke dalam)
    sf::Text m_titleGlow2;
    sf::Text m_titleGlow1;
    sf::Text m_titleGlow0;
    sf::Text    m_titleText;
    sf::Text    m_titleShadow;
    float       m_titleBobTimer  = 0.f;
    float       m_titleAlpha     = 0.f;
    bool        m_titleAnimDone  = false;
    // BUTTON (dipakai untuk menu utama & semua popup) 
    struct Button {
        sf::RectangleShape box;
        sf::Text           label;
        sf::Vector2f       basePos;
        bool               hovered   = false;

        float slideOffset  = 120.f;
        float alpha        = 0.f;
        float animDelay    = 0.f;
        bool  animDone     = false;

        float scaleTarget  = 1.f;
        float scaleCurrent = 1.f;
    };

    // Menu utama: hanya 3 tombol sekarang
    Button m_btnPlay;
    Button m_btnSettings;
    Button m_btnQuit;

    float m_introTimer = 0.f;

    // ── SLIDER (reusable: Volume, Enemy Speed, dan 6 channel RGB) ─────
    struct Slider {
        sf::Text           label;
        sf::RectangleShape barBg;
        sf::RectangleShape barFill;
        float               value    = 0.5f;   // 0..1 normalized
        float               minValue = 0.f;
        float               maxValue = 1.f;
        sf::Color           fillColor = sf::Color(120, 220, 180);
    };
    void setupSlider(Slider& s, const std::string& labelText,
                      sf::Vector2f pos, float initialValue,
                      float minV, float maxV, sf::Color fillColor);
    void updateSliderVisual(Slider& s); // sinkronkan barFill dengan s.value
    void handleSliderDrag(Slider& s, const sf::Vector2f& mp); // set value dari posisi mouse

    // Slider mana yang sedang di-drag (nullptr = tidak ada)
    Slider* m_draggingSlider = nullptr;

    //State setting
    enum class SettingsPage { None, Hub, Volume, Speed, Color, Difficulty };
    SettingsPage m_settingsPage = SettingsPage::None;

    // Popup container generik (background + title + tombol back)
    sf::RectangleShape m_popupBg;
    sf::Text           m_popupTitle;
    Button             m_btnBack;       // tombol "< Back" di tiap sub-popup

    // Hub: 4 tombol kategori
    Button m_btnOpenVolume;
    Button m_btnOpenSpeed;
    Button m_btnOpenColor;
    Button m_btnOpenDifficulty;

    // Page: Volume
    Slider m_sliderVolume;

    // Page: Enemy SpeedFLOATING JUDGEMENT TEXT
    Slider m_sliderSpeed;
    float  m_enemySpeed = 220.f;       // nilai aktual (pixel/detik), dikirim ke Stage
    static constexpr float kSpeedMin = 80.f;
    static constexpr float kSpeedMax = 500.f;

    // Page: Enemy Color (6 slider: A.r A.g A.b, B.r B.g B.b)
    Slider m_sliderA_R, m_sliderA_G, m_sliderA_B;
    Slider m_sliderB_R, m_sliderB_G, m_sliderB_B;
    sf::Color m_enemyColorA = sf::Color(0, 220, 255);   // default cyan
    sf::Color m_enemyColorB = sf::Color(255, 0, 200);   // default magenta

    // Preview shape kecil di page Color (lingkaran utk A, kotak utk B)
    sf::CircleShape    m_previewA;
    sf::RectangleShape m_previewB;

    // Page: Difficulty (3 tombol pilihan: Easy / Medium / Hard)
    Button     m_btnDiffEasy;
    Button     m_btnDiffMedium;
    Button     m_btnDiffHard;
    Difficulty m_difficulty = Difficulty::Easy;

    void setupSettingsPopup();
    void openSettingsPage(SettingsPage page);
    void drawSettingsHub();
    void drawSettingsVolume();
    void drawSettingsSpeed();
    void drawSettingsColor();
    void drawSettingsDifficulty();
    struct Particle {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float life;
        float radius;
        sf::Color color;
    };
    std::vector<Particle> m_particles;
    void spawnParticle();
    void updateParticles(sf::Time dt);
    void drawParticles();

    //Music
    std::vector<std::string> m_playlist;
    sf::Music m_music;
    void loadPlaylist();
    void playRandomMenuTrack();
    // click.ogg        → klik tombol apapun
    // hover.ogg        → mouse pertama kali masuk ke atas tombol
    // slider_tick.ogg  → drag slider (dimainkan berkala, bukan tiap frame)
    sf::SoundBuffer m_sfxClickBuffer;
    sf::SoundBuffer m_sfxHoverBuffer;
    sf::SoundBuffer m_sfxSliderBuffer;
    sf::Sound       m_sfxClick;
    sf::Sound       m_sfxHover;
    sf::Sound       m_sfxSlider;
    void loadSfx();
    void playClickSfx();
    void playHoverSfx();
    void playSliderSfx();
    // Throttle supaya slider_tick tidak diputar tiap frame saat drag
    float m_sliderSfxCooldown = 0.f;
    static constexpr float kSliderSfxInterval = 0.06f; // detik antar tick
    void setupUI();
    void updateButtonHover(const sf::Vector2f& mousePos);
    bool pointInRect(const sf::Vector2f& p, const sf::FloatRect& r) const;

    void updateTitleAnim(float dt);
    void updateButtonAnims(float dt);
    void applyButtonTransform(Button& btn);

    void initButton(Button& b, const std::string& text, sf::Vector2f pos,
                     sf::Vector2f size, float delay);
};

#endif