#pragma once
#include <map>
#include <list>
#include <functional>
#include <SFML/Graphics.hpp>

namespace ui {

    class Contour : public sf::Drawable {

        private:

            sf::Texture texture;
            std::vector<sf::Vertex> vertices;
            sf::VertexBuffer buffer;

        public:

            Contour(
                const sf::Texture &_texture,
                const std::vector<sf::Vector2f> &_outline = std::vector<sf::Vector2f>(),
                const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping = [](const sf::Vector2f &_v) { return _v; }
            );

            Contour(const Contour &) = delete;
            Contour(Contour &&) = delete;
            virtual ~Contour() = default;

            bool contains(const sf::Vector2f &_position) const;
            virtual void draw(sf::RenderTarget &_target, sf::RenderStates _states) const override;
    };

    class Element : public Contour, public sf::Transformable {

        private:

            static std::map<std::string, Element *> elements;
            std::list<Element *> children;
            bool active;

        protected:

            virtual bool mouseMoved(const sf::Vector2f &_pos) { return false; }
            virtual bool mouseScrolled(const sf::Vector2f &_pos, float _delta) { return false; }
            virtual bool mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) { return false; }
            virtual bool mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) { return false; }
            virtual bool keyPressed(sf::Keyboard::Key _key, bool _ctrl, bool _alt, bool _shift) { return false; }
            virtual bool keyReleased(sf::Keyboard::Key _key, bool _ctrl, bool _alt, bool _shift) { return false; }

        public:

            Element(
                const std::string &_id,
                const sf::Texture &_texture,
                const std::vector<sf::Vector2f> &_outline = std::vector<sf::Vector2f>(),
                const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping = [](const sf::Vector2f &_v) { return _v; }
            ) :
                active(true),
                Contour(_texture, _outline, _mapping)
            {
                elements[_id] = this;
            }

            virtual ~Element() = default;

            void enable() { active = true; }
            void disable() { active = false; }

            void attach(const std::string &_id) { children.push_back(elements[_id]); }
            void detach(const std::string &_id) { children.remove(elements[_id]); }

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
                const sf::Texture &_texture,
                const std::function<void(void)> &_leftClick = []() { return false; },
                const std::function<void(void)> &_rightClick = []() { return false; },
                const std::vector<sf::Vector2f> &_outline = std::vector<sf::Vector2f>(),
                const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping = [](const sf::Vector2f &_v) { return _v; }
            ) :
                leftPressed(false),
                rightPressed(false),
                leftClick(_leftClick),
                rightClick(_rightClick),
                Element(_id, _texture, _outline, _mapping)
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
                const sf::Texture &_texture,
                const std::vector<sf::Vector2f> &_outline = std::vector<sf::Vector2f>(),
                const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping = [](const sf::Vector2f &_v) { return _v; }
            ) :
                Element(_id, _texture, _outline, _mapping)
            {}

            virtual ~Draggable() = default;
    };
}
