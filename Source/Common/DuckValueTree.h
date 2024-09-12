#pragma once
#include <JuceHeader.h>
#include "ValueTreeManager.h"

enum class Property {
    // trees
    ROOT, CD_CURVE_DATA, CD_NORMALIZED_POINTS, CD_POINT, CD_COORDS,
    LS_LENGTH_MS,
    
    // properties
    CD_POWER, CD_MAX_ABSOLUTE_POWER, CD_SIZE, CD_X, CD_Y,
    LS_IS_MS, LS_RAW_NORMALIZED_VALUE, LS_DISPLAY_VALUE, LS_MIN_VALUE, LS_MAX_VALUE,
    
    COUNT
};

namespace duck::vt {

class ValueTree : public subnite::vt::ValueTree<Property> {
public:
    ValueTree()
    : subnite::vt::ValueTree<Property>()
    {
        setupMap();
    };
    ~ValueTree(){};

    void setupMap() override {
        using p = Property;
        using id = juce::Identifier;

        #pragma region trees
        map[p::ROOT] = id{"HentaiDuckRoot"};

        // curve display trees
        map[p::CD_CURVE_DATA] = id{"CD_CurveData"};
        map[p::CD_NORMALIZED_POINTS] = id{"CD_NormalizedPoints"};
        map[p::CD_POINT] = id{"CD_Points"};
        map[p::CD_COORDS] = id{"CD_Coords"};

        // length slider trees
        map[p::LS_LENGTH_MS] = id{"LS_LengthMS"};

        #pragma endregion trees

        #pragma region properties
        // curve display properties
        map[p::CD_POWER] = id{"CD_Power"};
        map[p::CD_MAX_ABSOLUTE_POWER] = id{"CD_MaxAbsolutePower"};
        map[p::CD_SIZE] = id{"CD_Size"};
        map[p::CD_X] = id{"CD_X"};
        map[p::CD_Y] = id{"CD_Y"};

        // length slider properties
        map[p::LS_IS_MS] = id{"LS_isMS"};
        map[p::LS_RAW_NORMALIZED_VALUE] = id{"LS_RawNormalizedValue"};
        map[p::LS_DISPLAY_VALUE] = id{"LS_DisplayValue"};
        map[p::LS_MIN_VALUE] = id{"LS_MinValue"};
        map[p::LS_MAX_VALUE] = id{"LS_MaxValue"};

        #pragma endregion properties
    }

    // makes the default tree
    void create() override {
        using vt = subnite::vt::ValueTree<Property>;
        using prop = Property;
        using id = juce::Identifier;

        // create new one
        vtRoot = juce::ValueTree{getIDFromType(prop::ROOT).value_or(id{"undefined"})};
        juce::ValueTree curve{getIDFromType(prop::CD_CURVE_DATA).value_or(id{"undefined"})};
        juce::ValueTree points{getIDFromType(prop::CD_NORMALIZED_POINTS).value_or(id{"undefined"})};
        curve.appendChild(points, &undoManager);
        vtRoot.appendChild(curve, &undoManager);

        addPoint({0.f, 0.f}, -10.f, 50.f, 20.f);
        addPoint({0.002f, 1.f}, 8.f, 50.f, 20.f);
        addPoint({0.3f, 0.3f}, -8.f, 50.f, 20.f);
        addPoint({0.5f, 0.f}, 0.f, 50.f, 20.f);
        addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);

        // right now the length slider tree is created by the slider class, changing this would also work, but then add all possible values.
        juce::ValueTree lengthSliderTree{getIDFromType(prop::LS_LENGTH_MS).value_or(id{"undefined"})};
        lengthSliderTree.setProperty(getIDFromType(prop::LS_IS_MS).value_or(id{"undefined"}), true, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::LS_DISPLAY_VALUE).value_or(id{"undefined"}), 50.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::LS_MIN_VALUE).value_or(id{"undefined"}), 10.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::LS_MAX_VALUE).value_or(id{"undefined"}), 2000.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::LS_RAW_NORMALIZED_VALUE).value_or(id{"undefined"}), 0.5, nullptr);

        vtRoot.appendChild(lengthSliderTree, &undoManager);
    }

    void addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size) {
        using prop = Property;
        using vt = subnite::vt::ValueTree<prop>;
        using id = juce::Identifier;

        auto curve = vtRoot.getOrCreateChildWithName(getIDFromType(prop::CD_CURVE_DATA).value_or(id{"undefined"}), &undoManager);
        auto points = curve.getOrCreateChildWithName(getIDFromType(prop::CD_NORMALIZED_POINTS).value_or(id{"undefined"}), &undoManager);
        if (!points.isValid()) return;
        juce::ValueTree point{getIDFromType(prop::CD_POINT).value_or(id{"undefined"})};
        juce::ValueTree xy{getIDFromType(prop::CD_COORDS).value_or(id{"undefined"})};

        xy.setProperty(getIDFromType(prop::CD_X).value_or(id{"undefined"}), {double(coords.x)}, &undoManager);
        xy.setProperty(getIDFromType(prop::CD_Y).value_or(id{"undefined"}), {double(coords.y)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_POWER).value_or(id{"undefined"}), {double(power)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_MAX_ABSOLUTE_POWER).value_or(id{"undefined"}), {double(maxAbsPower)}, &undoManager);
        point.setProperty(getIDFromType(prop::CD_SIZE).value_or(id{"undefined"}), {double(size)}, &undoManager);

        point.appendChild(xy, &undoManager);
        points.appendChild(point, &undoManager);
    };
};



} // namespace
