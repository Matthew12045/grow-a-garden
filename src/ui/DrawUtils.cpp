#include "DrawUtils.h"

namespace DrawUtils {

void drawPxPanel(sf::RenderWindow& window, sf::Font&, sf::Vector2f pos, sf::Vector2f sz,
                 sf::Color mid, sf::Color dark, sf::Color lite) {
    sf::RectangleShape outer(sz + sf::Vector2f{6.f, 6.f});
    outer.setPosition(pos - sf::Vector2f{3.f, 3.f});
    outer.setFillColor(dark);
    window.draw(outer);

    sf::RectangleShape body(sz);
    body.setPosition(pos);
    body.setFillColor(mid);
    window.draw(body);

    sf::RectangleShape hl({sz.x, 4.f});
    hl.setPosition(pos);
    hl.setFillColor(lite);
    window.draw(hl);
    sf::RectangleShape vl({4.f, sz.y});
    vl.setPosition(pos);
    vl.setFillColor(lite);
    window.draw(vl);

    sf::RectangleShape sb({sz.x, 4.f});
    sb.setPosition({pos.x, pos.y + sz.y - 4.f});
    sb.setFillColor(dark);
    window.draw(sb);
    sf::RectangleShape sr({4.f, sz.y});
    sr.setPosition({pos.x + sz.x - 4.f, pos.y});
    sr.setFillColor(dark);
    window.draw(sr);
}

void drawPxButton(sf::RenderWindow& window, sf::Font& font, sf::FloatRect b, const std::string& label,
                  sf::Color col, sf::Color hover,
                  sf::Vector2f mouse, unsigned fontSize) {
    bool h = b.contains(mouse);
    sf::RectangleShape bg(b.size);
    bg.setPosition(b.position);
    bg.setFillColor(h ? hover : col);
    window.draw(bg);

    sf::RectangleShape hl({b.size.x, 2.f});
    hl.setPosition(b.position);
    hl.setFillColor({255, 255, 255, 60});
    window.draw(hl);

    auto t = makeText(font, label, fontSize, Pal::CREAM);
    auto lb = t.getLocalBounds();
    t.setPosition({b.position.x + (b.size.x - lb.size.x) / 2.f,
                   b.position.y + (b.size.y - lb.size.y) / 2.f - 2.f});
    window.draw(t);
}

sf::Text makeText(const sf::Font& font, const std::string& s, unsigned sz, sf::Color col) {
    sf::Text t(font, s, sz);
    t.setFillColor(col);
    return t;
}

void centreText(sf::Text& t, sf::FloatRect area) {
    auto lb = t.getLocalBounds();
    t.setPosition({area.position.x + (area.size.x - lb.size.x) / 2.f,
                   area.position.y + (area.size.y - lb.size.y) / 2.f - 2.f});
}

}
