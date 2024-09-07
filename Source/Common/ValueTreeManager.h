#pragma once
#include <JuceHeader.h>

namespace duck::vt {

enum class Tree {
    ROOT,

    CURVE_DATA,
    NORMALIZED_POINTS,
    POINT,
    COORDS,

    UNDEFINED
};

enum class Property {
    POWER,
    MAX_ABSOLUTE_POWER,
    SIZE,
    X,
    Y,

    UNDEFINED
};

class ValueTree {
public:
    ValueTree();
    ~ValueTree(){};

    static duck::vt::Tree getTreeType(const juce::ValueTree& tree);
    static duck::vt::Property getPropertyType(const juce::Identifier& property);
    static juce::Identifier getIDFromType(const duck::vt::Tree& tree);
    static juce::Identifier getIDFromType(const duck::vt::Property& property);
    
    // makes a new root tree
    void create();

    bool isValid() {return vtRoot.isValid();}
    void addListener(juce::ValueTree::Listener* listener);
    juce::UndoManager* getUndoManager() {return &undoManager;}

    bool copyFrom(const void* data, int sizeInBytes);
    void writeToStream(juce::OutputStream& stream) const;
    // writes the root tree structure to an XML file. Example path: "C:/Dev/tree.xml"
    void createXML(const juce::String& pathToFile) const;

    // functions for the curve class
    void addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size);
    const juce::ValueTree& getRoot() const;
    void setChild(const duck::vt::Tree& type, juce::ValueTree& toTree);
    juce::ValueTree* getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree);
    
private:
    juce::ValueTree vtRoot;
    juce::UndoManager undoManager{};
};



} // namespace
