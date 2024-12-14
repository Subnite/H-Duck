#pragma once
#include <JuceHeader.h>
#include "PngGifFrameViewer.h"

namespace duck {
enum class GifSyncResult {
    RootNotFound,
    GifNotFound,
    Success
};

class GifViewer : public juce::Component, public juce::Timer, public juce::ChangeListener {
public:
    GifViewer(const std::string& fileName, juce::ChangeBroadcaster* sidechainTriggerBroadcast = nullptr)
    : fileName(fileName)
    {
        auto syncResult = syncJSONData();
        jassert(syncResult == GifSyncResult::Success);
        gif = std::make_unique<PngGifFrameViewer>(getUserGif(fileName), rows, columns, totalAmount);
        idleAnimation();
        if (sidechainTriggerBroadcast != nullptr) {
            broadcaster = sidechainTriggerBroadcast;
            broadcaster->addChangeListener(this);
        }

    }
    ~GifViewer() {
        stopTimer();
        if (broadcaster != nullptr) broadcaster->removeChangeListener(this);
    }

    void paint(juce::Graphics &g) override {
        auto img = gif->getFrame(currentFrameIdx);
        g.drawImage(img, getLocalBounds().toFloat(), RectanglePlacement(RectanglePlacement::centred));
    }

    void timerCallback() override {
        if (isIdle) {
            // we can assume the timer is on idle fps
            handleFrameIndex(idleFrameRange);
            
        } else {
            // we can assume the timer is on trigger fps
            handleFrameIndex(triggerFrameRange);
        }

        // check for frame change and repaint
        if (previousFrameIdx != currentFrameIdx) repaint();
        previousFrameIdx = currentFrameIdx;

        // check for end of trigger
        bool triggerMaybeCompleted = bounceBack ? isReversed && !isIdle : !isReversed && !isIdle;
        if (triggerMaybeCompleted){
            size_t lastIndex = bounceBack ? triggerFrameRange.getStart() : triggerFrameRange.getEnd();
            if (currentFrameIdx == lastIndex) idleAnimation();
        }
    }

    void changeListenerCallback (ChangeBroadcaster *source) {
        triggerAnimation();
    }

    static File getUserResources() {
        auto fileDir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
        auto duckDir = fileDir.getChildFile("Subnite Plugins").getChildFile("H-Duck");
        auto resourceDir = duckDir.getChildFile("Resources");
        if (!resourceDir.exists()) {
            auto res = resourceDir.createDirectory();
            jassert(res.wasOk());
        }
        return resourceDir;
    }

    static File getGifJsonFile() {
        auto resourceDir = getUserResources();
        auto json = resourceDir.getChildFile("gifs.json");
        if (!json.exists()) createDefaultJSON();
        return json;
    }
private:
    static File getUserGif(const std::string& fileName) {
        auto resourceDir = getUserResources();
        auto fileDir = resourceDir.getChildFile(fileName);
        jassert(fileDir.exists());
        return fileDir;
    }


    void handleFrameIndex(const juce::Range<size_t>& range) {
        if (range.getLength() > 0) {
            if (currentFrameIdx == range.getStart()) isReversed = false;
            else if (currentFrameIdx == range.getEnd()) isReversed = true;

            bool rangeIsInverted = range.getStart() > range.getEnd(); // the order of the range itself is inverted
            int add = 1;
            if (rangeIsInverted) add *= -1;
            if (isReversed) add *= -1;
            currentFrameIdx += add;

        }
    }


    // assumes that all necessary values have defaults from constructor already
    GifSyncResult syncJSONData() {
        // 1. get or create the resources required for the gif
        auto json = getGifJsonFile();

        // 2. get full settings json object
        var rootVar = JSON::parse(json);
        if (!rootVar.isVoid());
        DynamicObject* rootObject = rootVar.getDynamicObject();
        if (rootObject == nullptr) return GifSyncResult::RootNotFound;

        // 3. get the gif object in question
        auto gifVar = rootObject->getProperty(juce::Identifier{fileName});
        jassert(!gifVar.isVoid());
        DynamicObject* gifObject = gifVar.getDynamicObject();
        if (gifObject == nullptr) return GifSyncResult::GifNotFound;

        // 4. get the properties
        tryAssignInt<size_t>(gifObject, "rows", rows);
        tryAssignInt<size_t>(gifObject, "columns", columns);
        tryAssignInt<size_t>(gifObject, "total_frames", totalAmount);

        size_t idleX{idleFrameRange.getStart()}, idleY{idleFrameRange.getEnd()}, triggerX{triggerFrameRange.getStart()}, triggerY{triggerFrameRange.getEnd()};
        tryAssignInt<size_t>(gifObject, "idle_start_frame_index", idleX);
        tryAssignInt<size_t>(gifObject, "idle_end_frame_index", idleY);
        tryAssignInt<size_t>(gifObject, "trigger_start_index", triggerX);
        tryAssignInt<size_t>(gifObject, "trigger_end_index", triggerY);

        idleFrameRange = juce::Range<size_t>{idleX, idleY};
        triggerFrameRange = juce::Range<size_t>{triggerX, triggerY};

        tryAssignDouble<float>(gifObject, "idle_fps", idleFPS);
        tryAssignDouble<float>(gifObject, "trigger_fps", triggerFPS);

        auto bounceVar = gifObject->getProperty(juce::Identifier{"bounce_back"});
        if (!bounceVar.isVoid() && bounceVar.isBool()) {
            bool value = bounceVar.operator bool();
            bounceBack = value;
        }

        return GifSyncResult::Success;
    }

    template<typename T>
    static void tryAssignInt(juce::DynamicObject* obj, const char* name, T& assignTo) {
        auto var = obj->getProperty(juce::Identifier{name});
        if (!var.isVoid() && var.isInt()) {
            int value = var.operator int();
            assignTo = static_cast<T>(value);
        }
    }

    template<typename T>
    static void tryAssignDouble(juce::DynamicObject* obj, const char* name, T& assignTo) {
        auto var = obj->getProperty(juce::Identifier{name});
        if (!var.isVoid() && (var.isInt() || var.isDouble() || var.isInt64())) {
            float value = var.operator float();
            assignTo = static_cast<T>(value);
        }
    }

    void idleAnimation(){
        currentFrameIdx = idleFrameRange.getStart();
        isIdle = true;
        startTimer(1000.f / idleFPS);
    }
    void triggerAnimation() {
        currentFrameIdx = triggerFrameRange.getStart();
        isIdle = false;
        startTimer(1000.f / triggerFPS);
    }

    static void createDefaultJSON(){};

private:
    size_t rows{1}, columns{1}, totalAmount{1};
    std::unique_ptr<PngGifFrameViewer> gif;
    const std::string fileName;
    size_t currentFrameIdx = 0;
    size_t previousFrameIdx = 0;
    bool isIdle = true;
    bool isReversed = false;
    juce::ChangeBroadcaster* broadcaster = nullptr;


    juce::Range<size_t> idleFrameRange = juce::Range<size_t>(0, 1);
    juce::Range<size_t> triggerFrameRange = juce::Range<size_t>(2, 6);
    bool bounceBack = true; /** after the trigger, will then bounce back from end to beginning before going to idle */
    float idleFPS{1}, triggerFPS{7};
};


}