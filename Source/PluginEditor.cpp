/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
HentaiDuckEditor::HentaiDuckEditor(HentaiDuckProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    curveDisplay(audioProcessor.vTree),
    lengthSliderMs(10.f, 2000.f, 50.f, &audioProcessor.vTree)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (800, 500);
    setResizable(true, true);
    setResizeLimits(400, 250, 1500, 1000);

    // update curveDisplay settings

    curveDisplay.onCurveUpdated = [this](){
        audioProcessor.updateCurveValues(curveDisplay.getNormalizedPoints());
    };

    curveDisplay.onCurveUpdated(); // initial update
    
    // update lengthSliderMs settings
    lengthSliderMs.setValuePrefix("Length: ");
    lengthSliderMs.setValuePostfix(" ms");
    lengthSliderMs.valueToString = [this](const float& val)->std::string{
        float newVal = val;
        if (val < 1000) {
            lengthSliderMs.setValuePostfix(" ms");
        }
        else {
            newVal /= 1000;
            lengthSliderMs.setValuePostfix(" s");
        }
        return juce::String(newVal, 2, false).toStdString();
    };
    lengthSliderMs.onValueChanged = [this](const float& newVal){
        audioProcessor.updateCurveLength(static_cast<double>(newVal));
    };

    lengthSliderMs.getFromValueTree();

    // make all visible
    addAndMakeVisible(&curveDisplay);
    addAndMakeVisible(&lengthSliderMs);
}

HentaiDuckEditor::~HentaiDuckEditor()
{

}

//==============================================================================
void HentaiDuckEditor::paint (juce::Graphics& g)
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(juce::Colours::purple.withSaturation(0.5f).withBrightness(0.10f));

        auto bounds = curveDisplay.getBounds();
        g.setColour(juce::Colours::grey.withLightness(0.6f));
        g.drawRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 5.f, 3.f);

        bounds = lengthSliderMs.getBounds();

        g.drawRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 5.f, 3.f);

    }

void HentaiDuckEditor::resized()
{
    auto bounds = getLocalBounds();
    auto paddedBounds = bounds.withSizeKeepingCentre(
        bounds.getWidth() - sectionPadding,
        bounds.getHeight() - sectionPadding
    );

    auto buttonsBounds = paddedBounds.removeFromRight(paddedBounds.getWidth()*0.3f);
    buttonsBounds.removeFromLeft(sectionPadding);

    curveDisplay.setBounds(paddedBounds);
    lengthSliderMs.setBounds(buttonsBounds);

}
