#pragma once
#include <JuceHeader.h>
#include <functional>
#include <string>
#include "ValueTreeManager.h"

namespace subnite {

template<typename T>
class Slider : public juce::Component {
public:
    /** Sets up the slider values.
        @param minValue The minimum display value.
        @param maxValue The maximum display value.
        @param defaultValue The default display value, should be between min and max.
        @see setValueTree, normalizedToDisplayed, valueToString
    */
    Slider(T minValue, T maxValue, T defaultValue);
    ~Slider();

    /**
    The function called when trying to display the value.

    Should convert the value to a string. This function lets the user pick how they want to display the value, for example when using seconds and milliseconds.
    You can also set the prefix and postfix in here, but the resulting string should not contain those.

    This gets called in paint() when it wants to display the value.
    
    @param value The Display value (so not normalized) from minValue to maxValue.
    @return The string that was converted from the value.
    
    Example code:
    @code
    subnite::Slider<float> mySlider{5.f, 1500.f, 5.f}; // slider in ms
    mySlider.valueToString = [mySlider](T value)->std::string {
        bool isMilli = value < 1000.f;
    
        std::string s = isMilli ? std::to_string(value) : std::to_string(value/1000.f);
        std::string postfix = isMilli ? " ms" : " s";
        mySlider.setValuePostfix(postfix);

        return s;
    }
    @endcode

    @see setValuePrefix setValuePostfix
    */
    std::function<std::string(T value)> valueToString = [](T value){
        return std::to_string(value);
    };


    /**
    Converts the normalized raw value to the value you wish.
     
    This lambda is especially handy for non linear values, like Hz. But could also be used when you want more control over smaller numbers than bigger ones, like an exponential function.
     
    @param normalizedValue This is the value of the slider between (and inclusive) [0.0 : 1.0].
    @return The displayed value, mapped from the normalizedValue.
     
    Example code:
    @code
    subnite::Slider<float> mySlider{ 5.f, 1500.f, 5.f };
    mySlider.normalizedToDisplayed = [mySlider](double normalizedValue)->float {
        // since normalizedValue is... normalized, the cubic response will still be normalized.
        const double cubicResponse = pow(normalizedValue, 3);

        // now map it to the range of the display values.
        auto result = cubicResponse * (mySlider.getMaxValue() - mySlider.getMinValue()) + mySlider.getMinValue();

        return static_cast<float>(result);
    }
    @endcode
    */
    std::function<T(double normalizedValue)> normalizedToDisplayed = [this](double normalizedValue){
        return static_cast<T>(normalizedValue) * (maxValue - minValue) + minValue;
    };
    
    /** Called whenever the display value changed. */
    std::function<void(T changedValue)> onValueChanged = [](T c){};
    
    /** Sets the prefix displayed on the slider. @see paint, getValueString */ 
    void setValuePrefix(std::string prefix);
    /** Sets the postfix displayed on the slider. @see paint, getValueString */ 
    void setValuePostfix(std::string postfix);
    /** Updates the displayValue and sets the normalizedRawValue accordingly. @see normalizedRawValue*/
    void setValue(T newValue);
    /** @return The minimum display value. */
    T getMinValue() const { return minValue; }
    /** @return The maximum display value. */
    T getMaxValue() const { return maxValue; }
    /** @return The default display value. */
    T getDefaultValue() const { return defaultValue; }
    
    /** @return The display value as a string, with prefix and postfix. @see setValuePrefix, setValuePostfix, normalizedToDisplayed */
    std::string getValueString() const;
    
    /** @return The value linearly mapped to the min and max angle. */
    double getValueAngle(const double& minAngle, const double& maxAngle) const;
    /** Replaces the state of this class with the values found in the value tree if it was set up. @see setValueTree */
    void getFromValueTree();

    /** Updates the slider properties in the value tree if it exists. */
    void updateValueTree();

    /** Sets up the value tree, and updates this component from its values. */
    void setValueTree(subnite::vt::ValueTreeBase* parentTree, juce::Identifier uniqueSliderTreeID, juce::Identifier rawNormalizedValueID, juce::Identifier displayValueID, juce::Identifier minValueID, juce::Identifier maxValueID);

    // optionals
    /** If the value should be displayed when you hover over the slider. */
    bool displayValueOnHover = true;
    /** If the tree gets updates while you are dragging, else it updates on mouseUp. */
    bool updateTreeOnDrag = true;
private:
    /** The value on which the slider is based, this is always inclusively in the range [0.0 : 1.0]. */
    double normalizedRawValue;
    /** The displayed value, between minValue and maxValue */
    T displayedValue;
    T minValue, maxValue, defaultValue;
    
    /** The value tree used for finding and saving the sliders parameters. */
    subnite::vt::ValueTreeBase* vTree = nullptr;
    /** The UNIQUE identifier associated with this slider. */
    juce::Identifier sliderTreeUniqueID{"undefined"};
    juce::Identifier rawNormalizedValueID{"undefined"}, displayValueID{"undefined"}, minValueID{"undefined"},  maxValueID{"undefined"};

    std::string prefix = "";
    std::string postfix = "";
    
    /** Used in paint. @see paint */
    bool isHovering = false;
    /** Used for onDrag. @see onDrag */
    juce::Point<int> lastDragOffset{0, 0};
    
    /** Updates the displayed value from the normalized value. @param updateTree Calls onValueChanged() if set to true. */
    void updateDisplayedValueChecked(bool updateTree = true); 

    /** Draws the slider. */
    void paint(juce::Graphics &g) override;
    
    /** Starts displaying the slider value if displayValueOnHover = true. @see displayValueOnHover, normalizedToDisplayed, setValuePrefix, setValuePostfix */
    void mouseEnter(const juce::MouseEvent &event) override;
    /** Stops displaying the value. @see mouseEnter */
    void mouseExit(const juce::MouseEvent &event) override;
    /** Hides mouse */
    void mouseDown(const juce::MouseEvent &event) override;
    /** Shows mouse again and updates the tree. */
    void mouseUp(const juce::MouseEvent &event) override;
    /** Updates the tree and value if updateValueOnHover = true. */
    void mouseDrag(const juce::MouseEvent &event) override;
    /** Resets the slider value to defaultValue. */
    void mouseDoubleClick(const juce::MouseEvent &event) override; // reset to default

}; // Slider class

} // namespace
