#include <iostream>
#include "UI/Element.hpp"
//#include "Math/Geodesic.hpp"

void test1() {

    auto renderTexture = sf::RenderTexture({ 900u, 600u });
    auto earth = sf::CircleShape(300.0f);
    earth.setFillColor(sf::Color::Blue);
    earth.setOrigin({ 300.0f, 300.0f });
    earth.move({ 450.0f, 300.0f });
    renderTexture.clear(sf::Color::Black);
    renderTexture.draw(earth);
    renderTexture.display();
    auto canvasTxr = renderTexture.getTexture();
    auto canvas = ui::Element("canvas", canvasTxr);

    auto img = sf::Image({ 150u, 100u }, sf::Color::Magenta);
    auto txr = sf::Texture(img);
    auto form = ui::Draggable("form", txr);
    form.move({ 700.0f, 450.0f });
    form.rotate(sf::radians(0.5f));
/*
    auto callback1 = []() {
        std::cout << "Left mouse form clicked" << std::endl;
        return true;
    };

    auto callback2 = []() {
        std::cout << "Right mouse form clicked" << std::endl;
        return true;
    };

    form.setLeftClick(callback1);
    form.setRightClick(callback2);
*/
    auto image = sf::Image({ 20u, 20u }, sf::Color(255, 0, 0));
    auto texture = sf::Texture(image);

    auto patch = ui::Draggable("patch", texture);
    patch.rotate(sf::radians(0.2f));

    canvas.attach("form");
    form.attach("patch");

    canvas.enable();
    form.enable();
    patch.enable();

    auto contour = ui::Contour(canvasTxr);

    auto window = sf::RenderWindow(sf::VideoMode({ 900u, 600u }), "test");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else
                canvas.handle(event);
        }
        window.clear(sf::Color::Black);
        //window.draw(contour);
        window.draw(canvas);
        window.display();
    }
}


int main() {

    test1();    

    return 0;
}
