#include "menu.hpp"
#include <random>
#include <cmath>
#include <iostream>
#include <algorithm>

Menu::Menu(sf::RenderWindow& window)
: m_window(window)
{
    if (!m_bgTex.loadFromFile(std::string(ASSET_DIR) + "background/menu.jpg")) {
        std::cerr << "Failed to load assets/background/menu.jpg\n";
    } else {
        m_bgSprite.setTexture(m_bgTex);
        sf::Vector2u texSize = m_bgTex.getSize();
        sf::Vector2u winSize(m_window.getSize());
        float sx = float(winSize.x) / texSize.x;
        float sy = float(winSize.y) / texSize.y;
        m_bgSprite.setScale(sx, sy);
    }

    if (!m_font.loadFromFile(std::string(ASSET_DIR) + "fonts/title.ttf")) {
        std::cerr << "Failed to load font assets/fonts/title.ttf\n";
    }

    setupUI();
    setupSettingsPopup();
    loadPlaylist();
    playRandomMenuTrack();
    loadSfx();

    for (int i = 0; i < 60; i++) spawnParticle();
}

Menu::~Menu()
{
    m_music.stop();
}
void Menu::setupUI()
{
    sf::Vector2u win = m_window.getSize();
    float cx = float(win.x) / 2.f;
    float titleY = float(win.y) * 0.22f;

    if (!m_fontNeon.loadFromFile(std::string(ASSET_DIR) + "fonts/neon.ttf")) {
        m_fontNeon = m_font;
    }

    auto makeTitle = [&](sf::Text& t, const std::string& str,
                        unsigned int size, sf::Color col, float offX = 0.f, float offY = 0.f)
    {
        t.setFont(m_fontNeon);
        t.setString(str);
        t.setCharacterSize(size);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(sf::Color(col.r, col.g, col.b, 0));
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        t.setPosition(cx + offX, titleY + offY);
    };

    makeTitle(m_titleGlow2,  "Beat It", 104, sf::Color(120,  20, 200));
    makeTitle(m_titleGlow1,  "Beat It",  98, sf::Color( 80,  80, 255));
    makeTitle(m_titleGlow0,  "Beat It",  96, sf::Color(  0, 180, 255));
    makeTitle(m_titleText,   "Beat It",  96, sf::Color(200, 235, 255));
    makeTitle(m_titleShadow, "Beat It",  96, sf::Color( 80,   0, 160));
    m_titleShadow.setPosition(cx + 4.f, titleY + 6.f);

    float startY = float(win.y) * 0.55f;
    initButton(m_btnPlay,     "Play",     {cx, startY},          {350.f, 64.f}, 0.5f);
    initButton(m_btnSettings, "Settings", {cx, startY + 90.f},   {350.f, 64.f}, 0.7f);
    initButton(m_btnQuit,     "Quit",     {cx, startY + 180.f},  {350.f, 64.f}, 0.9f);
}

