#ifndef HUD_HPP
#define HUD_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include "enemy.hpp"

//HUD : floating judgement text, combo counter, score counter
class Hud {
public:
    Hud(sf::RenderWindow& window);
    void update(sf::Time dt);
    void draw();
    // Dipanggil dari Stage setiap kali ada hasil hit (dari onPunch/onKick)
    void registerHit(const HitResult& result);
    // Reset semua status (skor, combo, counter) untuk fitur Replay
    void reset();

    // GETTERS UNTUK RESULT SCREEN
    int getScore() const    { return m_score; }
    int getCombo() const    { return m_combo; }
    int getMaxCombo() const { return m_maxCombo; }
    int getPerfect() const  { return m_perfectCount; }
    int getGood() const     { return m_goodCount; }
    int getBad() const      { return m_badCount; }
    int getMiss() const     { return m_missCount; }

private:
    sf::RenderWindow& m_window;
    sf::Font          m_font;

    //SCORE & COMBO
    int m_score    = 0;
    int m_combo    = 0;
    int m_maxCombo = 0;

    //COUNTERS FOR STATS
    int m_perfectCount = 0;
    int m_goodCount    = 0;
    int m_badCount     = 0;
    int m_missCount    = 0;

    sf::Text m_scoreText;
    sf::Text m_comboText;
    float    m_comboPulseTimer = 0.f;   // buat efek "pop" saat combo naik
    float    m_comboPulseScale = 1.f;

    void setupCounters();
    void updateCounters();

    //Floating Texts (Judgement)
    struct FloatingText {
        sf::Text text;
        sf::Vector2f pos;
        float   life     = 0.f;   // detik tersisa
        float   maxLife  = 0.7f;
        float   riseSpeed = 60.f; // pixel per detik, melayang naik
    };
    std::vector<FloatingText> m_floatingTexts;

    void spawnFloatingText(const std::string& str, sf::Vector2f pos, sf::Color color);
    void updateFloatingTexts(float dt);
    void drawFloatingTexts();

    // Helper: judgement → string + warna
    std::string judgementToString(Judgement j) const;
    sf::Color   judgementToColor(Judgement j) const;

    // Poin per judgement (dipakai untuk scoring)
    int judgementToPoints(Judgement j) const;
};

#endif