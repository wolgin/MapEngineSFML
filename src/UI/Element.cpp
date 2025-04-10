#include "Element.hpp"

namespace ui {

    // Contour

    static const float EPSILON = 1e-10f;

    static std::vector<sf::Vector2f> checkOutline(
        const sf::Texture &tex,
        const std::vector<sf::Vector2f> &out
    ) {
        const float w = static_cast<float>(tex.getSize().x - 1);
        const float h = static_cast<float>(tex.getSize().y - 1);
        const std::vector<sf::Vector2f> textureOutline = { { 0.0f, 0.0f }, { 0.0f, h }, { w, h }, { w, 0.0f } };
        
        if (out.size() < 3) return textureOutline;

        sf::Vector2f min = { w, h }, max = { 0.0f, 0.0f };

        for (auto &pt : out) {
            if (pt.x < min.x) min.x = pt.x;
            if (pt.y < min.y) min.y = pt.y;
            if (pt.x > max.x) max.x = pt.x;
            if (pt.y > max.y) max.y = pt.y;
        }

        if (min.x < 0.0f || min.y < 0.0f || max.x > w || max.y > h)
            return textureOutline;
        
        return out;
    }

    static bool simplexContains(
        const sf::Vector2f &A,
        const sf::Vector2f &B,
        const sf::Vector2f &C,
        const sf::Vector2f &P
    ) {
        const float aCROSSbp = (C.x - B.x) * (P.y - B.y) - (C.y - B.y) * (P.x - B.x);
        const float cCROSSap = (B.x - A.x) * (P.y - A.y) - (B.y - A.y) * (P.x - A.x);
        const float bCROSScp = (A.x - C.x) * (P.y - C.y) - (A.y - C.y) * (P.x - C.x);

        return aCROSSbp >= 0.0f && bCROSScp >= 0.0f && cCROSSap >= 0.0f;
    }

    static bool snip(
        const std::vector<sf::Vector2f> &out,
        size_t u,
        size_t v,
        size_t w,
        size_t nv,
        size_t *V
    ) {
        const sf::Vector2f A = out[V[u]];
        const sf::Vector2f B = out[V[v]];
        const sf::Vector2f C = out[V[w]];

        if ((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x) < EPSILON) return false;

        for (size_t p = 0; p < nv; ++p)
            if (p != u && p != v && p != w && simplexContains(A, B, C, out[V[p]]))
                return false;

        return true;
    };

    Contour::Contour(
        const sf::Texture &_texture,
        const std::vector<sf::Vector2f> &_outline,
        const std::function<sf::Vector2f(const sf::Vector2f &)> &_mapping
    ) :
        texture(_texture)
    {
        const std::vector<sf::Vector2f> out = checkOutline(_texture, _outline);

        size_t n = out.size();
        size_t *V = new size_t[n];

        float orientation = 0.0f;
        for (size_t p = n - 1, q = 0; q < n; p = q++)
            orientation += out[p].x * out[q].y - out[q].x * out[p].y;

        if (orientation > 0.0f)
            for (size_t v = 0; v < n; ++v) V[v] = v;
        else
            for (size_t v = 0; v < n; ++v) V[v] = (n - 1) - v;

        size_t nv = n;

        for (size_t v = nv - 1; nv > 2;) {

            size_t u = v;       if (nv <= u) u = 0;
            v = u + 1;          if (nv <= v) v = 0;
            size_t w = v + 1;   if (nv <= w) w = 0;

            if (snip(out, u, v, w, nv, V)) {

                sf::Vertex a; a.texCoords = out[V[u]]; a.position = _mapping(a.texCoords); vertices.push_back(a);
                sf::Vertex b; b.texCoords = out[V[v]]; b.position = _mapping(b.texCoords); vertices.push_back(b);
                sf::Vertex c; c.texCoords = out[V[w]]; c.position = _mapping(c.texCoords); vertices.push_back(c);

                for (size_t s = v; s + 1 < nv; ++s)
                    V[s] = V[s + 1];

                --nv;
            }
        }
        
        delete[] V;

        buffer = sf::VertexBuffer(sf::PrimitiveType::Triangles, sf::VertexBuffer::Usage::Static);

        if (!buffer.create(vertices.size()) || !buffer.update(vertices.data()))
            throw std::bad_alloc();
    }

