/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <mutex>
#include <vector>

//==============================================================================
HentaiDuckProcessor::HentaiDuckProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
                       : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
.withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
.withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                       )
#endif
{
    resizeCurve(getSampleRate()/2);
    if (!vTree.isValid()) vTree.create();
}

HentaiDuckProcessor::~HentaiDuckProcessor()
{}

void HentaiDuckProcessor::resizeCurve(size_t newSize){
    curveMultiplier = std::vector<float>();
    curveMultiplier.reserve(newSize);
    curveMultiplier.resize(newSize);
    std::fill(curveMultiplier.begin(), curveMultiplier.end(), 1.0f);

    noteStartPositions.reserve(50);
    noteStartPositions.resize(50);
}

void HentaiDuckProcessor::applyCurve(juce::AudioBuffer<float> &buffer) {
    auto channels = buffer.getArrayOfWritePointers();
    auto numChannels = buffer.getNumChannels();

    auto guard = std::lock_guard<std::mutex>(curveGuard);
    for (size_t sample = 0; sample < buffer.getNumSamples(); sample++) {
        // if this sample is a trigger position, restart the curve counter
        for (size_t i = 0; i < amtTriggers; i++) {
            const auto startPos = noteStartPositions[i];
            if (sample == startPos) currentCurveIndex = 0;
        }

        for (size_t ch = 0; ch < numChannels; ch++){
            auto channel = buffer.getWritePointer(ch);
            channel[sample] *= 1-curveMultiplier[currentCurveIndex];
        }
        if (currentCurveIndex < curveMultiplier.size()-1){
            // this makes sure that the multiplier stays on the last one after the trigger.
            currentCurveIndex++;
        }
    }
}

void HentaiDuckProcessor::updateCurveValues(const duck::curve::CurveDisplay& display){
    auto lock = std::lock_guard<std::mutex>(curveGuard);

    for (size_t i = 0; i < curveMultiplier.size(); i++){
        float normX = i / static_cast<float>(curveMultiplier.size()-1);
        curveMultiplier[i] = display.getCurveAtNormalized(normX);
    }
}

//==============================================================================
void HentaiDuckProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    resizeCurve(sampleRate/2);
}

void HentaiDuckProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    // find positions to start the ducker
    amtTriggers = 0;

    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        if (message.isNoteOn(true)) {
            jassert(amtTriggers < noteStartPositions.size());
            noteStartPositions[amtTriggers] = metadata.samplePosition;
            amtTriggers++;
        }           
    }

    applyCurve(buffer);
}

const juce::String HentaiDuckProcessor::getName() const
{
    return juce::String("HentaiDuck");
}

bool HentaiDuckProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool HentaiDuckProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool HentaiDuckProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double HentaiDuckProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HentaiDuckProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int HentaiDuckProcessor::getCurrentProgram()
{
    return 0;
}

void HentaiDuckProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String HentaiDuckProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void HentaiDuckProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================


void HentaiDuckProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HentaiDuckProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================
bool HentaiDuckProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HentaiDuckProcessor::createEditor()
{
    return new HentaiDuckEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void HentaiDuckProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream stream(destData, true);
    if (vTree.isValid()){
        vTree.writeToStream(stream);
        // vTree.createXML("C:/Dev/Juce Projects/HentaiDuck/getStateOutputTree.xml");
    }
}

void HentaiDuckProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    vTree.copyFrom(data, sizeInBytes);
    if (!vTree.isValid()) vTree.create();
    // vTree.createXML("C:/Dev/Juce Projects/HentaiDuck/setStateOutputTree.xml");
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HentaiDuckProcessor();
}
