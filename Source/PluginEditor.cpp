/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
HentaiDuckEditor::HentaiDuckEditor(HentaiDuckProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    curveDisplay(audioProcessor.vTree)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (500, 500);
    setResizable(true, true);
    setResizeLimits(250, 250, 1500, 1000);

    curveDisplay.onCurveUpdated = [this](){
        audioProcessor.updateCurveValues(curveDisplay);
    };

    addAndMakeVisible(&curveDisplay);
}

HentaiDuckEditor::~HentaiDuckEditor()
{

}

//==============================================================================
void HentaiDuckEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::red.withSaturation(0.2f).withBrightness(0.35f));
}

void HentaiDuckEditor::resized()
{
    auto bounds = getLocalBounds();
    size_t removeAmt = 50;
    auto paddedBounds = bounds.withSizeKeepingCentre(
        bounds.getWidth() - removeAmt,
        bounds.getHeight() - removeAmt
    );
    curveDisplay.setBounds(paddedBounds);
}
