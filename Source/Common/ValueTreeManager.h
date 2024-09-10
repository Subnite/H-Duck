#pragma once
#include <JuceHeader.h>

namespace duck::vt {

enum class Tree {
    ROOT,
    
    // CurveDisplay
    CD_CURVE_DATA,
    CD_NORMALIZED_POINTS,
    CD_POINT,
    CD_COORDS,

    // Length Slider
    LS_LENGTH_MS,

    UNDEFINED
};

enum class Property {
    // CurveDisplay
    CD_POWER,
    CD_MAX_ABSOLUTE_POWER,
    CD_SIZE,
    CD_X,
    CD_Y,

    // Length Slider
    LS_IS_MS,
    LS_RAW_NORMALIZED_VALUE,
    LS_MIN_VALUE,
    LS_MAX_VALUE,


    UNDEFINED
};

class ValueTree {
public:
    // creates an empty, invalid tree
    ValueTree();
    ~ValueTree(){};

    static duck::vt::Tree getTreeType(const juce::ValueTree& tree);
    static duck::vt::Property getPropertyType(const juce::Identifier& property);
    static juce::Identifier getIDFromType(const duck::vt::Tree& tree);
    static juce::Identifier getIDFromType(const duck::vt::Property& property);
    
    // makes a new root tree
    void create();
    
    // returns if the underlying root tree is valid
    bool isValid() {return vtRoot.isValid();}
    // adds a listener to the root tree
    void addListener(juce::ValueTree::Listener* listener);
    // returns the undo manager used for all things
    juce::UndoManager* getUndoManager() {return &undoManager;}
    
    // replaces the current tree from the tree found in the data if possible.
    bool copyFrom(const void* data, int sizeInBytes);
    // writes the current tree to an output stream
    void writeToStream(juce::OutputStream& stream) const;
    // writes the root tree structure to an XML file. Example path: "C:/Dev/tree.xml"
    void createXML(const juce::String& pathToFile) const;

    // functions for the curve class
    void addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size);
    // returns the root
    const juce::ValueTree& getRoot() const;
    // removes the first child matching the type, and replacing it with tree if possible. This happens recursively though all children (not optimized)
    void setChild(const duck::vt::Tree& type, juce::ValueTree& toTree);
    // recursively looks for a sub-tree matching the ID (not optimized)
    juce::ValueTree* getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree);
    
private:
    juce::ValueTree vtRoot;
    juce::UndoManager undoManager{};
};



} // namespace
