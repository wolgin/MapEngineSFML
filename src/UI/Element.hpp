#pragma once
#include <map>
#include <list>
#include <functional>
#include <SFML/Graphics.hpp>



#include <iostream>


namespace ui {

    class Element : public sf::Drawable, public sf::Transformable {

        private:

            bool active;
            const sf::Texture *const texture;
            std::vector<sf::Vertex> vertices;
            sf::VertexBuffer buffer;

        protected:

            static std::map<std::string, Element *> elements;
            std::list<Element *> children;

            virtual bool mouseMoved(const sf::Vector2f &_pos) { return false; }
            virtual bool mouseScrolled(const sf::Vector2f &_pos, float _delta) { return false; }
            virtual bool mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) { return false; }
            virtual bool mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) { return false; }
            virtual bool keyPressed(sf::Keyboard::Key _key, bool _ctrl, bool _alt, bool _shift) { return false; }
            virtual bool keyReleased(sf::Keyboard::Key _key, bool _ctrl, bool _alt, bool _shift) { return false; }

        public:

            Element(
                const std::string &_id,
                const sf::Texture *const _texture
            ) :
                active(false),
                texture(_texture),
                buffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Static)
            {
                elements[_id] = this;
                const float width = static_cast<float>(texture->getSize().x);
                const float height = static_cast<float>(texture->getSize().y);
                resample({ { 0.0f, 0.0f }, { 0.0f, height }, { width, height }, { width, 0.0f } });
            }

            Element(const Element &_element) = delete;
            Element(Element &&_element) = delete;
            virtual ~Element() = default;

            void enable() { active = true; }
            void disable() { active = false; }

            void attach(const std::string &_id) { children.push_back(elements[_id]); }
            void detach(const std::string &_id) { children.remove(elements[_id]); }

            void resample(
                const std::vector<sf::Vector2f> &_outline = std::vector<sf::Vector2f>(),
                const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping = [](const sf::Vector2f &_v) { return _v; }
            );

            bool contains(const sf::Vector2f &_position) const;
            bool handle(const std::optional<sf::Event> &_event, sf::Transform _global = sf::Transform());
            void draw(sf::RenderTarget &_target, sf::RenderStates _states) const override;
    };

    class Button : public Element {

        private:

            bool leftPressed;
            bool rightPressed;
            std::function<void(void)> leftClick;
            std::function<void(void)> rightClick;

            bool mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) override;
            bool mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) override;

        public:

            Button(
                const std::string &_id,
                const sf::Texture *const _texture,
                const std::function<void(void)> &_leftClick = []() { return false; },
                const std::function<void(void)> &_rightClick = []() { return false; }
            ) :
                leftPressed(false),
                rightPressed(false),
                leftClick(_leftClick),
                rightClick(_rightClick),
                Element(_id, _texture)
            {}

            virtual ~Button() = default;

            void setLeftClick(const std::function<void(void)> &_leftClick) { leftClick = _leftClick; }
            void setRightClick(const std::function<void(void)> &_rightClick) { rightClick = _rightClick; }
    };

    class Draggable : public Element {

        private:

            bool dragged;
            sf::Vector2f dragPoint;

            bool mouseMoved(const sf::Vector2f &_pos) override;
            bool mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) override;
            bool mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) override;

        public:

            Draggable(
                const std::string &_id,
                const sf::Texture *const _texture
            ) :
                Element(_id, _texture)
            {}

            virtual ~Draggable() = default;
    };
}
