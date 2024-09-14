/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Curve.h"
#include "CustomSliders.h"

//==============================================================================
/**
*/
class HentaiDuckEditor  : public juce::AudioProcessorEditor
{
public:
    HentaiDuckEditor (HentaiDuckProcessor&);
    ~HentaiDuckEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    static const size_t sectionPadding = 20;
    static const size_t componentPadding = 10;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HentaiDuckProcessor& audioProcessor;
    
    duck::curve::CurveDisplay curveDisplay;
    subnite::Slider<float> lengthSliderMs;
    subnite::Slider<float> lookaheadSliderMs;

    void setupCurveDisplay();
    void setupLengthSlider();
    void setupLookaheadSlider();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HentaiDuckEditor)
};
