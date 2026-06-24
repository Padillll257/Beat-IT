#include "stage.hpp"
#include <iostream>

Stage::Stage(sf::RenderWindow& window)
: m_window(window)
, m_scrollSpeed(200.f)
, m_scrollX(0.f)
, m_tileWidth(0.f)
, m_player(window)
, m_enemies(window, float(window.getSize().y) * 0.75f)
, m_hud(window)
{
    // Load font untuk Result Screen (SESUAIKAN PATH FONT KAMU)
    if (!m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        std::cerr << "Gagal meload font untuk result screen!\n";
    }

    setupBackground();
    loadPlaylist();
    playMusic();
}

void Stage::setEnemySpeed(float speed)
{
    m_enemies.setSpeed(speed);  }

void Stage::setEnemyColors(sf::Color colorA, sf::Color colorB)
{
    m_enemies.setColors(colorA, colorB);    }

void Stage::setDifficulty(Difficulty diff)
{
    m_enemies.setDifficulty(diff);  }

void Stage::setupBackground()
{
    sf::Vector2u win = m_window.getSize();

    if (!m_bgTex1.loadFromFile(std::string(ASSET_DIR) + "background/stage1.png")) {
        std::cerr << "Failed to load assets/background/stage1.png\n";
    }
    if (!m_bgTex2.loadFromFile(std::string(ASSET_DIR) + "background/stage1_rotated.png")) {
        std::cerr << "Failed to load assets/background/stage1_rotated.png\n";
    }

    auto scaleToWindow = [&](sf::Sprite& spr, sf::Texture& tex) {
        sf::Vector2u texSize = tex.getSize();
        spr.setTexture(tex);
        float scaleY = float(win.y) / float(texSize.y);
        spr.setScale(scaleY, scaleY);
    };

    scaleToWindow(m_bgSprite1, m_bgTex1);
    scaleToWindow(m_bgSprite2, m_bgTex2);

    m_tileWidth = m_bgSprite1.getGlobalBounds().width;

    m_scrollX = 0.f;
    m_bgSprite1.setPosition(m_scrollX, 0.f);
    m_bgSprite2.setPosition(m_scrollX + m_tileWidth, 0.f);
}

void Stage::updateBackground(float dt)
{
    m_bgSprite1.move(-m_scrollSpeed * dt, 0.f);
    m_bgSprite2.move(-m_scrollSpeed * dt, 0.f);

    if (m_bgSprite1.getPosition().x <= -m_tileWidth) {
        m_bgSprite1.setPosition(m_bgSprite2.getPosition().x + m_tileWidth, 0.f);
    }
    if (m_bgSprite2.getPosition().x <= -m_tileWidth) {
        m_bgSprite2.setPosition(m_bgSprite1.getPosition().x + m_tileWidth, 0.f);
    }
}

void Stage::drawBackground()
{
    m_window.draw(m_bgSprite1);
    m_window.draw(m_bgSprite2);
}

void Stage::loadPlaylist(){
    m_playlist.clear();
    m_playlist.push_back(std::string(ASSET_DIR) + "audio/stage_song.ogg");
}

void Stage::playMusic(){
    if (m_playlist.empty()) return;
    if (!m_music.openFromFile(m_playlist[0])) {
        std::cerr << "Failed open music: " << m_playlist[0] << "\n";
    } else {
        m_music.setLoop(false); // Jangan di-loop agar tau kapan habis
        m_music.setVolume(60.f);
        m_music.play();
    }
}

void Stage::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed) {
        // Kontrol jika sedang di layar hasil
        if (m_state == State::ResultScreen) {
            if (event.key.code == sf::Keyboard::R) {
                if (onReplay) onReplay(); // Panggil fungsi reset/replay
            }
            else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Escape) {
                if (onBackToMenu) onBackToMenu();
            }
            else if (event.key.code == sf::Keyboard::Q) {
                m_window.close(); // Keluar dari game
            }
            return; // Jangan proses input gameplay lagi
        }

        // Kontrol Gameplay Standard
        if (event.key.code == sf::Keyboard::Escape) {
            if (onBackToMenu) onBackToMenu();
        }

        if (event.key.code == sf::Keyboard::A) {
            HitResult result = m_enemies.onPunch();
            if (!result.isHoldNote) {
                m_hud.registerHit(result);
            }
        }
        else if (event.key.code == sf::Keyboard::D) {
            HitResult result = m_enemies.onKick();
            if (!result.isHoldNote) {
                m_hud.registerHit(result);
            }
        }
    }
    else if (event.type == sf::Event::KeyReleased) {
        if (m_state == State::Playing) {
            if (event.key.code == sf::Keyboard::A) {
                HitResult result = m_enemies.onPunchRelease();
                if (result.judgement != Judgement::None) {
                    m_hud.registerHit(result);
                }
            }
            else if (event.key.code == sf::Keyboard::D) {
                HitResult result = m_enemies.onKickRelease();
                if (result.judgement != Judgement::None) {
                    m_hud.registerHit(result);
                }
            }
        }
    }
    m_player.handleEvent(event);
}

