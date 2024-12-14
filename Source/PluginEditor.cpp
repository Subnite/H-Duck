/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
HentaiDuckEditor::HentaiDuckEditor(HentaiDuckProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      curveDisplay(audioProcessor.vTree),
      lengthSliderMs(10.f, 2000.f, 50.f),
      lookaheadSliderMs(0.f, 50.f, 0.f)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(800, 500);
    setResizable(true, true);
    setResizeLimits(400, 250, 1500, 1000);

    setupGifViewer();
    setupCurveDisplay();
    setupLengthSlider();
    setupLookaheadSlider();

    // make all visible
    addAndMakeVisible(gifViewer.get());
    addAndMakeVisible(&curveDisplay);
    addAndMakeVisible(&lengthSliderMs);
    addAndMakeVisible(&lookaheadSliderMs);
}

HentaiDuckEditor::~HentaiDuckEditor()
{
}

//==============================================================================
void HentaiDuckEditor::paintOverChildren(juce::Graphics &g)
{
    auto bounds = curveBounds;
    g.setColour(juce::Colours::grey.withLightness(0.6f));
    g.drawRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 5.f, 3.f);

    bounds = sliderBounds;
    g.drawRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 5.f, 3.f);
}

void HentaiDuckEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::purple.withSaturation(0.5f).withBrightness(0.10f));
}

void HentaiDuckEditor::resized()
{
    auto bounds = getLocalBounds();
    if (gifViewer.get() != nullptr)
        gifViewer->setBounds(bounds);
    auto paddedBounds = bounds.withSizeKeepingCentre(
        bounds.getWidth() - sectionPadding,
        bounds.getHeight() - sectionPadding);

    auto buttonsBounds = paddedBounds.removeFromRight(paddedBounds.getWidth() * 0.3f);
    buttonsBounds.removeFromLeft(sectionPadding);

    this->sliderBounds = buttonsBounds;
    this->curveBounds = paddedBounds;

    curveDisplay.setBounds(paddedBounds);
    lengthSliderMs.setBounds(buttonsBounds.removeFromBottom(buttonsBounds.getHeight()*0.5f));
    lookaheadSliderMs.setBounds(buttonsBounds);
}

//
//
//
//
//
// ==================== Setups ==========================
//
//
//
//
//
//

void HentaiDuckEditor::setupCurveDisplay()
{
    curveDisplay.onCurveUpdated = [this]()
    {
        audioProcessor.updateCurveValues(curveDisplay.getNormalizedPoints());
    };

    curveDisplay.onCurveUpdated(); // initial update
}

void HentaiDuckEditor::setupLengthSlider()
{
    auto &vTree = audioProcessor.vTree;
    using prop = Property;

    lengthSliderMs.setValuePrefix("Length: ");
    lengthSliderMs.setValuePostfix(" ms");
    lengthSliderMs.valueToString = [this](float val) -> std::string
    {
        float newVal = val;
        if (val < 1000)
        {
            lengthSliderMs.setValuePostfix(" ms");
        }
        else
        {
            newVal /= 1000;
            lengthSliderMs.setValuePostfix(" s");
        }
        return juce::String(newVal, 2, false).toStdString();
    };
    lengthSliderMs.onValueChanged = [this](float newVal)
    {
        audioProcessor.updateCurveLength(static_cast<double>(newVal));
    };

    lengthSliderMs.setValueTree(
        &vTree,
        vTree.getIDFromType(prop::T_LENGTH_MS).value_or("undefined"),
        vTree.getIDFromType(prop::P_RAW_NORMALIZED_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_DISPLAY_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_MIN_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_MAX_VALUE).value_or("undefined")
    );
}

void HentaiDuckEditor::setupLookaheadSlider()
{
    lookaheadSliderMs.setValuePrefix("Lookahead: ");
    lookaheadSliderMs.setValuePostfix(" ms");
    lookaheadSliderMs.valueToString = [this](float val) -> std::string
    {
        float newVal = val;
        if (val < 1000)
        {
            lookaheadSliderMs.setValuePostfix(" ms");
        }
        else
        {
            newVal /= 1000;
            lookaheadSliderMs.setValuePostfix(" s");
        }
        return juce::String(newVal, 2, false).toStdString();
    };
    lookaheadSliderMs.onValueChanged = [this](float newVal)
    {
        audioProcessor.updateLookahead(static_cast<double>(newVal));
    };

    lookaheadSliderMs.updateTreeOnDrag = false;

    using prop = Property;
    auto &vTree = audioProcessor.vTree;
    lookaheadSliderMs.setValueTree(
        &vTree,
        vTree.getIDFromType(prop::T_LOOKAHEAD_MS).value_or("undefined"),
        vTree.getIDFromType(prop::P_RAW_NORMALIZED_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_DISPLAY_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_MIN_VALUE).value_or("undefined"),
        vTree.getIDFromType(prop::P_MAX_VALUE).value_or("undefined")
    );
}

void HentaiDuckEditor::setupGifViewer() {
    auto json = duck::GifViewer::getGifJsonFile();

    // 2. get full settings json object
    var rootVar = JSON::parse(json);
    if (!rootVar.isVoid());
    DynamicObject* rootObject = rootVar.getDynamicObject();
    if (rootObject == nullptr) 
        gifViewer = std::make_unique<duck::GifViewer>("catgif.png", &audioProcessor.sidechainTriggeredBroadcaster);
;

    // 3. get the gif object in question
    auto gifVar = rootObject->getProperty(juce::Identifier{"active_gif"});
    if (!gifVar.isVoid() && gifVar.isString()) {
        String value = gifVar.operator juce::String();
        gifViewer = std::make_unique<duck::GifViewer>(value.toStdString(), &audioProcessor.sidechainTriggeredBroadcaster);
    } else {
        gifViewer = std::make_unique<duck::GifViewer>("catgif.png", &audioProcessor.sidechainTriggeredBroadcaster);
    }
    gifViewer->setBounds(getLocalBounds());
}