    bool Contour::contains(const sf::Vector2f &_position) const {

        for (size_t i = 0; i < vertices.size() / 3; ++i) {

            const sf::Vector2f a = vertices[3 * i + 0].position;
            const sf::Vector2f b = vertices[3 * i + 1].position;
            const sf::Vector2f c = vertices[3 * i + 2].position;

            if (simplexContains(a, b, c, _position)) return true;
        }

        return false;
    }

    void Contour::draw(sf::RenderTarget &_target, sf::RenderStates _states) const {

        _states.texture = &texture;
        _target.draw(buffer, _states);
    }


    // Element

    auto Element::elements = std::map<std::string, Element *>();

    bool Element::handle(const std::optional<sf::Event> &_event, sf::Transform _global) {

        if (!active) return false;

        _global *= getTransform();

        std::function<bool(void)> thisFocused = []() { return false; };

        if (const auto *move = _event->getIf<sf::Event::MouseMoved>()) {
            thisFocused = [&]() {
                const sf::Vector2f local = _global.getInverse().transformPoint({
                    static_cast<float>(move->position.x), 
                    static_cast<float>(move->position.y)
                });
                return mouseMoved(local);
            };
        }
        else if (const auto *scroll = _event->getIf<sf::Event::MouseWheelScrolled>()) {
            thisFocused = [&]() {
                const sf::Vector2f local = _global.getInverse().transformPoint({
                    static_cast<float>(scroll->position.x),
                    static_cast<float>(scroll->position.y)
                });
                return mouseScrolled(local, scroll->delta);
            };
        }
        else if (const auto *press = _event->getIf<sf::Event::MouseButtonPressed>()) {
            thisFocused = [&]() {
                const sf::Vector2f local = _global.getInverse().transformPoint({
                    static_cast<float>(press->position.x),
                    static_cast<float>(press->position.y)
                });
                return mousePressed(local, press->button);
            };
        }
        else if (const auto *release = _event->getIf<sf::Event::MouseButtonReleased>()) {
            thisFocused = [&]() {
                const sf::Vector2f local = _global.getInverse().transformPoint({
                    static_cast<float>(release->position.x),
                    static_cast<float>(release->position.y)
                });
                return mouseReleased(local, release->button);
            };
        }
        else if (const auto *press = _event->getIf<sf::Event::KeyPressed>())
            thisFocused = [&]() {
                return keyPressed(press->code, press->control, press->alt, press->shift);
            };
        else if (const auto *release = _event->getIf<sf::Event::KeyReleased>())
            thisFocused = [&]() {
                return keyReleased(press->code, press->control, press->alt, press->shift);
            };

        bool subtreeFocused = false;

        for (auto &child : children)
            subtreeFocused = subtreeFocused || child->handle(_event, _global);

        return subtreeFocused || thisFocused();
    }

    void Element::draw(sf::RenderTarget &_target, sf::RenderStates _states) const {

        if (!active) return;

        _states.transform *= getTransform();
        Contour::draw(_target, _states);

        for (auto &child : children)
            child->draw(_target, _states);
    }


    // Button

    bool Button::mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) {

        if (contains(_pos)) {
            if (_btn == sf::Mouse::Button::Left) leftPressed = true;
            if (_btn == sf::Mouse::Button::Right) rightPressed = true;
            return true;
        }
        else {
            leftPressed = rightPressed = false;
            return false;
        }
    }

    bool Button::mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) {

        if (contains(_pos)) {
            if (_btn == sf::Mouse::Button::Left && leftPressed) leftClick();
            if (_btn == sf::Mouse::Button::Right && rightPressed) rightClick();
            leftPressed = rightPressed = false;
            return true;
        }
        else {
            leftPressed = rightPressed = false;
            return false;
        }
    }

    // Draggable

    bool Draggable::mouseMoved(const sf::Vector2f &_pos) {

        if (dragged) {
            move(_pos - dragPoint);
            return true;
        }
        return false;
    }

    bool Draggable::mousePressed(const sf::Vector2f &_pos, sf::Mouse::Button _btn) {

        if (contains(_pos) && _btn == sf::Mouse::Button::Left) {
            dragged = true;
            dragPoint = _pos;
            return true;
        }
        return false;
    }

    bool Draggable::mouseReleased(const sf::Vector2f &_pos, sf::Mouse::Button _btn) {

        if (contains(_pos) && _btn == sf::Mouse::Button::Left && dragged) {
            dragged = false;
            return true;
        }
        return false;
    }
}
