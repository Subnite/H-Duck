#include "Curve.h"
#include "Utils.h"
#include <algorithm>


template <typename T>
duck::curve::Point<T>::Point(T xCenter, T yCenter)
: coords(xCenter, yCenter)
{
}

template <typename T>
duck::curve::Point<T> duck::curve::Point<T>::resizePoint(const duck::curve::Point<T> &point, const juce::Rectangle<float> &bounds) {
    auto resizedPoint = duck::curve::Point<T>(point);
    resizedPoint.coords.x *= bounds.getWidth();
    resizedPoint.coords.x += bounds.getX();
    resizedPoint.coords.y *= bounds.getHeight();
    resizedPoint.coords.y += bounds.getY();

    return resizedPoint;
}

template <typename T>
void duck::curve::Point<T>::setPowerClamped(float power) {
    if (power > maxAbsPower) this->power = maxAbsPower;
    else if (power < -maxAbsPower) this->power = -maxAbsPower;
    else this->power = power;
}



// =================================================================



duck::curve::CurveDisplay::CurveDisplay(duck::vt::ValueTree& tree)
: vTree(tree), juce::ValueTree::Listener()
{
    curvePointsNormalized.push_back(duck::curve::Point<float>(0,1));
    curvePointsNormalized.push_back(duck::curve::Point<float>(1,0));

    // don't really need the listener, just push data and pull when created.
    // tree.addListener(this);
    auto newPoints = getTreeNormalizedPoints();
    if (newPoints.size() >= 2) curvePointsNormalized = newPoints; 
    updateResizedCurve();
}

void duck::curve::CurveDisplay::updateResizedCurve() {
    path = juce::Path();
    size_t i{};

    auto bounds = getLocalBounds();
    curvePointsResizedBounds = std::vector<duck::curve::Point<float>>();
    curvePointsResizedBounds.reserve(curvePointsNormalized.size());

    for (const auto& point : curvePointsNormalized) {
        curvePointsResizedBounds.push_back(
            duck::curve::Point<float>::resizePoint(point, bounds.toFloat())
        );
    }

    // update the path now too
    path = juce::Path{};
    for (size_t i = 0; i < curvePointsResizedBounds.size()-1 && i < curvePointsResizedBounds.capacity()-1; i++){
        path.startNewSubPath(curvePointsResizedBounds[i].coords);

        // add lines to path for each pixel
        updatePathSection(curvePointsResizedBounds[i], curvePointsResizedBounds[i+1]);
        // path.lineTo(curvePointsResizedBounds[i+1].coords); // make sure line is finished

        path.closeSubPath();
    }

    onCurveUpdated();
    repaint();
}

void duck::curve::CurveDisplay::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::red);
    const auto type = juce::PathStrokeType(3.f);
    g.strokePath(path, type);

    g.setColour(juce::Colours::black);
    for (const auto& point : curvePointsResizedBounds){
        float size = point.size;
        g.fillEllipse(point.coords.x-size/2, point.coords.y-size/2, size, size);
    }
}

void duck::curve::CurveDisplay::resized() {
    auto localBounds = getLocalBounds();
    

    updateResizedCurve();
}

float duck::curve::CurveDisplay::interpolatePoints(const duck::curve::Point<float>& from, const duck::curve::Point<float>& to, float x) const {
    if (from.coords.x == to.coords.x) return to.coords.y;

    float height = to.coords.y - from.coords.y;
    float width = to.coords.x - from.coords.x;

    if (x == to.coords.x-width) return from.coords.y;
    else if (x == from.coords.x+width) return to.coords.y;

    x = (x - from.coords.x) / width; // to make it normalized

    auto power = from.power;
    if (power > -0.005f && power < 0.005f) {
        return from.coords.y + height * x;
    }

    // the actual function
    float t1 = std::powf(juce::MathConstants<float>::euler, power*x) - 1;
    float t2 = std::powf(juce::MathConstants<float>::euler, power) - 1;
    float normalized = t1/t2;

    // that was normalized so now we adjust it to the points
    auto result = from.coords.y + normalized * height;

    // clamp because it sometimes fails and goes through the points
    auto biggest = std::max(from.coords.y, to.coords.y);
    auto smallest = std::min(from.coords.y, to.coords.y);
    result = std::clamp(result, smallest, biggest);

    return result;
}

void duck::curve::CurveDisplay::updatePathSection(const duck::curve::Point<float>& from, const duck::curve::Point<float>& to) {
    jassert(from.coords.x <= to.coords.x);

    auto bounds = getLocalBounds();
    size_t jump = 2; // essentially the resolution

    auto width = bounds.getWidth();
    auto height = bounds.getHeight();

    juce::Point<float> lastPoint = from.coords;
    for (size_t x = from.coords.x; x <= to.coords.x; x+=jump){
        auto y = interpolatePoints(from, to, x);
        auto nextPoint = juce::Point<float>(x, y);
        path.addLineSegment({lastPoint, nextPoint}, 2.0f);
        lastPoint = nextPoint;
    }
    path.addLineSegment({lastPoint, to.coords}, 2.f);
}

