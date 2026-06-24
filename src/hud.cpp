#include "hud.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

Hud::Hud(sf::RenderWindow& window)
: m_window(window)
{
    if (!m_font.loadFromFile( std::string(ASSET_DIR) + "fonts/title.ttf")) {
        std::cerr << "Failed to load font assets/fonts/title.ttf (Hud)\n";
    }

    setupCounters();
}
void Hud::setupCounters()
{
    m_scoreText.setFont(m_font);
    m_scoreText.setCharacterSize(28);
    m_scoreText.setFillColor(sf::Color::White);
    m_scoreText.setStyle(sf::Text::Bold);
    m_scoreText.setPosition(24.f, 20.f);

    m_comboText.setFont(m_font);
    m_comboText.setCharacterSize(22);
    m_comboText.setFillColor(sf::Color(255, 220, 80));
    m_comboText.setStyle(sf::Text::Bold);
    m_comboText.setPosition(24.f, 56.f);

    updateCounters();
}

void Hud::updateCounters()
{
    m_scoreText.setString("Score: " + std::to_string(m_score));

    if (m_combo > 0) {
        m_comboText.setString(std::to_string(m_combo) + " Combo");
    } else {
        m_comboText.setString("");
    }
}
void Hud::registerHit(const HitResult& result)
{
    // Judgement::None → tombol ditekan sia-sia (tidak ada musuh di zona)
    if (result.judgement == Judgement::None) return;

    Judgement j = result.judgement;

    //Update Statistik Judgement
    switch (j) {
        case Judgement::Perfect: m_perfectCount++; break;
        case Judgement::Good:    m_goodCount++;    break;
        case Judgement::Bad:     m_badCount++;     break;
        case Judgement::Miss:    m_missCount++;    break;
        default: break;
    }

    //Combo logic 
    if (j == Judgement::Miss || j == Judgement::Bad) {
        m_combo = 0;
    } else {
        m_combo++;
        m_maxCombo = std::max(m_maxCombo, m_combo);
        m_comboPulseTimer = 0.15f;   // trigger efek "pop" sebentar
    }
    m_score += judgementToPoints(j);
    updateCounters();
    if (result.hasPosition) {
        spawnFloatingText(judgementToString(j), result.position, judgementToColor(j));
    }
}
void Hud::reset()
{
    m_score = 0;
    m_combo = 0;
    m_maxCombo = 0;
    
    m_perfectCount = 0;
    m_goodCount = 0;
    m_badCount = 0;
    m_missCount = 0;

    m_comboPulseTimer = 0.f;
    m_comboPulseScale = 1.f;
    
    m_floatingTexts.clear();
    updateCounters();
}

int Hud::judgementToPoints(Judgement j) const
{
    switch (j) {
        case Judgement::Perfect: return 100;
        case Judgement::Good:    return 60;
        case Judgement::Bad:     return 20;
        case Judgement::Miss:    return 0;
        default:                 return 0;
    }
}

std::string Hud::judgementToString(Judgement j) const
{
    switch (j) {
        case Judgement::Perfect: return "PERFECT";
        case Judgement::Good:    return "GOOD";
        case Judgement::Bad:     return "BAD";
        case Judgement::Miss:    return "MISS";
        default:                 return "";
    }
}

sf::Color Hud::judgementToColor(Judgement j) const
{
    switch (j) {
        case Judgement::Perfect: return sf::Color(255, 220, 60);   // kuning emas
        case Judgement::Good:    return sf::Color(80, 220, 255);   // cyan
        case Judgement::Bad:     return sf::Color(255, 140, 60);   // oranye
        case Judgement::Miss:    return sf::Color(255, 60, 60);    // merah
        default:                 return sf::Color::White;
    }
}
//  FLOATING TEXT (PERFECT/GOOD/BAD/MISS) — melayang naik lalu hilang
void Hud::spawnFloatingText(const std::string& str, sf::Vector2f pos, sf::Color color)
{
    FloatingText ft;
    ft.text.setFont(m_font);
    ft.text.setString(str);
    ft.text.setCharacterSize(26);
    ft.text.setStyle(sf::Text::Bold);
    ft.text.setFillColor(color);

    sf::FloatRect tb = ft.text.getLocalBounds();
    ft.text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);

    ft.pos      = pos;
    ft.life     = ft.maxLife;
    ft.riseSpeed = 60.f;

    ft.text.setPosition(ft.pos);

    m_floatingTexts.push_back(std::move(ft));
}
void Hud::updateFloatingTexts(float dt)
{
    for (auto& ft : m_floatingTexts) {
        ft.life -= dt;
        ft.pos.y -= ft.riseSpeed * dt;   // melayang naik

        float lifeRatio = std::clamp(ft.life / ft.maxLife, 0.f, 1.f);

        // Fade out di 40% sisa umur terakhir
        sf::Color c = ft.text.getFillColor();
        c.a = sf::Uint8(255.f * std::min(1.f, lifeRatio / 0.4f));
        ft.text.setFillColor(c);

        // Sedikit scale-up di awal munculnya (efek "pop")
        float age   = ft.maxLife - ft.life;
        float scale = 1.f + std::max(0.f, (0.12f - age) / 0.12f) * 0.4f;
        ft.text.setScale(scale, scale);

        ft.text.setPosition(ft.pos);
    }

    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
            [](const FloatingText& ft) { return ft.life <= 0.f; }),
        m_floatingTexts.end()
    );
}

void Hud::drawFloatingTexts()
{
    for (auto& ft : m_floatingTexts) {
        m_window.draw(ft.text);
    }
}
void Hud::update(sf::Time dt)
{
    float secs = dt.asSeconds();
    updateFloatingTexts(secs);
    // Efek pulse kecil untuk combo text saat combo baru naik
    if (m_comboPulseTimer > 0.f) {
        m_comboPulseTimer -= secs;
        float t = std::clamp(m_comboPulseTimer / 0.15f, 0.f, 1.f);
        m_comboPulseScale = 1.f + t * 0.3f;
    } else {
        m_comboPulseScale = 1.f;
    }
    m_comboText.setScale(m_comboPulseScale, m_comboPulseScale);
}
void Hud::draw()
{
    m_window.draw(m_scoreText);
    if (m_combo > 0) {
        m_window.draw(m_comboText);
    }
    drawFloatingTexts();
}