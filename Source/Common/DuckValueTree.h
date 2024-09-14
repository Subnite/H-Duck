#pragma once
#include <JuceHeader.h>
#include "ValueTreeManager.h"

enum class Property {
    // trees
    T_ROOT, T_CURVE_DATA, T_NORMALIZED_POINTS, T_POINT,
    T_LENGTH_MS,
    T_LOOKAHEAD_MS,
    
    // properties
    P_POWER, P_MAX_ABSOLUTE_POWER, P_SIZE, P_X, P_Y,
    P_IS_MS, P_RAW_NORMALIZED_VALUE, P_DISPLAY_VALUE, P_MIN_VALUE, P_MAX_VALUE,
    
    COUNT
};

namespace duck::vt {

class ValueTree : public subnite::vt::ValueTreeBase, public subnite::vt::IDMap<Property> {
public:
    ValueTree()
    : subnite::vt::ValueTreeBase(), subnite::vt::IDMap<Property>()
    {
        setupMap();
    };
    ~ValueTree(){};

    void setupMap() override {
        using p = Property;
        using id = juce::Identifier;

        #pragma region trees
        map[p::T_ROOT] = id{"HentaiDuckRoot"};

        // curve display trees
        map[p::T_CURVE_DATA] = id{"CurveData"};
        map[p::T_NORMALIZED_POINTS] = id{"NormalizedPoints"};
        map[p::T_POINT] = id{"Point"};

        // length slider trees
        map[p::T_LENGTH_MS] = id{"LengthMS"};

        // lookahead slider trees
        map[p::T_LOOKAHEAD_MS] = id{"LookaheadMS"};

        #pragma endregion trees

        #pragma region properties
        // curve display properties
        map[p::P_POWER] = id{"power"};
        map[p::P_MAX_ABSOLUTE_POWER] = id{"maxAbsolutePower"};
        map[p::P_SIZE] = id{"size"};
        map[p::P_X] = id{"x"};
        map[p::P_Y] = id{"y"};

        // length slider properties
        map[p::P_IS_MS] = id{"isMS"};
        map[p::P_RAW_NORMALIZED_VALUE] = id{"rawNormalizedValue"};
        map[p::P_DISPLAY_VALUE] = id{"displayValue"};
        map[p::P_MIN_VALUE] = id{"minValue"};
        map[p::P_MAX_VALUE] = id{"maxValue"};

        #pragma endregion properties
    }

    // makes the default tree
    void create() override {
        using prop = Property;
        using id = juce::Identifier;

        // create new one
        vtRoot = juce::ValueTree{getIDFromType(prop::T_ROOT).value_or(id{"undefined"})};
        juce::ValueTree curve{getIDFromType(prop::T_CURVE_DATA).value_or(id{"undefined"})};
        juce::ValueTree points{getIDFromType(prop::T_NORMALIZED_POINTS).value_or(id{"undefined"})};
        curve.appendChild(points, &undoManager);
        vtRoot.appendChild(curve, &undoManager);

        addPoint({0.f, 0.f}, -10.f, 50.f, 20.f);
        addPoint({0.002f, 1.f}, 8.f, 50.f, 20.f);
        addPoint({0.3f, 0.3f}, -8.f, 50.f, 20.f);
        addPoint({0.5f, 0.f}, 0.f, 50.f, 20.f);
        addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);

        // right now the length slider tree is created by the slider class, changing this would also work, but then add all possible values.
        juce::ValueTree lengthSliderTree{getIDFromType(prop::T_LENGTH_MS).value_or(id{"undefined"})};
        lengthSliderTree.setProperty(getIDFromType(prop::P_IS_MS).value_or(id{"undefined"}), true, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::P_DISPLAY_VALUE).value_or(id{"undefined"}), 50.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::P_MIN_VALUE).value_or(id{"undefined"}), 10.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::P_MAX_VALUE).value_or(id{"undefined"}), 2000.0, nullptr);
        lengthSliderTree.setProperty(getIDFromType(prop::P_RAW_NORMALIZED_VALUE).value_or(id{"undefined"}), 0.5, nullptr);

        vtRoot.appendChild(lengthSliderTree, &undoManager);
    }

    void addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size) {
        using prop = Property;
        using id = juce::Identifier;

        auto curve = vtRoot.getOrCreateChildWithName(getIDFromType(prop::T_CURVE_DATA).value_or(id{"undefined"}), &undoManager);
        auto points = curve.getOrCreateChildWithName(getIDFromType(prop::T_NORMALIZED_POINTS).value_or(id{"undefined"}), &undoManager);
        if (!points.isValid()) return;
        juce::ValueTree point{getIDFromType(prop::T_POINT).value_or(id{"undefined"})};

        point.setProperty(getIDFromType(prop::P_X).value_or(id{"undefined"}), {double(coords.x)}, &undoManager);
        point.setProperty(getIDFromType(prop::P_Y).value_or(id{"undefined"}), {double(coords.y)}, &undoManager);
        point.setProperty(getIDFromType(prop::P_POWER).value_or(id{"undefined"}), {double(power)}, &undoManager);
        point.setProperty(getIDFromType(prop::P_MAX_ABSOLUTE_POWER).value_or(id{"undefined"}), {double(maxAbsPower)}, &undoManager);
        point.setProperty(getIDFromType(prop::P_SIZE).value_or(id{"undefined"}), {double(size)}, &undoManager);

        points.appendChild(point, &undoManager);
    };
};



} // namespace
