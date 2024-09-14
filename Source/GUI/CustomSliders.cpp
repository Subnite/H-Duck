#include "CustomSliders.h"
#include <algorithm>


template <typename T>
subnite::Slider<T>::Slider(T minValue, T maxValue, T defaultValue)
    : normalizedRawValue(0), displayedValue(defaultValue),
    minValue(minValue), maxValue(maxValue), defaultValue(defaultValue),
    vTree(vTree)
{
    jassert(minValue < maxValue); // Minimum value should be lower than max
    jassert(defaultValue >= minValue && defaultValue <= maxValue); // Default value should be between [min : max] inclusive

    T range = maxValue - minValue;
    T v = defaultValue - minValue;
    normalizedRawValue = v / static_cast<double>(range);
    updateDisplayedValueChecked(); // in case getFromValueTree fails

    // getFromValueTree();
}

template <typename T>
subnite::Slider<T>::~Slider(){
    // maybe make sure that the mouse is normal
    setMouseCursor(juce::MouseCursor::NormalCursor);
    updateValueTree();
}

/*
    updates the displayedValue from the normalizedRawValue, and clamping it between min and max. It's up to the user's normalizedToDisplayed function to make sure this doesn't need to be clamped.
*/
template <typename T>
void subnite::Slider<T>::updateDisplayedValueChecked() {
    const T newValue = normalizedToDisplayed(normalizedRawValue);
    jassert(newValue >= minValue && newValue <= maxValue); // Value should be between [min : max] inclusive. The normalizedToDisplayed function did not do this
    displayedValue = std::clamp(newValue, minValue, maxValue);

    onValueChanged(displayedValue);
}


template <typename T>
std::string subnite::Slider<T>::getValueString() const {
    return prefix + valueToString(displayedValue) + postfix;
}

template <typename T>
void subnite::Slider<T>::setValuePrefix(std::string prefix) {
    this->prefix = prefix;
}

template <typename T>
void subnite::Slider<T>::setValuePostfix(std::string postfix) {
    this->postfix = postfix;
}

template <typename T>
void subnite::Slider<T>::setValue(T newValue) {
    jassert (newValue <= maxValue && newValue >= minValue);

    newValue = std::clamp(newValue, minValue, maxValue);
    displayedValue = newValue;
    
    normalizedRawValue = (newValue-minValue) / static_cast<double>(maxValue - minValue);
    onValueChanged(displayedValue);
}

template <typename T>
double subnite::Slider<T>::getValueAngle(const double& minAngle, const double& maxAngle) const {
    const auto range = maxAngle - minAngle;
    return normalizedRawValue * range + minAngle;
}

template <typename T>
void subnite::Slider<T>::getFromValueTree() {
    if (vTree == nullptr) return;

    auto slider = vTree->getRoot().getChildWithName(sliderTreeUniqueID);
    if (!slider.isValid()) return; // didn't exist or wasn't child of root
    
    auto raw = static_cast<double>(slider.getProperty(rawNormalizedValueID));
    auto min = static_cast<double>(slider.getProperty(minValueID));
    auto max = static_cast<double>(slider.getProperty(maxValueID));
    auto isMs = static_cast<bool>(slider.getProperty(isMsID));
    auto displayVal = static_cast<double>(slider.getProperty(displayValueID));
    
    this->normalizedRawValue = raw;
    this->minValue = min;
    this->maxValue = max;
    this->displayedValue = displayVal;
    this->isMS = isMs;
    
    updateDisplayedValueChecked(); // we're saving the display value too, but this is still useful for initialization.
}

template <typename T>
void subnite::Slider<T>::updateValueTree() {
    if (vTree == nullptr) return;

    juce::ValueTree slider{sliderTreeUniqueID};

    slider.setProperty(rawNormalizedValueID, normalizedRawValue, nullptr);
    slider.setProperty(minValueID, minValue, nullptr);
    slider.setProperty(maxValueID, maxValue, nullptr);
    slider.setProperty(isMsID, isMS, nullptr); // this will be a variable later
    slider.setProperty(displayValueID, displayedValue, nullptr);
    
    vTree->setChild(sliderTreeUniqueID, slider);
}


