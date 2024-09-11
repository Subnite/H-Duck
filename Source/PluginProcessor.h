/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <mutex>
#include "Curve.h"
#include "ValueTreeManager.h"

//==============================================================================

class HentaiDuckProcessor  : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
, public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    HentaiDuckProcessor();
    ~HentaiDuckProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void updateCurveValues(const std::vector<duck::curve::Point<float>>& normalizedPoints);
    void updateCurveLength(const double& ms);
    duck::vt::ValueTree vTree{};
private:
    std::vector<float> curveMultiplier; // list of multiplier for the curve
    std::mutex curveGuard; // save us from threads (each access of curve multipler)
    int currentCurveIndex = 0; // last read multiplier index
    // float finalMultiplier = 1.0f;
    size_t amtTriggers = 0;

    std::vector<size_t> noteStartPositions;

    void resizeCurve(size_t newSize);

    // need length since it might be triggered more than once before it ends
    void applyCurve(juce::AudioBuffer<float>& buffer); 
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HentaiDuckProcessor)
};
