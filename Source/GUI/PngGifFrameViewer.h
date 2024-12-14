#pragma once
#include <JuceHeader.h>
#include <string>

namespace duck {

/**
 * Will go from top left to bottom right.
*/
class PngGifFrameViewer {
public:
    PngGifFrameViewer(const File& file, size_t rows, size_t columns, size_t totalAmount)
    : rows(rows), columns(columns), totalAmount(totalAmount)
    {
        if (rows < 1 || columns < 1 || totalAmount < 1) jassertfalse;
        loadedImage = ImageFileFormat::loadFrom(file);
        jassert(!loadedImage.isNull());
    }

    const Image getFrame(size_t frameIndex) {
        const size_t row = frameIndex / columns;
        const size_t column = frameIndex % columns;
        auto dims = getFrameDimensions();
        dims.setPosition(column * dims.getWidth(), row * dims.getHeight());
        return loadedImage.getClippedImage(dims);
    }

    Rectangle<int> getFrameDimensions()
    {
        const int width = loadedImage.getWidth() / columns;
        const int height = loadedImage.getHeight() / rows;
        return {0, 0, width, height};
    }

private:
    size_t rows, columns, totalAmount;
    Image loadedImage;
};

}