void Stage::update(sf::Time dt)
{
    // Jika game masih berjalan
    if (m_state == State::Playing) {
        updateBackground(dt.asSeconds());
        m_player.update(dt);
        m_enemies.update(dt.asSeconds());
        m_hud.update(dt);

        bool isPunchHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        bool isKickHeld  = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

        HitResult punchResult, kickResult;
        m_enemies.updateHolds(dt.asSeconds(), isPunchHeld, isKickHeld, punchResult, kickResult);

        if (punchResult.judgement != Judgement::None) {
            m_hud.registerHit(punchResult);
        }
        if (kickResult.judgement != Judgement::None) {
            m_hud.registerHit(kickResult);
        }

        // PENGECEKAN STAGE SELESAI
        if (m_enemies.isFinished()) {
            m_finishTimer += dt.asSeconds();
            if (m_finishTimer >= 1.0f) { // Tunggu 1 detik sebelum layar hasil muncul
                m_state = State::ResultScreen;
                m_music.stop(); // Hentikan sisa audio jika masih jalan
            }
        }
    }
}

void Stage::draw()
{
    drawBackground();

    m_enemies.drawTargets(); 
    m_enemies.draw();        
    m_player.draw();         
    m_hud.draw();             

    // Menggambar Result Screen di atas segalanya jika state = ResultScreen
    if (m_state == State::ResultScreen) {
        drawResultScreen();
    }
}
void Stage::drawResultScreen()
{
    sf::Vector2f winSize(m_window.getSize().x, m_window.getSize().y);

    // Bikin background semi-transparan buat overlay
    sf::RectangleShape overlay(winSize);
    overlay.setFillColor(sf::Color(0, 0, 0, 180)); // Hitam dengan alpha 180
    m_window.draw(overlay);

    // Bikin kotak panel hasil di tengah
    sf::RectangleShape panel(sf::Vector2f(600.f, 400.f));
    panel.setFillColor(sf::Color(40, 40, 80, 240));
    panel.setOutlineThickness(4.f);
    panel.setOutlineColor(sf::Color(0, 220, 255));
    panel.setOrigin(300.f, 200.f);
    panel.setPosition(winSize.x / 2.f, winSize.y / 2.f);
    m_window.draw(panel);

    // Helper untuk menggambar teks
    auto drawText = [&](std::string str, float yOffset, int size, sf::Color col) {
        sf::Text txt(str, m_font, size);
        txt.setFillColor(col);
        sf::FloatRect bounds = txt.getLocalBounds();
        txt.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        txt.setPosition(winSize.x / 2.f, (winSize.y / 2.f) + yOffset);
        m_window.draw(txt);
    };

    // Ambil data statistik dari HUD
    int score    = m_hud.getScore();
    int maxCombo = m_hud.getMaxCombo();
    int perfects = m_hud.getPerfect();
    int goods    = m_hud.getGood();
    int misses   = m_hud.getMiss();

    // Judul
    drawText("STAGE CLEARED!", -150.f, 40, sf::Color(255, 215, 0));

    // Stats
    drawText("Score: " + std::to_string(score), -70.f, 28, sf::Color::White);
    drawText("Max Combo: " + std::to_string(maxCombo), -30.f, 24, sf::Color::Cyan);
    drawText("Perfect: " + std::to_string(perfects), 10.f, 20, sf::Color(0, 255, 0));
    drawText("Good: " + std::to_string(goods), 40.f, 20, sf::Color(200, 200, 200));
    drawText("Miss: " + std::to_string(misses), 70.f, 20, sf::Color(255, 50, 50));

    // Petunjuk Opsi
    drawText("[ R ] Replay    [ M ] Main Menu    [ Q ] Quit", 150.f, 18, sf::Color(255, 255, 100));
}