// Helper umum untuk inisialisasi 1 button + animasinya
void Menu::initButton(Button& b, const std::string& text, sf::Vector2f pos,
                       sf::Vector2f size, float delay)
{
    b.box.setSize(size);
    b.box.setFillColor(sf::Color(0, 0, 0, 120));
    b.box.setOutlineThickness(2.f);
    b.box.setOutlineColor(sf::Color(255, 255, 255, 60));
    b.box.setOrigin(size / 2.f);
    b.box.setPosition(pos);
    b.basePos = pos;

    b.label.setFont(m_font);
    b.label.setString(text);
    b.label.setCharacterSize(24);
    b.label.setFillColor(sf::Color(255, 255, 255, 0));
    sf::FloatRect lb = b.label.getLocalBounds();
    b.label.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
    b.label.setPosition(pos);

    b.slideOffset  = 130.f;
    b.alpha        = 0.f;
    b.animDelay    = delay;
    b.animDone     = false;
    b.scaleCurrent = 1.f;
    b.scaleTarget  = 1.f;
    b.hovered      = false;
}
void Menu::setupSettingsPopup()
{
    sf::Vector2u win = m_window.getSize();
    float cx = float(win.x) / 2.f;
    float cy = float(win.y) / 2.f;

    // Popup container generik (dipakai ulang untuk hub & semua page)
    // Tinggi 460 supaya cukup untuk page Color (6 slider + preview + judul)
    m_popupBg.setSize({480.f, 460.f});
    m_popupBg.setFillColor(sf::Color(10, 10, 10, 230));
    m_popupBg.setOutlineThickness(2.f);
    m_popupBg.setOutlineColor(sf::Color(255, 255, 255, 80));
    m_popupBg.setOrigin(m_popupBg.getSize() / 2.f);
    m_popupBg.setPosition(cx, cy);

    m_popupTitle.setFont(m_font);
    m_popupTitle.setCharacterSize(26);
    m_popupTitle.setFillColor(sf::Color::White);
    m_popupTitle.setStyle(sf::Text::Bold);

    // Tombol back, pojok kiri-atas popup (relatif terhadap popup, bukan layar)
    sf::Vector2f popupTopLeft = m_popupBg.getPosition() - m_popupBg.getSize() / 2.f;
    initButton(m_btnBack, "< Back",
        {popupTopLeft.x + 65.f, popupTopLeft.y + 35.f},
        {110.f, 40.f}, 0.f);
    m_btnBack.alpha = 255.f;
    m_btnBack.slideOffset = 0.f;
    m_btnBack.animDone = true;
    m_btnBack.box.setFillColor(sf::Color(0, 0, 0, 150));
    m_btnBack.label.setFillColor(sf::Color::White);

    float hubStartY = cy - 110.f;
    initButton(m_btnOpenVolume,     "Volume",       {cx, hubStartY},           {320.f, 60.f}, 0.f);
    initButton(m_btnOpenSpeed,      "Enemy Speed",  {cx, hubStartY + 80.f},    {320.f, 60.f}, 0.f);
    initButton(m_btnOpenColor,      "Enemy Colors", {cx, hubStartY + 160.f},   {320.f, 60.f}, 0.f);
    initButton(m_btnOpenDifficulty, "Difficulty",   {cx, hubStartY + 240.f},   {320.f, 60.f}, 0.f);
    for (Button* b : { &m_btnOpenVolume, &m_btnOpenSpeed, &m_btnOpenColor, &m_btnOpenDifficulty }) {
        b->alpha = 255.f;
        b->slideOffset = 0.f;
        b->animDone = true;
        b->label.setFillColor(sf::Color::White);
        b->box.setFillColor(sf::Color(0, 0, 0, 150));
    }
    setupSlider(m_sliderVolume, "Volume", {cx, cy}, 0.6f, 0.f, 1.f,
                sf::Color(120, 220, 180));

    float speedNorm = (m_enemySpeed - kSpeedMin) / (kSpeedMax - kSpeedMin);
    setupSlider(m_sliderSpeed, "Enemy Speed", {cx, cy}, speedNorm, kSpeedMin, kSpeedMax,
                sf::Color(255, 200, 80));

    float previewY  = cy - 145.f;
    float colStartY = cy - 90.f;
    float rowGap    = 32.f;

    m_previewA.setRadius(18.f);
    m_previewA.setOrigin(18.f, 18.f);
    m_previewA.setPosition(cx - 90.f, previewY);
    m_previewA.setFillColor(m_enemyColorA);
    m_previewA.setOutlineThickness(2.f);
    m_previewA.setOutlineColor(sf::Color::White);
    m_previewB.setSize({36.f, 36.f});
    m_previewB.setOrigin(18.f, 18.f);
    m_previewB.setPosition(cx + 90.f, previewY);
    m_previewB.setFillColor(m_enemyColorB);
    m_previewB.setOutlineThickness(2.f);
    m_previewB.setOutlineColor(sf::Color::White);

    setupSlider(m_sliderA_R, "A - R", {cx, colStartY},              float(m_enemyColorA.r)/255.f, 0.f, 255.f, sf::Color(255,80,80));
    setupSlider(m_sliderA_G, "A - G", {cx, colStartY + rowGap},      float(m_enemyColorA.g)/255.f, 0.f, 255.f, sf::Color(80,255,80));
    setupSlider(m_sliderA_B, "A - B", {cx, colStartY + rowGap*2.f},  float(m_enemyColorA.b)/255.f, 0.f, 255.f, sf::Color(80,160,255));

    setupSlider(m_sliderB_R, "B - R", {cx, colStartY + rowGap*3.4f}, float(m_enemyColorB.r)/255.f, 0.f, 255.f, sf::Color(255,80,80));
    setupSlider(m_sliderB_G, "B - G", {cx, colStartY + rowGap*4.4f}, float(m_enemyColorB.g)/255.f, 0.f, 255.f, sf::Color(80,255,80));
    setupSlider(m_sliderB_B, "B - B", {cx, colStartY + rowGap*5.4f}, float(m_enemyColorB.b)/255.f, 0.f, 255.f, sf::Color(80,160,255));

    float diffStartY = cy - 60.f;
    initButton(m_btnDiffEasy,   "Easy",   {cx, diffStartY},          {280.f, 60.f}, 0.f);
    initButton(m_btnDiffMedium, "Medium", {cx, diffStartY + 80.f},   {280.f, 60.f}, 0.f);
    initButton(m_btnDiffHard,   "Hard",   {cx, diffStartY + 160.f},  {280.f, 60.f}, 0.f);
    for (Button* b : { &m_btnDiffEasy, &m_btnDiffMedium, &m_btnDiffHard }) {
        b->alpha = 255.f;
        b->slideOffset = 0.f;
        b->animDone = true;
        b->label.setFillColor(sf::Color::White);
        b->box.setFillColor(sf::Color(0, 0, 0, 150));
    }
}

