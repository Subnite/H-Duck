#pragma once

#include <JuceHeader.h>
#include <vector>
#include "DuckValueTree.h"

namespace duck::curve {

template <typename T>
struct Point {
    juce::Point<T> coords;
    float power = 0.f; // power of 0 is linear.
    float maxAbsPower = 50.0f;
    float size = 20.f;

    Point(T xCenter, T yCenter);
    ~Point(){};

    // only works when the point passed in is normalized
    static duck::curve::Point<T> resizePoint(const duck::curve::Point<T>& point, const juce::Rectangle<float>& bounds);

    // clamps between -maxAbsPower and maxAbsPower
    void setPowerClamped(float power);
};


// =================================================================================


class CurveDisplay : public juce::Component, public juce::ValueTree::Listener {
private:
    void paint(juce::Graphics &g) override;
    void resized() override;

    void mouseDrag(const MouseEvent &event) override; 	
    void mouseUp(const MouseEvent &event) override;
    void mouseDoubleClick(const MouseEvent &event) override;

    // updates the curvePointsResizedBounds to match the new curvePointNormalized, then remakes the path and repaints.
    void updateResizedCurve(); 
    void updatePathSection(const duck::curve::Point<float>& from, const duck::curve::Point<float>& to);
    // looks for x in the BOUNDS of this component. -1 if not found
    static int findPointPositionIndex(float x, const std::vector<duck::curve::Point<float>>& points);
    int isOverPoint(const juce::Point<float>& position) const; // returns -1 if not over a point, otherwise returns index of the point



    // value tree stuff
    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property) override;
    void changePoint(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property);
    void updateTree() const;
public:
    CurveDisplay(duck::vt::ValueTree& tree);
    ~CurveDisplay(){updateTree();};
    
    static float getCurveAtNormalized(float normalizedX, const std::vector<duck::curve::Point<float>>& normalizedPoints);
    std::function<void()> onCurveUpdated = [](){};
    // @param x should be between the two points.
    static float interpolatePoints(const duck::curve::Point<float>& from, const duck::curve::Point<float>& to, float x);
    // returns a copy of the current normalized points.
    std::vector<duck::curve::Point<float>> getNormalizedPoints() const {return curvePointsNormalized;}
    static std::vector<duck::curve::Point<float>> getTreeNormalizedPoints(const duck::vt::ValueTree& vTree);
private:
    duck::vt::ValueTree& vTree;
    juce::Path path;
    // the normalized points which curvePointsResizedBounds is built from.
    std::vector<duck::curve::Point<float>> curvePointsNormalized;
    // the points in the bounds of this component.
    std::vector<duck::curve::Point<float>> curvePointsResizedBounds;
    // previous update mouse pos offset while dragging
    juce::Point<int> lastDragOffset{0,0};
    int isDraggingIndex = -1; // -1 if not dragging.

};


}; // namespace
