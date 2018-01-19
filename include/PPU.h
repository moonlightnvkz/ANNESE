#pragma once

#include <memory>
#include <functional>
#include "PictureBus.h"
#include "Screen.h"

namespace ANNESE {
    class PPU {
    public:
        static constexpr const unsigned VisibleScanlines = 240;

        static constexpr const unsigned ScanlineVisibleDots = 256;

        PPU(std::shared_ptr<PictureBus> pictureBus, std::shared_ptr<Screen> screen);

        virtual ~PPU() = default;

        void step();

        void reset();

        void setInterruptCallback(std::function<void(void)> cb) {
            mVBlankCallback = cb;
        }

        void doDMA(const Byte *page);

        void control(Byte ctrl);

        void mask(Byte mask);

        void OAMAddress(Byte addr) {
            mSpriteDataAddress = addr;
        }

        void dataAddress(Byte addr);

        void scroll(Byte scroll);

        void data(Byte data);

        Byte status();

        Byte data();

        Byte OAMData() {
            return mSpriteMemory.at(mSpriteDataAddress);
        }

        void OAMData(Byte value) {
            mSpriteMemory.at(mSpriteDataAddress++) = value;
        }


    protected:
        Byte read(Address addr) {
            return mPictureBus->read(addr);
        }

        static constexpr const int ScanlineEndCycle = 340;

        static constexpr const int FrameEndScanline = 261;

        std::shared_ptr<PictureBus> mPictureBus;

        std::shared_ptr<Screen> mScreen;

        std::function<void(void)> mVBlankCallback;

        std::vector<Byte> mSpriteMemory;

        std::vector<Byte> mScanlineSprites;

        enum class State {
            PreRender,
            Render,
            PostRender,
            VerticalBlank,
        };

        State mPipelineState;

        int mCycle;

        int mScanline;

        bool mEvenFrame;

        bool mVBlank;

        bool mSprZeroHit;

        Address mDataAddress;

        Address mTempAddress;

        Byte mFineXScroll;

        bool mFirstWrite;

        Byte mDataBuffer;

        Byte mSpriteDataAddress;

        bool mLongSprites;

        bool mGenerateInterrupt;

        bool mShowSprites;

        bool mShowBackground;

        bool mHideEdgeSprites;

        bool mHideEdgeBackground;

        enum class CharacterPage {
            Low,
            High,
        };

        CharacterPage mBgPage;

        CharacterPage mSprPage;

        Address mDataAddrIncrement;

        std::vector<std::vector<sf::Color>> mPictureBuffer;
    };
}