// Helper umum untuk setup 1 slider
void Menu::setupSlider(Slider& s, const std::string& labelText,
                        sf::Vector2f pos, float initialValue,
                        float minV, float maxV, sf::Color fillColor)
{
    s.label.setFont(m_font);
    s.label.setString(labelText);
    s.label.setCharacterSize(18);
    s.label.setFillColor(sf::Color::White);
    sf::FloatRect lb = s.label.getLocalBounds();
    s.label.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
    s.label.setPosition(pos.x, pos.y - 22.f);

    s.barBg.setSize({260.f, 12.f});
    s.barBg.setFillColor(sf::Color(70, 70, 70));
    s.barBg.setOrigin(s.barBg.getSize() / 2.f);
    s.barBg.setPosition(pos);

    s.minValue   = minV;
    s.maxValue   = maxV;
    s.fillColor  = fillColor;

    // initialValue diasumsikan sudah normalized 0..1
    s.value = std::clamp(initialValue, 0.f, 1.f);

    s.barFill.setSize({260.f * s.value, 12.f});
    s.barFill.setFillColor(fillColor);
    s.barFill.setOrigin(0.f, s.barFill.getSize().y / 2.f);
    s.barFill.setPosition(pos.x - 130.f, pos.y);
}

void Menu::updateSliderVisual(Slider& s)
{
    s.barFill.setSize({260.f * s.value, 12.f});
}

void Menu::handleSliderDrag(Slider& s, const sf::Vector2f& mp)
{
    float left = s.barBg.getPosition().x - s.barBg.getSize().x / 2.f;
    float rel  = std::clamp((mp.x - left) / s.barBg.getSize().x, 0.f, 1.f);
    s.value = rel;
    updateSliderVisual(s);
}
void Menu::openSettingsPage(SettingsPage page)
{
    m_settingsPage = page;
}

void Menu::updateTitleAnim(float dt)
{
    if (!m_titleAnimDone) {
        m_titleAlpha = std::min(m_titleAlpha + dt * 300.f, 255.f);
        if (m_titleAlpha >= 255.f) m_titleAnimDone = true;
    }

    m_titleBobTimer += dt;
    float bobY = std::sin(m_titleBobTimer * 2.0f) * 6.f;
    float flicker = 1.f + std::sin(m_titleBobTimer * 13.f) * 0.08f;

    float cx    = m_window.getSize().x / 2.f;
    float baseY = float(m_window.getSize().y) * 0.22f;

    auto setPos = [&](sf::Text& t, float ox, float oy) {
        t.setPosition(cx + ox, baseY + bobY + oy);
    };
    setPos(m_titleGlow2,  0.f, 0.f);
    setPos(m_titleGlow1,  0.f, 0.f);
    setPos(m_titleGlow0,  0.f, 0.f);
    setPos(m_titleText,   0.f, 0.f);
    setPos(m_titleShadow, 4.f, 6.f);

    auto setAlpha = [&](sf::Text& t, float maxA) {
        sf::Color c = t.getFillColor();
        c.a = static_cast<sf::Uint8>(std::min(m_titleAlpha * maxA / 255.f, 255.f));
        t.setFillColor(c);
    };

    setAlpha(m_titleShadow, m_titleAlpha * 0.5f);
    setAlpha(m_titleGlow2,  m_titleAlpha * 0.5f * flicker);
    setAlpha(m_titleGlow1,  m_titleAlpha * 0.7f * flicker);
    setAlpha(m_titleGlow0,  m_titleAlpha * 0.9f);
    setAlpha(m_titleText,   m_titleAlpha);
}
void Menu::updateButtonAnims(float dt)
{
    m_introTimer += dt;

    auto animateBtn = [&](Button& b) {
        if (m_introTimer < b.animDelay) return;

        b.slideOffset += (0.f - b.slideOffset) * 8.f * dt;
        if (std::abs(b.slideOffset) < 0.3f) {
            b.slideOffset = 0.f;
            b.animDone    = true;
        }

        b.alpha = std::min(b.alpha + dt * 300.f, 255.f);

        b.scaleTarget  = b.hovered ? 1.06f : 1.f;
        b.scaleCurrent += (b.scaleTarget - b.scaleCurrent) * 12.f * dt;

        applyButtonTransform(b);
    };

    animateBtn(m_btnPlay);
    animateBtn(m_btnSettings);
    animateBtn(m_btnQuit);
}

