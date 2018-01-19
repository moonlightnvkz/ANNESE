#include <cassert>
#include <SFML/Graphics/RenderTarget.hpp>
#include "../include/Screen.h"

namespace ANNESE {
    Screen::Screen(sf::Vector2i screenSize, float pixelScale, sf::Color filling)
            : mScreenSize(screenSize), mPixelScale(pixelScale) {
        assert(mScreenSize.x > 0 && mScreenSize.y > 0);
        mVertices.resize(static_cast<size_t>(screenSize.x * screenSize.y * 6));  // 2 triangles
        mVertices.setPrimitiveType(sf::Triangles);
        for (int x = 0; x < mScreenSize.x; ++x) {
            for (int y = 0; y < mScreenSize.y; ++y) {
                size_t index = static_cast<size_t>((x * mScreenSize.y + y) * 6);
                sf::Vector2f coords(x * mPixelScale, y * mPixelScale);

                for (size_t i = 0; i < 6; ++i) {
                    mVertices[index + i].position = coords +
                                                    sf::Vector2f{mPixelScale * (i >= 1 && i <= 3 ? 1 : 0),
                                                                 mPixelScale * (i >= 2 && i <= 4 ? 1 : 0)};
                    mVertices[index + i].color = filling;
                }
            }
        }
    }

    void Screen::setPixel(Pixel pixel, sf::Color color) {
        size_t index = static_cast<size_t>((pixel.x * mScreenSize.y + pixel.y) * 6);
        if (index >= mVertices.getVertexCount()) {
            return;
        }

        for (size_t i = 0; i < 6; ++i) {
            mVertices[index + i].color = color;
        }
    }

    void Screen::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(mVertices, states);
    }
}