void duck::curve::CurveDisplay::mouseDrag(const juce::MouseEvent& event) {
    auto offset = event.getOffsetFromDragStart();
    auto clickPos = event.mouseDownPosition;
    float yOffset = offset.y - lastDragOffset.y;
    auto bounds = getLocalBounds();

    // find the point to change the curve of
    int pointToPowerIndex = findPointPositionIndex(clickPos.x);

    // check if you're draggin or supposed to drag a point.
    auto checkPoint = juce::Point<float>(clickPos.x + offset.x, clickPos.y + offset.y);
    if (isDraggingIndex == -1) {
        isDraggingIndex = isOverPoint(checkPoint);
    }
    if (isDraggingIndex != -1) {
        // is currently dragging a point.
        // clamp to the bounds of the component
        duck::utils::clampToBounds<float>(checkPoint, bounds.toFloat());

        auto unchangedX = curvePointsNormalized[isDraggingIndex].coords.x;
        curvePointsNormalized[isDraggingIndex].coords = juce::Point<float>(
            (isDraggingIndex == 0 || isDraggingIndex == curvePointsNormalized.size()-1)
            ? curvePointsNormalized[isDraggingIndex].coords.x
            : checkPoint.x / bounds.getWidth(),

            checkPoint.y / bounds.getHeight()
        );

        // check for x overlaps
        if (isDraggingIndex > 0 && isDraggingIndex < curvePointsNormalized.size()-1){
            if (curvePointsNormalized[isDraggingIndex].coords.x == curvePointsNormalized[isDraggingIndex+1].coords.x
                || curvePointsNormalized[isDraggingIndex].coords.x == curvePointsNormalized[isDraggingIndex-1].coords.x)
                curvePointsNormalized[isDraggingIndex].coords.x = unchangedX;

        }

        // check if it passed another point.
        if (isDraggingIndex > 0) {
            if (curvePointsNormalized[isDraggingIndex].coords.x < curvePointsNormalized[isDraggingIndex-1].coords.x) {
                std::iter_swap(
                    curvePointsNormalized.begin() + isDraggingIndex,
                    curvePointsNormalized.begin() + isDraggingIndex-1
                );
                isDraggingIndex -= 1;
            }
        }
        if (isDraggingIndex < curvePointsNormalized.size()-1) {
            if (curvePointsNormalized[isDraggingIndex].coords.x > curvePointsNormalized[isDraggingIndex+1].coords.x) {
                std::iter_swap(
                    curvePointsNormalized.begin() + isDraggingIndex,
                    curvePointsNormalized.begin() + isDraggingIndex+1
                );
                isDraggingIndex += 1;
            }
        }
    }

    // change power
    else if (pointToPowerIndex != -1) {
        float multiplier = 0.05f;
        if (curvePointsNormalized[pointToPowerIndex].coords.y > curvePointsNormalized[pointToPowerIndex+1].coords.y) {
            // invert because the second point is higher up
            multiplier *= -1;
        }

        curvePointsNormalized[pointToPowerIndex].setPowerClamped(
            curvePointsNormalized[pointToPowerIndex].power + yOffset * -multiplier
        );
    }

    lastDragOffset = offset;
    updateResizedCurve();
}

void duck::curve::CurveDisplay::mouseUp(const juce::MouseEvent& event) {
    lastDragOffset = juce::Point<int>(0,0);
    isDraggingIndex = -1;
}

void duck::curve::CurveDisplay::mouseDoubleClick(const juce::MouseEvent& event) {
    int pointIndex = findPointPositionIndex(event.mouseDownPosition.x);
    if (pointIndex == -1) return; // safety check

    auto bounds = getLocalBounds();

    // add point
    if (event.mods.isShiftDown()){
        auto normalizedPos = juce::Point<float>(event.mouseDownPosition.x / bounds.getWidth(), event.mouseDownPosition.y / bounds.getHeight());
        curvePointsNormalized.insert (
            curvePointsNormalized.begin() + pointIndex+1,
            duck::curve::Point<float>(normalizedPos.x, normalizedPos.y)
        );
    }

    // remove point
    else if (event.mods.isAltDown()) {
        auto pointIndex = isOverPoint(event.mouseDownPosition);
        if (pointIndex != -1 && pointIndex != 0 && pointIndex != curvePointsNormalized.size()-1) {
            curvePointsNormalized.erase(curvePointsNormalized.begin() + pointIndex);
        }
    }

    // straighten curve
    else {
        int pointIndex = findPointPositionIndex(event.mouseDownPosition.x);
        if (pointIndex != -1) curvePointsNormalized[pointIndex].setPowerClamped(0.0f);
    }

    updateResizedCurve();
}