void Menu::applyButtonTransform(Button& b)
{
    {
        sf::Color c = b.box.getFillColor();
        c.a = static_cast<sf::Uint8>(b.hovered ? 160.f : 120.f * (b.alpha / 255.f));
        b.box.setFillColor(c);

        sf::Color oc = b.box.getOutlineColor();
        oc.a = static_cast<sf::Uint8>((b.hovered ? 200.f : 60.f) * (b.alpha / 255.f));
        b.box.setOutlineColor(oc);
    }
    {
        sf::Color c = b.label.getFillColor();
        c.a = static_cast<sf::Uint8>(b.alpha);
        b.label.setFillColor(c);
    }
}
void Menu::loadPlaylist()
{
    m_playlist.clear();
    for (int i = 0; i < 1; i++) {
        m_playlist.push_back(
            std::string(ASSET_DIR) + "audio/song" + std::to_string(i) + ".ogg"
        );
    }
}

void Menu::playRandomMenuTrack()
{
    if (m_playlist.empty()) return;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, int(m_playlist.size()) - 1);
    int idx = dist(gen);

    if (!m_music.openFromFile(m_playlist[idx])) {
        std::cerr << "Failed open music: " << m_playlist[idx] << "\n";
    } else {
        m_music.setLoop(true);
        m_music.setVolume(m_sliderVolume.value * 100.f);
        m_music.play();
    }
}

void Menu::stopMusic() { m_music.stop(); }

void Menu::resumeMusic()
{
    if (m_music.getStatus() != sf::Music::Status::Playing) {
        m_music.play();
    }
}
void Menu::loadSfx()
{
    if (!m_sfxClickBuffer.loadFromFile( std::string(ASSET_DIR) + "audio/click.ogg")) {
        std::cerr << "Failed to load sfx/click.ogg\n";
    } else {
        m_sfxClick.setBuffer(m_sfxClickBuffer);
        m_sfxClick.setVolume(80.f);
    }

    if (!m_sfxHoverBuffer.loadFromFile( std::string(ASSET_DIR) + "audio/hover.ogg")) {
        std::cerr << "Failed to load sfx/hover.ogg\n";
    } else {
        m_sfxHover.setBuffer(m_sfxHoverBuffer);
        m_sfxHover.setVolume(50.f);
    }

    if (!m_sfxSliderBuffer.loadFromFile( std::string(ASSET_DIR) + "audio/slider_tick.ogg")) {
        std::cerr << "Failed to load sfx/slider_tick.ogg\n";
    } else {
        m_sfxSlider.setBuffer(m_sfxSliderBuffer);
        m_sfxSlider.setVolume(40.f);
    }
}

void Menu::playClickSfx()
{
    if (m_sfxClickBuffer.getDuration() != sf::Time::Zero) {
        m_sfxClick.play();
    }
}

void Menu::playHoverSfx()
{
    if (m_sfxHoverBuffer.getDuration() != sf::Time::Zero) {
        m_sfxHover.play();
    }
}

