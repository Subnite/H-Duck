#pragma once
#include <JuceHeader.h>
#include <optional>
#include <unordered_map>

namespace subnite::vt {

    template <typename E_ID>
    class IDMap
    {
    protected:
        std::unordered_map<E_ID, juce::Identifier> map;
        /*
        you can access the this->map variable, and are supposed to fill it with all possible enums.
        example:
        `this->map[Enum::ROOT] = juce::Identifier{"nameForRoot"};`
        */
        virtual void setupMap() = 0;

    public:
        IDMap()
        {
            // setupMap(); // this crashes the juce vst3 helper since it's not technically defined here.
        }
        std::optional<E_ID> getTypeFromID(const juce::Identifier &id) const;
        std::optional<juce::Identifier> getIDFromType(const E_ID &property) const;
    };

    /*
        The base class for a ValueTree. All functionality is here except for mappings from Enum to ID.
    */
    class ValueTreeBase
    {
    public:
        ValueTreeBase(){};
        ~ValueTreeBase(){};

        // makes a new default tree. Setup the default root (vtRoot) and sub trees yourself.
        virtual void create() = 0;

        // returns if the underlying root tree is valid
        bool isValid() { return vtRoot.isValid(); }

        // adds a listener to the root tree
        void addListener(juce::ValueTree::Listener *listener);

        // returns the undo manager used for all things
        juce::UndoManager *getUndoManager() { return &undoManager; }

        // replaces the current tree from the tree found in the data if possible.
        bool copyFrom(const void *data, int sizeInBytes);

        // writes the current tree to an output stream
        void writeToStream(juce::OutputStream &stream) const;

        // writes the root tree structure to an XML file. Example path: "C:/Dev/tree.xml"
        void createXML(const juce::String &pathToFile) const;

        // returns the root
        const juce::ValueTree &getRoot() const;

        // recursively looks for a sub-tree matching the ID (not optimized)
        juce::ValueTree *getChildRecursive(const juce::Identifier &id, juce::ValueTree &tree);

        // removes the first child matching the type, and replacing it with tree if possible. This happens recursively though all children (not optimized)
        void setChild(const juce::Identifier &id, juce::ValueTree &toTree);

    protected:
        juce::ValueTree vtRoot;
        juce::UndoManager undoManager{};
    };

    /*
        Make sure to override `void setupMap()` AND call it in the constructor!

        You need to provide an enum class which represents all the trees (including root) and sub trees, and the properties that you will use.

        after that, just make sure to override `create()` which should create the default tree, and `setupMap()` which sets up all the bindings for getID and getType functions.
    */
    template <typename E_ID>
    class ValueTree : public subnite::vt::ValueTreeBase, public subnite::vt::IDMap<E_ID>
    {
    public:
        // creates an empty, invalid tree
        ValueTree();
        ~ValueTree() {};
    };

} // namespace






