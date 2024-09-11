#pragma once
#include <JuceHeader.h>
#include "ValueTreeManager.h"

// edit enums in ValueTreeManager.cpp and .h

namespace duck::vt {
class ValueTree : public subnite::vt::ValueTree {
public:
    ValueTree(){};
    ~ValueTree(){};

    // makes the default tree
    void create() override {
        using vt = subnite::vt::ValueTree;
        using tree = subnite::vt::Tree;
        using prop = subnite::vt::Property;

        // create new one
        vtRoot = juce::ValueTree{getIDFromType(tree::ROOT)};
        juce::ValueTree curve{getIDFromType(tree::CD_CURVE_DATA)};
        juce::ValueTree points{getIDFromType(tree::CD_NORMALIZED_POINTS)};
        curve.appendChild(points, &undoManager);
        vtRoot.appendChild(curve, &undoManager);

        addPoint({0.f, 0.f}, -10.f, 50.f, 20.f);
        addPoint({0.002f, 1.f}, 8.f, 50.f, 20.f);
        addPoint({0.3f, 0.3f}, -8.f, 50.f, 20.f);
        addPoint({0.5f, 0.f}, 0.f, 50.f, 20.f);
        addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);
        
        // right now the length slider tree is created by the slider class, changing this would also work, but then add all possible values.
        juce::ValueTree lengthSliderTree{getIDFromType(tree::LS_LENGTH_MS)};
        lengthSliderTree.setProperty(subnite::vt::getIDFromType(prop::LS_IS_MS), true, nullptr);
        lengthSliderTree.setProperty(subnite::vt::getIDFromType(prop::LS_DISPLAY_VALUE), 50.0, nullptr);
        lengthSliderTree.setProperty(subnite::vt::getIDFromType(prop::LS_MIN_VALUE), 10.0, nullptr);
        lengthSliderTree.setProperty(subnite::vt::getIDFromType(prop::LS_MAX_VALUE), 2000.0, nullptr);
        lengthSliderTree.setProperty(subnite::vt::getIDFromType(prop::LS_RAW_NORMALIZED_VALUE), 0.5, nullptr);

        vtRoot.appendChild(lengthSliderTree, &undoManager);
    }

    void addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size) {
        using vt = subnite::vt::ValueTree;
        using tree = subnite::vt::Tree;
        using prop = subnite::vt::Property;

        auto curve = vtRoot.getOrCreateChildWithName(getIDFromType(tree::CD_CURVE_DATA), &undoManager);
        auto points = curve.getOrCreateChildWithName(getIDFromType(tree::CD_NORMALIZED_POINTS), &undoManager);
        if (!points.isValid()) return;
        juce::ValueTree point{getIDFromType(tree::CD_POINT)};
        juce::ValueTree xy{getIDFromType(tree::CD_COORDS)};

        xy.setProperty(getIDFromType(prop::CD_X), {double(coords.x)}, &undoManager);
        xy.setProperty(getIDFromType(prop::CD_Y), {double(coords.y)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_POWER), {double(power)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_MAX_ABSOLUTE_POWER), {double(maxAbsPower)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_SIZE), {double(size)}, &undoManager);

        point.appendChild(xy, &undoManager);
        points.appendChild(point, &undoManager);
    };
};



} // namespace