void Menu::playSliderSfx()
{
    if (m_sfxSliderBuffer.getDuration() == sf::Time::Zero) return;
    if (m_sliderSfxCooldown > 0.f) return;   // throttle, jangan tiap frame

    m_sfxSlider.play();
    m_sliderSfxCooldown = kSliderSfxInterval;
}
void Menu::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::Closed) {
        if (onQuit) onQuit();
        return;
    }
    if (m_settingsPage != SettingsPage::None) {

        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mp = sf::Vector2f(sf::Mouse::getPosition(m_window));

            // Tombol back (selalu ada kecuali di hub level teratas, tapi
            // di hub back -> tutup settings sepenuhnya)
            if (pointInRect(mp, m_btnBack.box.getGlobalBounds())) {
                playClickSfx();
                if (m_settingsPage == SettingsPage::Hub)
                    m_settingsPage = SettingsPage::None;   // tutup popup
                else
                    m_settingsPage = SettingsPage::Hub;    // balik ke hub
                return;
            }

            if (m_settingsPage == SettingsPage::Hub) {
                if (pointInRect(mp, m_btnOpenVolume.box.getGlobalBounds())) {
                    playClickSfx();
                    openSettingsPage(SettingsPage::Volume);
                }
                else if (pointInRect(mp, m_btnOpenSpeed.box.getGlobalBounds())) {
                    playClickSfx();
                    openSettingsPage(SettingsPage::Speed);
                }
                else if (pointInRect(mp, m_btnOpenColor.box.getGlobalBounds())) {
                    playClickSfx();
                    openSettingsPage(SettingsPage::Color);
                }
                else if (pointInRect(mp, m_btnOpenDifficulty.box.getGlobalBounds())) {
                    playClickSfx();
                    openSettingsPage(SettingsPage::Difficulty);
                }
            }
            else if (m_settingsPage == SettingsPage::Volume) {
                if (pointInRect(mp, m_sliderVolume.barBg.getGlobalBounds())) {
                    m_draggingSlider = &m_sliderVolume;
                    handleSliderDrag(m_sliderVolume, mp);
                    m_music.setVolume(m_sliderVolume.value * 100.f);
                    playSliderSfx();
                }
            }
            else if (m_settingsPage == SettingsPage::Speed) {
                if (pointInRect(mp, m_sliderSpeed.barBg.getGlobalBounds())) {
                    m_draggingSlider = &m_sliderSpeed;
                    handleSliderDrag(m_sliderSpeed, mp);
                    m_enemySpeed = kSpeedMin + m_sliderSpeed.value * (kSpeedMax - kSpeedMin);
                    playSliderSfx();
                }
            }
            else if (m_settingsPage == SettingsPage::Color) {
                Slider* sliders[6] = {
                    &m_sliderA_R, &m_sliderA_G, &m_sliderA_B,
                    &m_sliderB_R, &m_sliderB_G, &m_sliderB_B
                };
                for (Slider* s : sliders) {
                    if (pointInRect(mp, s->barBg.getGlobalBounds())) {
                        m_draggingSlider = s;
                        handleSliderDrag(*s, mp);
                        playSliderSfx();
                        break;
                    }
                }
                // Update warna langsung setiap drag mulai
                m_enemyColorA = sf::Color(
                    sf::Uint8(m_sliderA_R.value * 255.f),
                    sf::Uint8(m_sliderA_G.value * 255.f),
                    sf::Uint8(m_sliderA_B.value * 255.f)
                );
                m_enemyColorB = sf::Color(
                    sf::Uint8(m_sliderB_R.value * 255.f),
                    sf::Uint8(m_sliderB_G.value * 255.f),
                    sf::Uint8(m_sliderB_B.value * 255.f)
                );
                m_previewA.setFillColor(m_enemyColorA);
                m_previewB.setFillColor(m_enemyColorB);
            }
            else if (m_settingsPage == SettingsPage::Difficulty) {
                if (pointInRect(mp, m_btnDiffEasy.box.getGlobalBounds())) {
                    playClickSfx();
                    m_difficulty = Difficulty::Easy;
                }
                else if (pointInRect(mp, m_btnDiffMedium.box.getGlobalBounds())) {
                    playClickSfx();
                    m_difficulty = Difficulty::Medium;
                }
                else if (pointInRect(mp, m_btnDiffHard.box.getGlobalBounds())) {
                    playClickSfx();
                    m_difficulty = Difficulty::Hard;
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            m_draggingSlider = nullptr;
        }
        else if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mp(float(event.mouseMove.x), float(event.mouseMove.y));

            // Hover untuk tombol di page yang sedang aktif
            auto hoverCheck = [&](Button& b) {
                bool wasHovered = b.hovered;
                b.hovered = pointInRect(mp, b.box.getGlobalBounds());
                if (b.hovered && !wasHovered) playHoverSfx();
            };
            hoverCheck(m_btnBack);
            if (m_settingsPage == SettingsPage::Hub) {
                hoverCheck(m_btnOpenVolume);
                hoverCheck(m_btnOpenSpeed);
                hoverCheck(m_btnOpenColor);
                hoverCheck(m_btnOpenDifficulty);
            }
            else if (m_settingsPage == SettingsPage::Difficulty) {
                hoverCheck(m_btnDiffEasy);
                hoverCheck(m_btnDiffMedium);
                hoverCheck(m_btnDiffHard);
            }

            if (m_draggingSlider) {
                handleSliderDrag(*m_draggingSlider, mp);
                playSliderSfx();

                if (m_draggingSlider == &m_sliderVolume) {
                    m_music.setVolume(m_sliderVolume.value * 100.f);
                }
                else if (m_draggingSlider == &m_sliderSpeed) {
                    m_enemySpeed = kSpeedMin + m_sliderSpeed.value * (kSpeedMax - kSpeedMin);
                }
                else {
                    // salah satu slider RGB sedang di-drag
                    m_enemyColorA = sf::Color(
                        sf::Uint8(m_sliderA_R.value * 255.f),
                        sf::Uint8(m_sliderA_G.value * 255.f),
                        sf::Uint8(m_sliderA_B.value * 255.f)
                    );
                    m_enemyColorB = sf::Color(
                        sf::Uint8(m_sliderB_R.value * 255.f),
                        sf::Uint8(m_sliderB_G.value * 255.f),
                        sf::Uint8(m_sliderB_B.value * 255.f)
                    );
                    m_previewA.setFillColor(m_enemyColorA);
                    m_previewB.setFillColor(m_enemyColorB);
                }
            }
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                if (m_settingsPage == SettingsPage::Hub)
                    m_settingsPage = SettingsPage::None;
                else
                    m_settingsPage = SettingsPage::Hub;
            }
        }
        return; 
    }
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mp = sf::Vector2f(sf::Mouse::getPosition(m_window));

        if (pointInRect(mp, m_btnPlay.box.getGlobalBounds())) {
            playClickSfx();
            if (onPlay) onPlay();
        } else if (pointInRect(mp, m_btnSettings.box.getGlobalBounds())) {
            playClickSfx();
            openSettingsPage(SettingsPage::Hub);
        } else if (pointInRect(mp, m_btnQuit.box.getGlobalBounds())) {
            playClickSfx();
            if (onQuit) onQuit();
        }
    }
    else if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mp(float(event.mouseMove.x), float(event.mouseMove.y));
        updateButtonHover(mp);
    }
    else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            if (onQuit) onQuit();
        }
    }
}
void Menu::update(sf::Time dt)
{
    float secs = dt.asSeconds();

    updateTitleAnim(secs);
    updateButtonAnims(secs);
    updateParticles(dt);

    if (m_sliderSfxCooldown > 0.f) {
        m_sliderSfxCooldown -= secs;
    }

    static float spawnTimer = 0.f;
    spawnTimer += secs;
    if (spawnTimer > 0.06f) {
        spawnParticle();
        spawnTimer = 0.f;
    }
}
void Menu::draw()
{
    m_window.draw(m_bgSprite);
    drawParticles();

    if (m_settingsPage == SettingsPage::None) {
        sf::RenderStates neonBlend;
        neonBlend.blendMode = sf::BlendAdd;

        m_window.draw(m_titleShadow);
        m_window.draw(m_titleGlow2, neonBlend);
        m_window.draw(m_titleGlow1, neonBlend);
        m_window.draw(m_titleGlow0, neonBlend);
        m_window.draw(m_titleText);

        auto drawBtn = [&](Button& b) {
            sf::Vector2f center = b.basePos;
            sf::Transform tr;
            tr.translate(center);
            tr.scale(b.scaleCurrent, b.scaleCurrent);
            tr.translate(-center);
            tr.translate(0.f, b.slideOffset);
            m_window.draw(b.box,   tr);
            m_window.draw(b.label, tr);
        };

        drawBtn(m_btnPlay);
        drawBtn(m_btnSettings);
        drawBtn(m_btnQuit);
    }
    if (m_settingsPage != SettingsPage::None) {
        m_window.draw(m_popupBg);

        if (m_settingsPage == SettingsPage::Hub)        drawSettingsHub();
        else if (m_settingsPage == SettingsPage::Volume) drawSettingsVolume();
        else if (m_settingsPage == SettingsPage::Speed)  drawSettingsSpeed();
        else if (m_settingsPage == SettingsPage::Color)  drawSettingsColor();
        else if (m_settingsPage == SettingsPage::Difficulty) drawSettingsDifficulty();

        m_window.draw(m_btnBack.box);
        m_window.draw(m_btnBack.label);
    }
}

