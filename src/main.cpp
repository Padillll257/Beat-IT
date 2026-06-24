#include <SFML/Graphics.hpp>
#include "menu.hpp"
#include "stage.hpp"
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Beat It");
    window.setFramerateLimit(60);
    enum class GameState { Menu, Stage };
    GameState state = GameState::Menu;
    Menu  menu(window);
    Stage* stage = nullptr;
    menu.onPlay = [&]() {
        menu.stopMusic();          // stop musik menu dulu, sebelum masuk Stage
        delete stage;
        stage = new Stage(window);
        // Terapkan settings dari menu (Volume/Speed/Color/Difficulty sudah
        // diatur lewat popup Settings) ke Stage yang baru dibuat
        stage->setEnemySpeed(menu.getEnemySpeed());
        stage->setEnemyColors(menu.getEnemyColorA(), menu.getEnemyColorB());
        stage->setDifficulty(menu.getDifficulty());

        // Ketika Escape di dalam Stage → balik ke Menu
        stage->onBackToMenu = [&]() {
            state = GameState::Menu;
            delete stage;
            stage = nullptr;

            menu.resumeMusic();    // main-kan lagi musik menu
        };

        state = GameState::Stage;
    };

    menu.onQuit = [&]() {
        window.close();
    };
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
                break;
            }

            if (state == GameState::Menu)
                menu.handleEvent(e);
            else if (state == GameState::Stage && stage)
                stage->handleEvent(e);
        }

        sf::Time dt = clock.restart();

        if (state == GameState::Menu)
            menu.update(dt);
        else if (state == GameState::Stage && stage)
            stage->update(dt);

        window.clear();

        if (state == GameState::Menu)
            menu.draw();
        else if (state == GameState::Stage && stage)
            stage->draw();

        window.display();
    }

    delete stage;
    return 0;
}