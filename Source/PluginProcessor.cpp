/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "Curve.h"
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
    if (!vTree.isValid()) vTree.create();
    updateCurveLength(getSliderMsFromTree<double>(vTree, Property::T_LENGTH_MS, Property::P_DISPLAY_VALUE));
}

HentaiDuckProcessor::~HentaiDuckProcessor()
{}

void HentaiDuckProcessor::resizeCurve(size_t newSize){
    std::lock_guard<std::mutex> lock{curveGuard};
    curveMultiplier = std::vector<float>();
    curveMultiplier.reserve(newSize);
    curveMultiplier.resize(newSize);
    std::fill(curveMultiplier.begin(), curveMultiplier.end(), 1.0f);

    noteStartPositions.reserve(50);
    noteStartPositions.resize(50);

    if (newSize > 0) currentCurveIndex = newSize-1; 
}

void HentaiDuckProcessor::updateCurveLength(const double& ms) {
    auto samples = static_cast<size_t>(getSampleRate() * (ms/1000));
    resizeCurve(samples);
    updateCurveValues(duck::curve::CurveDisplay::getTreeNormalizedPoints(vTree));
}

void HentaiDuckProcessor::applyCurve(juce::AudioBuffer<float> &buffer) {
    auto channels = buffer.getArrayOfWritePointers();
    auto amtChannels = buffer.getNumChannels();

    auto guard = std::lock_guard<std::mutex>(curveGuard);
    for (size_t sample = 0; sample < buffer.getNumSamples(); sample++) {

        // if this sample is a trigger position, restart the curve counter
        for (size_t i = 0; i < amtTriggers; i++) {
            const auto startPos = noteStartPositions[i];
            if (sample == startPos) {
                currentCurveIndex = 0;
                sidechainTriggeredBroadcaster.sendChangeMessage();
            }
        }

        for (size_t ch = 0;
        ch < amtChannels &&
        (currentCurveIndex < curveMultiplier.size() && currentCurveIndex >= 0)
        && (ch < lookaheadBuffer.size())
        ; ch++)
        {
            auto channel = buffer.getWritePointer(ch);
            auto latentSample = lookaheadBuffer[ch].insertAndPop(channel[sample]);
            latentSample *= 1-curveMultiplier[currentCurveIndex];
            channel[sample] = latentSample;
        }
        if (currentCurveIndex < curveMultiplier.size()-1){
            // this makes sure that the multiplier stays on the last one after the trigger.
            currentCurveIndex++;
        }
    }

    // now delay them
    // for (size_t channel = 0; channel < numChannels && channel < lookaheadBuffer.size(); channel++){
    //     auto chPtr = buffer.getWritePointer(channel);
    //     for (size_t sample = 0; sample < buffer.getNumSamples() && sample < lookaheadBuffer[channel].size(); sample++) {
    //         auto latentSample = lookaheadBuffer[channel].insertAndPop(chPtr[sample]);
    //         chPtr[sample] = latentSample;
    //     }

    // }
}

void HentaiDuckProcessor::updateCurveValues(const std::vector<duck::curve::Point<float>>& normalizedPoints) {
    auto lock = std::lock_guard<std::mutex>(curveGuard);

    for (size_t i = 0; i < curveMultiplier.size(); i++){
        float normX = i / static_cast<float>(curveMultiplier.size()-1);
        curveMultiplier[i] = duck::curve::CurveDisplay::getCurveAtNormalized(normX, normalizedPoints);
    }
}

void HentaiDuckProcessor::updateLookahead(double ms) {
    jassert(ms >= 0);
    auto samples = static_cast<size_t>(this->sampleRate * (ms/1000));

    if (lookaheadBuffer.size() > 0) {

        if (samples > lookaheadBuffer[0].capacity()){
            // make new and bigger buffers.
            // TODO: 
            bool kanker = true;
        }
        // set size to match samples
        for (auto& buf : lookaheadBuffer) {
            buf.setRelativeSize(samples);
        }
    }

    setLatencySamples(samples);
}

