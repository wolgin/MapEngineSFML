#include <iostream>
#include "UI/Element.hpp"
//#include "Math/Geodesic.hpp"

#define WIDTH 900u
#define HEIGHT 600u

void test1() {

    auto canvasTxr = sf::Texture(sf::Image({ WIDTH, HEIGHT }, sf::Color::Black));
    auto canvas = ui::Element("canvas", &canvasTxr);

    auto labelTxr = sf::Texture(sf::Image({ 100u, 100u }, sf::Color::Magenta));
    auto label = ui::Draggable("label", &labelTxr);

    auto buttonTxr = sf::Texture(sf::Image({ 50u, 50u }, sf::Color::Yellow));
    auto button = ui::Button("button", &buttonTxr);

    auto stretch = [&button]() {
        button.resample({ { 0.0f, 0.0f }, { 0.0f, 50.0f }, { 150.0f, 50.0f }, { 150.0f, 0.0f } });
    };

    auto shrink = [&button]() {
        button.resample({ { 0.0f, 0.0f }, { 0.0f, 50.0f }, { 50.0f, 50.0f }, { 50.0f, 0.0f } });
    };

    bool mode = false;

    button.setLeftClick(
        [&mode, &stretch, &shrink]() {
            (mode = !mode) ? stretch() : shrink();
        }
    );

    canvas.attach("label");
    label.attach("button");

    canvas.enable();
    label.enable();
    button.enable();

    label.move({ 100.0, 100.0f });
    button.move({ 25.0f, 25.0f });

    auto window = sf::RenderWindow(sf::VideoMode({ WIDTH, HEIGHT }), "test");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent())
            if (event->is<sf::Event::Closed>()) window.close();
            else canvas.handle(event);
        window.clear(sf::Color::Black);
        window.draw(canvas);
        window.display();
    }
}

int main() {

    test1();
    return 0;
}