void Menu::drawSettingsHub()
{
    m_popupTitle.setString("Settings");
    sf::FloatRect pt = m_popupTitle.getLocalBounds();
    m_popupTitle.setOrigin(pt.left + pt.width / 2.f, pt.top + pt.height / 2.f);
    m_popupTitle.setPosition(m_popupBg.getPosition().x, m_popupBg.getPosition().y - 190.f);
    m_window.draw(m_popupTitle);

    auto drawSimple = [&](Button& b) {
        sf::Color boxCol = b.hovered ? sf::Color(30,30,80,200) : sf::Color(0,0,0,150);
        b.box.setFillColor(boxCol);
        m_window.draw(b.box);
        m_window.draw(b.label);
    };
    drawSimple(m_btnOpenVolume);
    drawSimple(m_btnOpenSpeed);
    drawSimple(m_btnOpenColor);
    drawSimple(m_btnOpenDifficulty);
}

void Menu::drawSettingsDifficulty()
{
    m_popupTitle.setString("Difficulty");
    sf::FloatRect pt = m_popupTitle.getLocalBounds();
    m_popupTitle.setOrigin(pt.left + pt.width / 2.f, pt.top + pt.height / 2.f);
    m_popupTitle.setPosition(m_popupBg.getPosition().x, m_popupBg.getPosition().y - 190.f);
    m_window.draw(m_popupTitle);

    // Highlight tombol yang sedang aktif dengan warna berbeda, sisanya netral
    auto drawDiffBtn = [&](Button& b, Difficulty d, sf::Color activeColor) {
        bool isActive = (m_difficulty == d);
        sf::Color boxCol;
        if (isActive)       boxCol = sf::Color(activeColor.r, activeColor.g, activeColor.b, 160);
        else if (b.hovered) boxCol = sf::Color(30, 30, 80, 200);
        else                boxCol = sf::Color(0, 0, 0, 150);

        b.box.setFillColor(boxCol);
        b.box.setOutlineColor(isActive ? sf::Color::White : sf::Color(255,255,255,60));
        m_window.draw(b.box);
        m_window.draw(b.label);
    };

    drawDiffBtn(m_btnDiffEasy,   Difficulty::Easy,   sf::Color(80, 220, 120));
    drawDiffBtn(m_btnDiffMedium, Difficulty::Medium, sf::Color(255, 200, 80));
    drawDiffBtn(m_btnDiffHard,   Difficulty::Hard,   sf::Color(255, 80, 80));
}

