#include <cstring>
#include <cassert>
#include <algorithm>
#include "../include/PPU.h"
#include "../include/PaletteColors.h"

namespace ANNESE {
    PPU::PPU(std::shared_ptr<ANNESE::PictureBus> pictureBus, std::shared_ptr<ANNESE::Screen> screen)
            : mPictureBus(std::move(pictureBus)), mScreen(std::move(screen)), mSpriteMemory(64 * 4),
              mPictureBuffer(ScanlineVisibleDots, std::vector<sf::Color>(VisibleScanlines, sf::Color::Magenta)) {
    }

    void PPU::reset() {
        mLongSprites = mGenerateInterrupt = mVBlank = false;
        mShowBackground = mShowSprites = mEvenFrame = mFirstWrite = true;
        mBgPage = mSprPage = CharacterPage::Low;
        mDataAddress = mTempAddress = 0;
        mCycle = mScanline = mSpriteDataAddress = mFineXScroll = 0;
        mDataAddrIncrement = 1;
        mPipelineState = State::PreRender;
        mScanlineSprites.reserve(8);
    }

    void PPU::doDMA(const Byte *page) {
        assert(mSpriteDataAddress <= 256);
        std::memcpy(mSpriteMemory.data() + mSpriteDataAddress, page, static_cast<size_t>(256 - mSpriteDataAddress));
        if (mSpriteDataAddress) {
            std::memcpy(mSpriteMemory.data(), page + (256 - mSpriteDataAddress), mSpriteDataAddress);
        }
    }

    void PPU::control(Byte ctrl) {
        mGenerateInterrupt = (ctrl & 0x80) != 0;
        mLongSprites = (ctrl & 0x20) != 0;
        mBgPage = static_cast<CharacterPage>(((ctrl & 0x10) != 0));
        mSprPage = static_cast<CharacterPage>(((ctrl & 0x8) != 0));
        if (ctrl & 0x4) {
            mDataAddrIncrement = 0x20;
        } else {
            mDataAddrIncrement = 1;
        }
        mTempAddress &= ~0xc00;
        mTempAddress |= (ctrl & 0x3) << 10;
    }

    void PPU::mask(Byte mask) {
        mHideEdgeBackground = !(mask & 0x2);
        mHideEdgeSprites = !(mask & 0x4);
        mShowSprites = (mask & 0x10) != 0;
        mShowBackground = (mask & 0x8) != 0;
    }

    Byte PPU::status() {
        Byte status = static_cast<Byte>(mSprZeroHit) << 6 |
                static_cast<Byte>(mVBlank) << 7;
        mVBlank = false;
        mFirstWrite = true;
        return status;
    }

    void PPU::dataAddress(Byte addr) {
        if (mFirstWrite) {
            mTempAddress &= ~0xff00; //Unset the upper byte
            mTempAddress |= (addr & 0x3f) << 8;
            mFirstWrite = false;
        } else {
            mTempAddress &= ~0xff; //Unset the lower byte;
            mTempAddress |= addr;
            mDataAddress = mTempAddress;
            mFirstWrite = true;
        }
    }

    Byte PPU::data() {
        Byte data = mPictureBus->read(mDataAddress);
        mDataAddress += mDataAddrIncrement;

        //Reads are delayed by one byte/read when address is in this range
        if (mDataAddress < 0x3f00) {
            //Return from the data buffer and store the current value in the buffer
            std::swap(data, mDataBuffer);
        }

        return data;
    }

    void PPU::data(Byte data) {
        mPictureBus->write(mDataAddress, data);
        mDataAddress += mDataAddrIncrement;
    }

    void PPU::scroll(Byte scroll) {
        if (mFirstWrite) {
            mTempAddress &= ~0x1f;
            mTempAddress |= (scroll >> 3) & 0x1f;
            mFineXScroll = scroll & Byte(0x7);
            mFirstWrite = false;
        } else {
            mTempAddress &= ~0x73e0;
            mTempAddress |= ((scroll & 0x7) << 12) |
                             ((scroll & 0xf8) << 2);
            mFirstWrite = true;
        }
    }