int duck::curve::CurveDisplay::findPointPositionIndex(float x) const {
    int pointToPowerIndex = -1;
    for (size_t i = 0; i < curvePointsResizedBounds.size()-1 && curvePointsResizedBounds.size() >= 2; i++) {
        const auto currentX = curvePointsResizedBounds[i].coords.x;
        const auto nextX = curvePointsResizedBounds[i+1].coords.x;
        if (x >= currentX && x <= nextX){
            pointToPowerIndex = i;
            break;
        }
    }

    return pointToPowerIndex;
}

int duck::curve::CurveDisplay::isOverPoint(const juce::Point<float>& position) const {
    int index = -1;
    for (int i = 0; i < curvePointsResizedBounds.size(); i++) {
        const auto& point = curvePointsResizedBounds[i];
        const auto pointBounds = juce::Rectangle<float>(
            point.coords.x-point.size/2,
            point.coords.y-point.size/2,
            point.size,
            point.size
        );
        if (pointBounds.contains(position)) {
            index = i;
            break;
        }
    }

    return index;
}

float duck::curve::CurveDisplay::getCurveAtNormalized(float normalizedX) const {
    int pos = findPointPositionIndex(normalizedX * getLocalBounds().getWidth());
    jassert(pos != -1);

    if (normalizedX == 0) return curvePointsNormalized[0].coords.y;
    else if (normalizedX == 1) return curvePointsNormalized.back().coords.y;

    return interpolatePoints(curvePointsNormalized[pos], curvePointsNormalized[pos+1], normalizedX);
}


void duck::curve::CurveDisplay::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier &property) {
    auto treeType = duck::vt::ValueTree::getTreeType(treeWhosePropertyHasChanged);

    switch (treeType) {
        case duck::vt::Tree::POINT:
            changePoint(treeWhosePropertyHasChanged, property);
            break;
        default:
            break; 
    }
}

void duck::curve::CurveDisplay::changePoint(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) {
    const auto index = treeWhosePropertyHasChanged.getParent().indexOf(treeWhosePropertyHasChanged);
    auto val = treeWhosePropertyHasChanged.getProperty(property);
    auto type = duck::vt::ValueTree::getPropertyType(property);
    
    if (index < curvePointsNormalized.size()){    
        switch (type) {
            case duck::vt::Property::MAX_ABSOLUTE_POWER:
                curvePointsNormalized[index].maxAbsPower = val;
                break;
            default:
                break;

        }
    }
}

std::vector<duck::curve::Point<float>> duck::curve::CurveDisplay::getTreeNormalizedPoints() const {
    const auto vtRoot = vTree.getRoot();
    const auto curve = vtRoot.getChildWithName("CurveData");
    const auto points = curve.getChildWithName("NormalizedPoints");
    const auto amtPoints = points.getNumChildren();

    std::vector<duck::curve::Point<float>> vec{};
    if (amtPoints <= 0) return vec;

    vec.reserve(amtPoints);
    // vec.resize(amtPoints);

    for (size_t i = 0; i < amtPoints; i++) {
        const auto p = points.getChild(i);
        const auto coords = p.getChildWithName("Coords");
        auto duckP = duck::curve::Point<float>(coords.getProperty("X"), coords.getProperty("Y"));
        duckP.size = p.getProperty("Size");
        duckP.power = p.getProperty("Power");
        duckP.maxAbsPower = p.getProperty("MaxAbsolutePower");
        vec.push_back(duckP);
 //       vec[i] = duckP;
    }

    return vec;
}

void duck::curve::CurveDisplay::updateTree() const {
    using vt = duck::vt::ValueTree;
    using t = duck::vt::Tree;
    using p = duck::vt::Property;

    if (!vTree.isValid()) return;

    auto undoManager = vTree.getUndoManager();


    juce::ValueTree cd {vt::getIDFromType(t::CURVE_DATA)};
    juce::ValueTree np {vt::getIDFromType(t::NORMALIZED_POINTS)};
    
    for (const auto& point : curvePointsNormalized) {
        juce::ValueTree treePoint{vt::getIDFromType(t::POINT)};
        treePoint.setProperty(vt::getIDFromType(p::POWER), {point.power}, undoManager);
        treePoint.setProperty(vt::getIDFromType(p::MAX_ABSOLUTE_POWER), {point.maxAbsPower}, undoManager);
        treePoint.setProperty(vt::getIDFromType(p::SIZE), {point.size}, undoManager);
        
        juce::ValueTree coords{vt::getIDFromType(t::COORDS)};
        coords.setProperty(vt::getIDFromType(p::X), {point.coords.x}, undoManager);
        coords.setProperty(vt::getIDFromType(p::Y), {point.coords.y}, undoManager);

        treePoint.appendChild(coords, undoManager);
        np.appendChild(treePoint, undoManager);
    }

    cd.appendChild(np, undoManager);
    vTree.setChild(t::CURVE_DATA, cd);
}