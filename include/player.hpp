#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <SFML/Graphics.hpp>

class Player {
public:
    Player(sf::RenderWindow& window);
    ~Player() = default;

    void handleEvent(const sf::Event& event);
    void update(sf::Time dt);
    void draw();

private:
    sf::RenderWindow& m_window;

    //textures & sprite
    static const int  m_runFrameCount = 6;
    sf::Texture       m_texRun[m_runFrameCount]; // 6 file terpisah
    sf::Texture       m_texPunch;
    sf::Texture       m_texKick;
    sf::Sprite        m_sprite;

    //animation state
    enum class Action { Run, Punch, Kick };
    Action m_action = Action::Run;

    int   m_currentFrame  = 0;
    float m_frameTimer    = 0.f;
    float m_frameDuration = 0.1f;   // detik per frame lari

    float m_actionTimer    = 0.f;
    float m_actionDuration = 0.35f; // detik tampil punch/kick sebelum balik lari

    //poisition
    sf::Vector2f m_position;

    //HELPERS
    void loadAssets();
    void updateRun(float dt);
    void updateAction(float dt);
    void applyFrame();
};

#endif