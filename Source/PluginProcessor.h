/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <mutex>
#include "Curve.h"
#include "DuckValueTree.h"
#include "RingBuffer.hpp"

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
    // updates the multiplier values in curveMultiplier to match the points.
    void updateCurveValues(const std::vector<duck::curve::Point<float>>& normalizedPoints);
    // changes the size of curveMultiplier (matches sampleRate) and applies the values from the tree.
    void updateCurveLength(const double& ms);
    // updates the size of the lookaheadBuffer and sets latency accordingly.
    void updateLookahead(double ms);

    // contains all info that is stored and restored from the plugin data block
    duck::vt::ValueTree vTree{};
private:
  // list of multiplier for the curve
  std::vector<float> curveMultiplier;
  // save us from threads (each access of curve multiplier)
  std::mutex curveGuard;
  // last read multiplier index
  int currentCurveIndex = 0;

  size_t amtTriggers = 0;
  std::vector<size_t> noteStartPositions;

  std::vector<RingBuffer<float>> lookaheadBuffer;

  // changes the size of the curveMultiplier vector.
  void resizeCurve(size_t newSize);

  // need length since it might be triggered more than once before it ends
  void applyCurve(juce::AudioBuffer<float> &buffer);

  template <typename T>
  static T getSliderMsFromTree(const duck::vt::ValueTree& tree, Property sliderTreeID, Property sliderPropertyID);



  size_t sampleRate = 48000;
  size_t samplesPerBlock = 512;
  size_t numChannels = 2;
  void busSettingsChanged(size_t sampleRate, size_t samplesPerBlock, size_t channels);
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HentaiDuckProcessor)
};