    void PPU::step() {
        switch (mPipelineState) {
            case State::PreRender:
                if (mCycle == 1) {
                    mVBlank = mSprZeroHit = false;
                } else if (mCycle == ScanlineVisibleDots + 2 && mShowBackground && mShowSprites) {
                    //Set bits related to horizontal position
                    mDataAddress &= ~0x41f; //Unset horizontal bits
                    mDataAddress |= mTempAddress & 0x41f; //Copy
                } else if (mCycle > 280 && mCycle <= 304 && mShowBackground && mShowSprites) {
                    //Set vertical bits
                    mDataAddress &= ~0x7be0; //Unset bits related to horizontal
                    mDataAddress |= mTempAddress & 0x7be0; //Copy
                }
                //if rendering is on, every other frame is one cycle shorter
                if (mCycle >= ScanlineEndCycle - (!mEvenFrame && mShowBackground && mShowSprites)) {
                    mPipelineState = State::Render;
                    mCycle = mScanline = 0;
                }
                break;
            case State::Render:
                if (mCycle > 0 && mCycle <= ScanlineVisibleDots) {
                    Byte bgColor = 0, sprColor = 0;
                    bool bgOpaque = false, sprOpaque = true;
                    bool sprFg = false;

                    int x = mCycle - 1;
                    int y = mScanline;

                    if (mShowBackground) {
                        Byte xFine = (mFineXScroll + x) % Byte(8);
                        if (!mHideEdgeBackground || x >= 8) {
                            //fetch tile
                            Address addr = Address(0x2000) | (mDataAddress & Address(0x0FFF)); //mask off fine y
                            Byte tile = read(addr);

                            //fetch pattern
                            //Each pattern occupies 16 bytes, so multiply by 16
                            addr = (tile * 16) + ((mDataAddress >> 12) & Address(0x7)); //Add fine y
                            //set whether the pattern is in the high or low page
                            addr |= static_cast<Address>(mBgPage) << 12;
                            //Get the corresponding bit determined by (8 - x_fine) from the right
                            bgColor = (read(addr) >> (7 ^ xFine)) & Byte(1); //bit 0 of palette entry
                            bgColor |= ((read(addr + Address(8)) >> (7 ^ xFine)) & Byte(1)) << 1; //bit 1

                            bgOpaque = bgColor; //flag used to calculate final pixel with the sprite pixel

                            //fetch attribute and calculate higher two bits of palette
                            addr = Address(0x23C0) | (mDataAddress & 0x0C00) | ((mDataAddress >> 4) & 0x38)
                                   | ((mDataAddress >> 2) & 0x07);
                            auto attribute = read(addr);
                            int shift = ((mDataAddress >> 4) & 4) | (mDataAddress & 2);
                            //Extract and set the upper two bits for the color
                            bgColor |= ((attribute >> shift) & 0x3) << 2;
                        }
                        //Increment/wrap coarse X
                        if (xFine == 7) {
                            if ((mDataAddress & 0x001F) == 31) {    // if coarse X == 31
                                mDataAddress &= ~0x001F;    // coarse X = 0
                                mDataAddress ^= 0x0400;     // switch horizontal nametable
                            } else {
                                mDataAddress += 1;  // increment coarse X
                            }
                        }
                    }

                    if (mShowSprites && (!mHideEdgeSprites || x >= 8)) {
                        for (Byte i : mScanlineSprites) {
                            Byte sprX = mSpriteMemory[i * 4 + 3];

                            if (x - sprX < 0 || x - sprX >= 8) {
                                continue;
                            }

                            Byte sprY      = mSpriteMemory[i * 4 + 0] + Byte(1);
                            Byte tile      = mSpriteMemory[i * 4 + 1];
                            Byte attribute = mSpriteMemory[i * 4 + 2];

                            int length = (mLongSprites) ? 16 : 8;

                            int xShift = (x - sprX) % 8;
                            int yOffset = (y - sprY) % length;

                            //If NOT flipping horizontally
                            if ((attribute & 0x40) == 0) {
                                xShift ^= 7;
                            }
                            //IF flipping vertically
                            if ((attribute & 0x80) != 0) {
                                yOffset ^= (length - 1);
                            }
                            Address addr = 0;

                            if (!mLongSprites) {
                                addr = tile * Address(16) + yOffset;
                                if (mSprPage == CharacterPage::High) {
                                    addr += 0x1000;
                                }
                            } else {    //8x16 sprites
                                //bit-3 is one if it is the bottom tile of the sprite,
                                // multiply by two to get the next pattern
                                yOffset = (yOffset & 7) | ((yOffset & 8) << 1);
                                addr = (tile >> 1) * Address(32) + yOffset;
                                addr |= (tile & 1) << 12; //Bank 0x1000 if bit-0 is high
                            }

                            sprColor |= (read(addr) >> (xShift)) & 1; //bit 0 of palette entry
                            sprColor |= ((read(addr + Address(8)) >> (xShift)) & 1) << 1; //bit 1

                            sprOpaque = sprColor != 0;
                            if (!sprOpaque) {
                                continue;
                            }

                            sprColor |= 0x10; //Select sprite palette
                            sprColor |= (attribute & 0x3) << 2; //bits 2-3

                            sprFg = !(attribute & 0x20);

                            //Sprite-0 hit detection
                            if (!mSprZeroHit && mShowBackground && i == 0 && bgOpaque) {
                                mSprZeroHit = true;
                            }
                            break; //Exit the loop now since we've found the highest priority sprite
                        }
                    }

                    Byte paletteAddr = bgColor;

                    if ( (!bgOpaque && sprOpaque) ||
                         (bgOpaque && sprOpaque && sprFg) ) {
                        paletteAddr = sprColor;
                    } else if (!bgOpaque) {
                        paletteAddr = 0;
                    }

                    mPictureBuffer[x][y] = sf::Color(PaletteColors[mPictureBus->readPalette(paletteAddr)]);
                } else if (mCycle == ScanlineVisibleDots + 1 && mShowBackground) {
                    //Shamelessly copied from nesdev wiki
                    if ((mDataAddress & 0x7000) != 0x7000) {  // if fine Y < 7
                        mDataAddress += 0x1000;     // increment fine Y
                    } else {
                        mDataAddress &= ~0x7000;    // fine Y = 0
                        int y = (mDataAddress & 0x03E0) >> 5;   // let y = coarse Y
                        if (y == 29) {
                            y = 0;  // coarse Y = 0
                            mDataAddress ^= 0x0800; // switch vertical nametable
                        } else if (y == 31) {
                            y = 0;  // coarse Y = 0, nametable not switched
                        } else {
                            y += 1; // increment coarse Y
                        }
                        // put coarse Y back into m_dataAddress
                        mDataAddress = (mDataAddress & Address(~0x03E0)) | (y << 5);
                    }
                } else if (mCycle == ScanlineVisibleDots + 2 && mShowBackground && mShowSprites) {
                    //Copy bits related to horizontal position
                    mDataAddress &= ~0x41f;
                    mDataAddress |= mTempAddress & 0x41f;
                }

                if (mCycle >= ScanlineEndCycle) {
                    //Find and index sprites that are on the next Scanline
                    //This isn't where/when this indexing, actually copying in 2C02 is done
                    //but (I think) it shouldn't hurt any games if this is done here

                    mScanlineSprites.resize(0);
                    int range = mLongSprites ? 16 : 8;

                    for (Byte i = mSpriteDataAddress / Byte(4), j = 0; i < 64; ++i) {
                        int diff = (mScanline - mSpriteMemory[i * 4]);
                        if (diff >= 0 && diff < range) {
                            mScanlineSprites.push_back(i);
                            ++j;
                            if (j >= 8) {
                                break;
                            }
                        }
                    }
                    ++mScanline;
                    mCycle = 0;
                }
                if (mScanline >= VisibleScanlines) {
                    mPipelineState = State::PostRender;
                }
                break;
            case State::PostRender:
                if (mCycle >= ScanlineEndCycle) {
                    ++mScanline;
                    mCycle = 0;
                    mPipelineState = State::VerticalBlank;
                    for (int x = 0; x < ScanlineVisibleDots; ++x) {
                        for (int y = 0; y < VisibleScanlines; ++y) {
                            mScreen->setPixel({x, y}, mPictureBuffer[x][y]);
                        }
                    }
                }
                break;
            case State::VerticalBlank:
                if (mCycle == 1 && mScanline == VisibleScanlines + 1) {
                    mVBlank = true;
                    if (mGenerateInterrupt) {
                        assert(mVBlankCallback);
                        mVBlankCallback();
                    }
                }
                if (mCycle >= ScanlineEndCycle) {
                    ++mScanline;
                    mCycle = 0;
                }
                if (mScanline >= FrameEndScanline) {
                    mPipelineState = State::PreRender;
                    mScanline = 0;
                    mEvenFrame = !mEvenFrame;
                }
        }
        ++mCycle;
    }
}