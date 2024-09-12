#pragma once
#include <JuceHeader.h>
#include <optional>
#include <unordered_map>

namespace subnite::vt {

template <typename E_ID>
class IDMap {
protected:
    std::unordered_map<E_ID, juce::Identifier> map;
    /*
    you can access the this->map variable, and are supposed to fill it with all possible enums.

    example:
    `this->map[Enum::ROOT] = juce::Identifier{"nameForRoot"};`
    */
    virtual void setupMap() = 0;
public:
    IDMap() {
        // setupMap(); // this crashes the juce vst3 helper since it's not technically defined here.
    }
    std::optional<E_ID> getTypeFromID(const juce::Identifier& id) const;
    std::optional<juce::Identifier> getIDFromType(const E_ID& property) const;
};

/*
    Make sure to override `void setupMap()` AND call it in the constructor!

    You need to provide an enum class which represents all the trees (including root) and sub trees, and the properties that you will use.

    after that, just make sure to override `create()` which should create the default tree, and `setupMap()` which sets up all the bindings for getID and getType functions.
*/
template <typename E_ID>
class ValueTree : public subnite::vt::IDMap<E_ID> {
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
    void setChild(const E_ID& type, juce::ValueTree& toTree);

    // recursively looks for a sub-tree matching the ID (not optimized)
    juce::ValueTree* getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree);
    
protected:
    juce::ValueTree vtRoot;
    juce::UndoManager undoManager{};
};



} // namespace


template <typename E_ID>
std::optional<E_ID> subnite::vt::IDMap<E_ID>::getTypeFromID(const juce::Identifier& id) const {
    auto it = std::find_if(map.begin(), map.end(), [&id](const std::pair<E_ID, juce::Identifier>& pair) {
        return pair.second == id;  // Find the pair where value matches
    });

    if (it != map.end()) {
        return it->first;  // Return key if value is found
    }
    return std::nullopt;  // Return an empty optional if value is not found
}

template <typename E_ID>
std::optional<juce::Identifier> subnite::vt::IDMap<E_ID>::getIDFromType(const E_ID& id) const {
    auto it = map.find(id);
    if (it != map.end()) {
        return it->second;  // Return value if key is found
    }
    return std::nullopt;
}

template <typename E_ID>
subnite::vt::ValueTree<E_ID>::ValueTree() 
: subnite::vt::IDMap<E_ID>()
{
}

template <typename E_ID>
void subnite::vt::ValueTree<E_ID>::addListener(juce::ValueTree::Listener* listener) {
    vtRoot.addListener(listener);
}

template <typename E_ID>
void subnite::vt::ValueTree<E_ID>::createXML(const juce::String& pathToFile) const {
    auto xml = vtRoot.toXmlString();
    juce::XmlDocument doc{xml};
    juce::File file(pathToFile);
    file.create();
    file.replaceWithText(xml);
}

template <typename E_ID>
const::juce::ValueTree& subnite::vt::ValueTree<E_ID>::getRoot() const {
    return vtRoot;
}

template <typename E_ID>
bool subnite::vt::ValueTree<E_ID>::copyFrom(const void* data, int sizeInBytes){
    vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
    return vtRoot.isValid();
}

template <typename E_ID>
void subnite::vt::ValueTree<E_ID>::writeToStream(juce::OutputStream& stream) const {
    vtRoot.writeToStream(stream);
}

template <typename E_ID>
void subnite::vt::ValueTree<E_ID>::setChild(const E_ID& type, juce::ValueTree& toTree) {
    const auto id = getIDFromType(type).value_or(juce::Identifier{"undefined"});
    auto oldTree = vtRoot.getChildWithName(id);
    //auto oldTree = getChildRecursive(id, vtRoot);

    if (/*oldTree != nullptr &&*/ oldTree.isValid()){
        auto parent = oldTree.getParent();
        if (parent.isValid()){
            auto idx = parent.indexOf(oldTree);
            if (idx >= 0){
                parent.removeChild(idx, &undoManager);
                parent.appendChild(toTree, &undoManager);
            }
        }
    } else {
        // it didn't exist so create one
        vtRoot.appendChild(toTree, &undoManager);
    }
}

// this doesn't work when using indexOf because it needs a ref, not a pointer. Can remake in the future, but don't use for now.
template <typename E_ID>
juce::ValueTree* subnite::vt::ValueTree<E_ID>::getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree) {
    if (tree.hasType(id)) return &tree;

    juce::ValueTree* oldTree = nullptr;

    // auto& would be better but this might be fine
    for (size_t childIdx = 0; childIdx < tree.getNumChildren(); childIdx++) {
        auto child = tree.getChild(childIdx);
        auto name = child.getType().toString();
        std::cout << "\n checking: " << name << "\n";
        if (!child.isValid()) break; // annoying, might find but be invalid

        if (child.hasType(id)) {
            oldTree = &child;
            return oldTree;
        } 
    }

    // check children of the children
    // if (oldTree == nullptr){
    //     for (size_t childIdx = 0; childIdx < tree.getNumChildren(); childIdx++) {
    //         if (oldTree != nullptr) return oldTree;
    //         auto child = tree.getChild(childIdx);
    //         auto name = child.getType().toString();
    //         std::cout << "checking: " << name << "\n";

    //         for (size_t childIdx2 = 0; childIdx2 < tree.getNumChildren(); childIdx2++) {
    //             auto child2 = child.getChild(childIdx2);
    //             auto name = child2.getType().toString();
    //             std::cout << "checking: " << name << "\n";
    //             if (!child2.isValid()) break;

    //             oldTree = getChildRecursive(id, child2);
    //             if (oldTree != nullptr) return oldTree;
    //         }

    //     }
    // }

    return oldTree;
}