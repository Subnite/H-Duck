#pragma once
#include <JuceHeader.h>
#include <optional>
#include <unordered_map>

namespace subnite::vt
{

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

        // returns the Enum associated with the ID if it exists.
        std::optional<E_ID> getTypeFromID(const juce::Identifier &id) const
        {
            auto it = std::find_if(map.begin(), map.end(), [&id](const std::pair<E_ID, juce::Identifier> &pair)
                                   {
                                       return pair.second == id; // Find the pair where value matches
                                   });

            if (it != map.end())
            {
                return it->first; // Return key if value is found
            }
            return std::nullopt; // Return an empty optional if value is not found
        }

        // returns the ID associated with the enum type if it exists.
        std::optional<juce::Identifier> getIDFromType(const E_ID &property) const
        {
            auto it = map.find(property);
            if (it != map.end())
            {
                return it->second; // Return value if key is found
            }
            return std::nullopt;
        }
    };

    /*
        The base class for a ValueTree. All functionality is here except for mappings from Enum to ID.
    */
    class ValueTreeBase
    {
    public:
        ValueTreeBase() {};
        ~ValueTreeBase() {};

        // makes a new default tree. Setup the default root (vtRoot) and sub trees yourself.
        virtual void create() = 0;

        // returns if the underlying root tree is valid
        bool isValid()
        {
            return vtRoot.isValid();
        }

        // adds a listener to the root tree
        void addListener(juce::ValueTree::Listener *listener)
        {
            vtRoot.addListener(listener);
        }

        // returns the undo manager used for all things
        juce::UndoManager *getUndoManager()
        {
            return &undoManager;
        }

        // replaces the current tree from the tree found in the data if possible.
        bool copyFrom(const void *data, int sizeInBytes)
        {
            vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
            return vtRoot.isValid();
        }

        // writes the current tree to an output stream
        void writeToStream(juce::OutputStream &stream) const
        {
            vtRoot.writeToStream(stream);
        }

        // writes the root tree structure to an XML file. Example path: "C:/Dev/tree.xml"
        void createXML(const juce::String &pathToFile) const
        {
            auto xml = vtRoot.toXmlString();
            juce::XmlDocument doc{xml};
            juce::File file(pathToFile);
            file.create();
            file.replaceWithText(xml);
        }

        // returns the root
        const juce::ValueTree &getRoot() const { return vtRoot; }

        // recursively looks for a sub-tree matching the ID (not optimized)
        juce::ValueTree *getChildRecursive(const juce::Identifier &id, juce::ValueTree &tree)
        {
            if (tree.hasType(id))
                return &tree;

            juce::ValueTree *oldTree = nullptr;

            // auto& would be better but this might be fine
            for (size_t childIdx = 0; childIdx < tree.getNumChildren(); childIdx++)
            {
                auto child = tree.getChild(childIdx);
                auto name = child.getType().toString();
                std::cout << "\n checking: " << name << "\n";
                if (!child.isValid())
                    break; // annoying, might find but be invalid

                if (child.hasType(id))
                {
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

        // removes the first child matching the type, and replacing it with tree if possible, otherwise make a new child. This happens recursively through all children (not optimized)
        void setChild(const juce::Identifier &id, juce::ValueTree &toTree)
        {
            auto oldTree = vtRoot.getChildWithName(id);
            // auto oldTree = getChildRecursive(id, vtRoot);

            if (/*oldTree != nullptr &&*/ oldTree.isValid())
            {
                auto parent = oldTree.getParent();
                if (parent.isValid())
                {
                    auto idx = parent.indexOf(oldTree);
                    if (idx >= 0)
                    {
                        parent.removeChild(idx, &undoManager);
                    }
                    parent.appendChild(toTree, &undoManager);
                }
            }
            else
            {
                // it didn't exist so create one
                vtRoot.appendChild(toTree, &undoManager);
            }
        }

    protected:
        juce::ValueTree vtRoot;
        juce::UndoManager undoManager{};
    };

} // namespace
