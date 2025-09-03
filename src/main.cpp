#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Game.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Cipher");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    // ImGui dark tweaks
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 8.f;
    style.ScrollbarRounding = 8.f;
    style.WindowRounding = 8.f;

    Game game;

    sf::Clock deltaClock;
    bool wantQuit = false;

    while (window.isOpen() && !wantQuit) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::TextEntered) game.onTextEntered(event.text.unicode);
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) wantQuit = true;
                game.onKeyPressed(event.key.code);
            }
        }

        float dt = deltaClock.restart().asSeconds();
        ImGui::SFML::Update(window, sf::seconds(dt));
        game.update(dt);

        // Check if game wants to quit
        if (game.wantsToQuit()) {
            wantQuit = true;
        }

        window.clear(sf::Color(20, 20, 26)); // dark background
        game.renderUI();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