template <typename T>
T HentaiDuckProcessor::getSliderMsFromTree(const duck::vt::ValueTree& tree, Property sliderTreeID, Property sliderPropertyID) {
    const auto sliderTree = tree.getRoot().getChildWithName(tree.getIDFromType(sliderTreeID).value_or("undefined"));
    double displayValueFromTree = sliderTree.getProperty(tree.getIDFromType(sliderPropertyID).value_or("undefined"));
    return static_cast<T>(displayValueFromTree);
}





//==============================================================================



void HentaiDuckProcessor::busSettingsChanged(size_t sampleRate, size_t samplesPerBlock, size_t channels) {
    jassert(channels >= 1);

    // lookahead buffer setup
    lookaheadBuffer = std::vector<RingBuffer<float>>();
    lookaheadBuffer.reserve(channels);
    for (int i = 0; i < channels; i++) {
        const auto latencyMs = vTree.isValid() ? getSliderMsFromTree<double>(vTree, Property::T_LOOKAHEAD_MS, Property::P_DISPLAY_VALUE) : 0.0;
        const auto maxLatencyMs = vTree.isValid() ? getSliderMsFromTree<double>(vTree, Property::T_LOOKAHEAD_MS, Property::P_MAX_VALUE) : 1000.0;

        const size_t size = sampleRate * (latencyMs/1000.0);
        auto newRing = RingBuffer<float>(sampleRate * (maxLatencyMs/1000.0)); // 10 safety padding, should actually just be the max amt of latency.
        newRing.setRelativeSize(size);
        lookaheadBuffer.push_back(newRing);
    }

    // resize curve multipliers and fill, set latency too.
    if (vTree.isValid())
    {
        updateCurveLength(getSliderMsFromTree<double>(vTree, Property::T_LENGTH_MS, Property::P_DISPLAY_VALUE));
        updateLookahead(getSliderMsFromTree<double>(vTree, Property::T_LOOKAHEAD_MS, Property::P_DISPLAY_VALUE));
    }
    else
    {
        resizeCurve(sampleRate / 2); // if tree is not valid
        setLatencySamples(0);
    }
}

void HentaiDuckProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // assuming stereo channels
    int channels = getMainBusNumInputChannels();
    busSettingsChanged(sampleRate, samplesPerBlock, channels); // didn't actually check if they changed, so this can be used as initializer too.
}

void HentaiDuckProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    if (totalNumInputChannels != this->numChannels || buffer.getNumSamples() != this->samplesPerBlock || getSampleRate() != sampleRate){
        this->samplesPerBlock = buffer.getNumSamples();
        this->numChannels = totalNumInputChannels;
        this->sampleRate = getSampleRate();
        busSettingsChanged(this->sampleRate, this->samplesPerBlock, this->numChannels);
    }

    // find positions to start the ducker
    amtTriggers = 0;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn(true))
        {
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
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int HentaiDuckProcessor::getCurrentProgram()
{
    return 0;
}

void HentaiDuckProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String HentaiDuckProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void HentaiDuckProcessor::changeProgramName(int index, const juce::String &newName)
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
bool HentaiDuckProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
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

juce::AudioProcessorEditor *HentaiDuckProcessor::createEditor()
{
    return new HentaiDuckEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void HentaiDuckProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ignoreUnused(destData);
    // You should use this method to store your parameters in the memory block.

    juce::MemoryOutputStream stream(destData, true);
    if (vTree.isValid())
    {
        vTree.writeToStream(stream);

#ifdef CMAKE_DEBUG
        vTree.createXML("C:/Dev/Juce Projects/HentaiDuck/getStateOutputTree.xml");
#endif
    }
}

void HentaiDuckProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
    // You should use this method to restore your parameters from this memory block, whose contents will have been created by the getStateInformation() call.

    vTree.copyFrom(data, sizeInBytes);
    if (!vTree.isValid())
        vTree.create(); // this shouldn't happen, unless there were breaking changes in an update
    updateCurveLength(getSliderMsFromTree<double>(vTree, Property::T_LENGTH_MS, Property::P_DISPLAY_VALUE));

#ifdef CMAKE_DEBUG
    vTree.createXML("C:/Dev/Juce Projects/HentaiDuck/setStateOutputTree.xml");
#endif
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new HentaiDuckProcessor();
}