// ============= Visuals   =================


template <typename T>
void subnite::Slider<T>::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    auto maxSize = std::min(bounds.getWidth(), bounds.getHeight());
    bounds = bounds.withSizeKeepingCentre(maxSize, maxSize);

    g.setColour(juce::Colours::grey.withLightness(0.3f));
    g.fillEllipse(bounds.getCentreX()-maxSize/2.f, bounds.getCentreY()-maxSize/2.f, maxSize, maxSize);

    // line at value pos
    const double remove = -1.5;
    const double pi = 3.1415;
    const auto angle = getValueAngle(pi-(remove/2), 0+(remove/2)); // radians
    auto r = maxSize*0.5*0.8;
    auto center = bounds.getCentre();
    float x = r*cos(angle) + center.x;
    float y = r*-sin(angle) + center.y; // -sin because y 0 is top in juce

    g.setColour(juce::Colours::red);
    g.fillEllipse(x-4, y, 8, 8);
    
    if (isHovering && displayValueOnHover) {
        auto textBounds = bounds.withSizeKeepingCentre(bounds.getWidth()*0.5f, bounds.getHeight()*0.5f);
        g.setColour(juce::Colours::white);
        g.drawFittedText(getValueString(), textBounds, juce::Justification::centredBottom, 20);
    }
}


// ============= behaviors =================


template <typename T>
void subnite::Slider<T>::mouseEnter(const juce::MouseEvent& e) {
    isHovering = true;
    repaint();
}

template <typename T>
void subnite::Slider<T>::mouseExit(const juce::MouseEvent& e) {
    isHovering = false;
    repaint();
}

template <typename T>
void subnite::Slider<T>::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isLeftButtonDown()){
        setMouseCursor(juce::MouseCursor::NoCursor);
    }
    // right click change the value from text.
}

template <typename T>
void subnite::Slider<T>::mouseUp(const juce::MouseEvent& e) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
    lastDragOffset.setXY(0, 0);
    // should reset mouse position but idk how
    Desktop::getInstance().getMainMouseSource().setScreenPosition(e.getMouseDownScreenPosition().toFloat());

    updateValueTree();
}

template <typename T>
void subnite::Slider<T>::mouseDrag(const juce::MouseEvent& e) {
    if (e.mods.isLeftButtonDown()){
        const auto offset = e.getOffsetFromDragStart();
        auto distanceFromLast = offset - lastDragOffset;

        const auto maxDist = (abs(distanceFromLast.x) > abs(distanceFromLast.y)) ? distanceFromLast.x : -distanceFromLast.y;
        normalizedRawValue += maxDist/1000.0;
        normalizedRawValue = std::clamp(normalizedRawValue, 0.0, 1.0);

        lastDragOffset = offset;
        updateDisplayedValueChecked();
        repaint();

        if (offset.getDistanceSquaredFrom({0,0}) > 50) {
            Desktop::getInstance().getMainMouseSource().setScreenPosition(e.getMouseDownScreenPosition().toFloat()); // sometimes reset so it remains smooth
            lastDragOffset.setXY(0,0);
        }
    }
}

template <typename T>
void subnite::Slider<T>::mouseDoubleClick(const juce::MouseEvent& e) {
    if (e.mods.isLeftButtonDown()){
        setValue(defaultValue);
        repaint();
    }

    updateValueTree();
}

template <typename T>
void subnite::Slider<T>::setValueTree(subnite::vt::ValueTreeBase* parentTree, juce::Identifier uniqueSliderTreeID,
juce::Identifier rawNormalizedValueID, juce::Identifier displayValueID, juce::Identifier minValueID, juce::Identifier maxValueID, juce::Identifier isMsID) {
    vTree = parentTree;
    sliderTreeUniqueID = rawNormalizedValueID; // the unique tree to look for
    
    // properties
    this->rawNormalizedValueID = rawNormalizedValueID;
    this->displayValueID = displayValueID;
    this->minValueID = minValueID;
    this->maxValueID = maxValueID;
    this->isMsID = isMsID;

    getFromValueTree();
}




template class subnite::Slider<double>;
template class subnite::Slider<float>;
template class subnite::Slider<int>;