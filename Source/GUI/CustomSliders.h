#pragma once
#include <JuceHeader.h>
#include <functional>
#include <string>

namespace subnite {

template<typename T>
class Slider : public juce::Component {
public:
    Slider(T minValue, T maxValue, T defaultValue);
    ~Slider();

    // no prefix or postfix needed, just the value as a string. This is not the normalized value, but displayed.
    std::function<std::string(const T& value)> valueToString = [](const T& value){
        return std::to_string(value);
    };

    // you convert the raw normalized value into your wished value, so things like Hz can be customized
    std::function<T(const double& normalizedValue)> normalizedToDisplayed = [this](const double& normalizedValue){
        return static_cast<T>(normalizedValue) * (maxValue - minValue) + minValue;
    };     

    void setValuePrefix(std::string prefix);
    void setValuePostfix(std::string postfix);
    void setValue(T newValue); // updates the Display value and also raw normalized.

    std::string getValueString() const; // returns the display value as a string with pre and postfix
    double getValueAngle(const double& minAngle, const double& maxAngle) const; // maps the normalizedValue to the range provided.
private:
    double normalizedRawValue; // always from 0 to 1, used for value calculations
    T displayedValue; // between min and max, used for displayed values
    const T minValue, maxValue, defaultValue;
    bool displayValueOnHover = true;

    std::string prefix = "";
    std::string postfix = "";

    bool isHovering = false;
    juce::Point<int> lastDragOffset{0,0};

    void updateDisplayedValueChecked(); // updates the display value, and checks if normalizedToDisplayed is between min and max inclusive. Otherwise clamp it.
    
    void paint(juce::Graphics &g) override;

    void mouseEnter(const juce::MouseEvent &event) override; // hover is true
    void mouseExit(const juce::MouseEvent &event) override; // hover is false (unless drag?)
    void mouseDown(const juce::MouseEvent &event) override; // hide mouse, right click type value
    void mouseUp(const juce::MouseEvent &event) override; // normal mouse
    void mouseDrag(const juce::MouseEvent &event) override; // update value
    void mouseDoubleClick(const juce::MouseEvent &event) override; // reset to default

}; // Slider class


} // namespace