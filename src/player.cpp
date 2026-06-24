#include "player.hpp"
#include <iostream>

Player::Player(sf::RenderWindow& window)
: m_window(window)
{
    loadAssets();

    sf::Vector2u win = m_window.getSize();
    float groundY = float(win.y) * 0.75f;

    // Pakai ukuran frame dari texture run pertama
    int fh = int(m_texRun[0].getSize().y);
    int fw = int(m_texRun[0].getSize().x);
    m_position = { float(win.x) * 0.25f, groundY - float(fh) };

    applyFrame();
    m_sprite.setPosition(m_position);
}

void Player::loadAssets()
{

    for (int i = 0; i < m_runFrameCount; i++) {
        std::string path = std::string(ASSET_DIR) + "texture/run_"
                           + std::to_string(i) + ".png";
        if (!m_texRun[i].loadFromFile(path)) {
            std::cerr << "Failed to load " << path << "\n";
        }
    }

    if (!m_texPunch.loadFromFile(std::string(ASSET_DIR) + "texture/punch.png")) {
        std::cerr << "Failed to load texture/punch.png\n";
    }

    if (!m_texKick.loadFromFile(std::string(ASSET_DIR) + "texture/kick.png")) {
        std::cerr << "Failed to load texture/kick.png\n";
    }

    // Mulai dengan frame lari pertama
    m_sprite.setTexture(m_texRun[0]);
}

void Player::handleEvent(const sf::Event& event)
{
    if (event.type != sf::Event::KeyPressed) return;
    if (m_action != Action::Run) return;

    if (event.key.code == sf::Keyboard::A) {
        m_action      = Action::Punch;
        m_actionTimer = 0.f;
        applyFrame();
    }
    else if (event.key.code == sf::Keyboard::D) {
        m_action      = Action::Kick;
        m_actionTimer = 0.f;
        applyFrame();
    }
}
void Player::update(sf::Time dt)
{
    float secs = dt.asSeconds();

    if (m_action == Action::Run)
        updateRun(secs);
    else
        updateAction(secs);
}

void Player::updateRun(float dt)
{
    m_frameTimer += dt;
    if (m_frameTimer >= m_frameDuration) {
        m_frameTimer  -= m_frameDuration;
        m_currentFrame = (m_currentFrame + 1) % m_runFrameCount;
        applyFrame();
    }
}

void Player::updateAction(float dt)
{
    m_actionTimer += dt;
    if (m_actionTimer >= m_actionDuration) {
        m_action       = Action::Run;
        m_currentFrame = 0;
        m_frameTimer   = 0.f;
        applyFrame();
    }
}
void Player::applyFrame()
{
    switch (m_action) {

    case Action::Run:
        // Ganti texture ke file frame yang sesuai, full rect
        m_sprite.setTexture(m_texRun[m_currentFrame]);
        m_sprite.setTextureRect(sf::IntRect(
            0, 0,
            int(m_texRun[m_currentFrame].getSize().x),
            int(m_texRun[m_currentFrame].getSize().y)
        ));
        break;

    case Action::Punch:
        m_sprite.setTexture(m_texPunch);
        m_sprite.setTextureRect(sf::IntRect(
            0, 0,
            int(m_texPunch.getSize().x),
            int(m_texPunch.getSize().y)
        ));
        break;

    case Action::Kick:
        m_sprite.setTexture(m_texKick);
        m_sprite.setTextureRect(sf::IntRect(
            0, 0,
            int(m_texKick.getSize().x),
            int(m_texKick.getSize().y)
        ));
        break;
    }
}
void Player::draw()
{
    m_window.draw(m_sprite);
}