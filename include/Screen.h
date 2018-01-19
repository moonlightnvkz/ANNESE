#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace ANNESE {
    struct Pixel {
        int x;

        int y;
    };

    class Screen : public sf::Drawable {
    public:
        Screen(sf::Vector2i screenSize, float pixelScale, sf::Color filling = sf::Color::White);

        virtual ~Screen() = default;

        void setPixel(Pixel pixel, sf::Color color);

    protected:
        void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        sf::Vector2i mScreenSize;

        float mPixelScale;

        sf::VertexArray mVertices;
    };
}