#ifndef STAGE_HPP
#define STAGE_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <functional>
#include "player.hpp"
#include "enemy.hpp"
#include "hud.hpp"

class Stage {
public:
    Stage(sf::RenderWindow& window);

    void handleEvent(const sf::Event& event);
    void update(sf::Time dt);
    void draw();

    // callback ke main.cpp ketika player tekan Escape
    std::function<void()> onBackToMenu;
    void setEnemySpeed(float speed);
    void setEnemyColors(sf::Color colorA, sf::Color colorB);
    void setDifficulty(Difficulty diff);
    std::function<void()> onReplay;

private:
    enum class State { Playing, ResultScreen };
    State m_state = State::Playing;
    
    float m_finishTimer = 0.f; // Timer untuk jeda 1 detik

    sf::Font m_font; // Untuk tulisan di Result Screen
    
    void setupResultUI();
    void drawResultScreen();
    sf::RenderWindow& m_window;

    // Background
    sf::Texture m_bgTex1;
    sf::Texture m_bgTex2;
    sf::Sprite  m_bgSprite1;
    sf::Sprite  m_bgSprite2;

    float m_scrollSpeed;
    float m_scrollX;
    float m_tileWidth;

    void setupBackground();
    void updateBackground(float dt);
    void drawBackground();

    //Music & Playlist
    std::vector<std::string> m_playlist;
    sf::Music                m_music;
    void loadPlaylist();
    void playMusic();

    //Gameplay Components
    Player       m_player;
    EnemyManager m_enemies;
    Hud          m_hud;
};

#endif