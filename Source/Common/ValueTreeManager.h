#pragma once
#include <JuceHeader.h>

namespace subnite::vt {

#pragma region config

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
    LS_DISPLAY_VALUE,
    LS_MIN_VALUE,
    LS_MAX_VALUE,


    UNDEFINED
};

vt::Tree getTreeType(const juce::ValueTree& tree);
vt::Property getPropertyType(const juce::Identifier& property);
juce::Identifier getIDFromType(const vt::Tree& tree);
juce::Identifier getIDFromType(const vt::Property& property);

#pragma endregion config


class ValueTree {
public:
    // creates an empty, invalid tree
    ValueTree();
    ~ValueTree(){};

    // makes a new default tree. Setup the default root (vtRoot) and sub trees yourself.
    virtual void create() = 0;
    
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

    // returns the root
    const juce::ValueTree& getRoot() const;

    // removes the first child matching the type, and replacing it with tree if possible. This happens recursively though all children (not optimized)
    void setChild(const vt::Tree& type, juce::ValueTree& toTree);

    // recursively looks for a sub-tree matching the ID (not optimized)
    juce::ValueTree* getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree);
    
protected:
    juce::ValueTree vtRoot;
    juce::UndoManager undoManager{};
};



} // namespace