void Menu::drawSettingsVolume()
{
    m_popupTitle.setString("Volume");
    sf::FloatRect pt = m_popupTitle.getLocalBounds();
    m_popupTitle.setOrigin(pt.left + pt.width / 2.f, pt.top + pt.height / 2.f);
    m_popupTitle.setPosition(m_popupBg.getPosition().x, m_popupBg.getPosition().y - 190.f);
    m_window.draw(m_popupTitle);

    m_window.draw(m_sliderVolume.label);
    m_window.draw(m_sliderVolume.barBg);
    m_window.draw(m_sliderVolume.barFill);

    // Tampilkan nilai persen
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d%%", int(m_sliderVolume.value * 100.f));
    sf::Text valText;
    valText.setFont(m_font);
    valText.setString(buf);
    valText.setCharacterSize(16);
    valText.setFillColor(sf::Color(200,200,200));
    sf::FloatRect vb = valText.getLocalBounds();
    valText.setOrigin(vb.left + vb.width/2.f, vb.top + vb.height/2.f);
    valText.setPosition(m_sliderVolume.barBg.getPosition().x, m_sliderVolume.barBg.getPosition().y + 30.f);
    m_window.draw(valText);
}

void Menu::drawSettingsSpeed()
{
    m_popupTitle.setString("Enemy Speed");
    sf::FloatRect pt = m_popupTitle.getLocalBounds();
    m_popupTitle.setOrigin(pt.left + pt.width / 2.f, pt.top + pt.height / 2.f);
    m_popupTitle.setPosition(m_popupBg.getPosition().x, m_popupBg.getPosition().y - 190.f);
    m_window.draw(m_popupTitle);

    m_window.draw(m_sliderSpeed.label);
    m_window.draw(m_sliderSpeed.barBg);
    m_window.draw(m_sliderSpeed.barFill);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.0f px/s", m_enemySpeed);
    sf::Text valText;
    valText.setFont(m_font);
    valText.setString(buf);
    valText.setCharacterSize(16);
    valText.setFillColor(sf::Color(200,200,200));
    sf::FloatRect vb = valText.getLocalBounds();
    valText.setOrigin(vb.left + vb.width/2.f, vb.top + vb.height/2.f);
    valText.setPosition(m_sliderSpeed.barBg.getPosition().x, m_sliderSpeed.barBg.getPosition().y + 30.f);
    m_window.draw(valText);
}

