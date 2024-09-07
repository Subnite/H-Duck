#pragma once
#include <JuceHeader.h>

namespace duck::utils {

template <typename T>
void clampToBounds(juce::Point<T>& point, const juce::Rectangle<T>& bounds) {
    if (point.x > bounds.getRight()) point.x = bounds.getRight();
    else if (point.x < 0) point.x = 0;

    if (point.y > bounds.getHeight()) point.y = bounds.getHeight();
    else if (point.y < 0) point.y = 0;
}





} // namespace