void Menu::drawSettingsColor()
{
    m_popupTitle.setString("Enemy Colors");
    sf::FloatRect pt = m_popupTitle.getLocalBounds();
    m_popupTitle.setOrigin(pt.left + pt.width / 2.f, pt.top + pt.height / 2.f);
    m_popupTitle.setPosition(m_popupBg.getPosition().x, m_popupBg.getPosition().y - 190.f);
    m_window.draw(m_popupTitle);

    m_window.draw(m_previewA);
    m_window.draw(m_previewB);

    Slider* sliders[6] = {
        &m_sliderA_R, &m_sliderA_G, &m_sliderA_B,
        &m_sliderB_R, &m_sliderB_G, &m_sliderB_B
    };
    for (Slider* s : sliders) {
        m_window.draw(s->label);
        m_window.draw(s->barBg);
        m_window.draw(s->barFill);
    }
}

void Menu::spawnParticle()
{
    Particle p;
    sf::Vector2u win = m_window.getSize();
    float x = float(rand() % win.x);
    float y = float(win.y) * (0.6f + (rand() % 40) / 100.f);
    p.pos = {x, y};
    float vx = (rand() % 100 - 50) / 50.f * 20.f;
    float vy = -(20.f + (rand() % 50));
    p.vel    = {vx, vy};
    p.life   = 1.2f + (rand() % 100) / 100.f * 1.8f;
    p.radius = 2.f + (rand() % 6);
    p.color  = sf::Color(150 + rand() % 100, 200 + rand() % 55, 180 + rand() % 60, 200);
    m_particles.push_back(p);

    if (m_particles.size() > 400)
        m_particles.erase(m_particles.begin(), m_particles.begin() + 50);
}

void Menu::updateParticles(sf::Time dt)
{
    float secs = dt.asSeconds();
    for (auto& p : m_particles) {
        p.pos  += p.vel * secs;
        p.life -= secs;
        p.vel.y += 10.f * secs;
        if (p.life < 0.6f) {
            float alphaFactor = std::max(0.f, p.life / 0.6f);
            p.color.a = sf::Uint8(255.f * alphaFactor);
        }
    }
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& pp){ return pp.life <= 0.f; }),
        m_particles.end()
    );
}

void Menu::drawParticles()
{
    sf::RenderStates rs;
    rs.blendMode = sf::BlendAdd;
    sf::CircleShape shape;
    for (const auto& p : m_particles) {
        shape.setRadius(p.radius);
        shape.setOrigin(p.radius, p.radius);
        shape.setPosition(p.pos);
        shape.setFillColor(p.color);
        m_window.draw(shape, rs);
    }
}
void Menu::updateButtonHover(const sf::Vector2f& mousePos)
{
    auto hoverSet = [&](Button& b) {
        bool wasHovered = b.hovered;
        b.hovered = pointInRect(mousePos, b.box.getGlobalBounds());
        if (b.hovered && !wasHovered) playHoverSfx();

        if (b.hovered) {
            b.box.setFillColor(sf::Color(30, 30, 80, static_cast<sf::Uint8>(b.alpha * 0.63f)));
            b.box.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(b.alpha)));
        } else {
            b.box.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(b.alpha * 0.47f)));
            b.box.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(b.alpha * 0.24f)));
        }
    };

    hoverSet(m_btnPlay);
    hoverSet(m_btnSettings);
    hoverSet(m_btnQuit);
}

bool Menu::pointInRect(const sf::Vector2f& p, const sf::FloatRect& r) const
{
    return r.contains(p);
}