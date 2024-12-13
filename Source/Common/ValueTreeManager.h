/**
 * @file ValueTreeManager.h
 * @author Subnite
 * @brief a valuetree manager based on juce::ValueTree to save and load params.
 *
 */

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
        /**
         * The map used for connecting an enum to id's
         */
        std::unordered_map<E_ID, juce::Identifier> map;

        /**
         * @brief creates the mappings from E_ID to juce::Identifier
         *
         * you can access the this->map variable, and are supposed to fill it with all possible enums.
         * 
         * example code:
         * @code
         * void setupMap() override {
         *     this->map[E_ID::ROOT] = juce::Identifier{"RootName"};
         *     this->map[E_ID::DELAY_SLIDER] = juce::Identifier{"DelaySlider"};
         * }
         * @endcode
         */
        virtual void setupMap() = 0;

    public:
        /**
         * Default IDMap constructor, doesn't do any setup.
         *
         * CALL setupMap() MANUALLY ON CONSTRUCTOR!!
         * The juce vst3 helper crashes when this abstract class tries to call setupMap(), so do it yourself!
         */
        IDMap()
        {
        }

        /**
         * get the enum linked with the id
         * @param id The identifier linked with an enum.
         * @return The E_ID::enum linked with id, or std::nullopt if it wasn't found.
         */
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

        /**
         * get the ID associated with mapped Enum
         *
         * @param property The enum that associates to a juce::Identifier.
         * @return The ID linked with the enum, or std::nullopt if not found.
         */
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
     *@brief The base class for a ValueTree.
     *
     * All functionality is here except for mappings from Enum to ID.
    */
    class ValueTreeBase
    {
    public:
    /** Default constructor, doesn't do anything. */
    ValueTreeBase() {};
    /** Default destructor, doesn't do anything. */
    ~ValueTreeBase() {}; 

    // makes a new default tree. Setup the default root (vtRoot) and sub trees yourself.
    /**
         * Creates a default tree.
         *
         * This would only be called by the user. This class doesn't call it since it's purely virtual.
         *
         * Setup the default root (vtRoot) and sub trees yourself!
         * 
         * Example code:
         * @code
         * void create() override {
         *     this->vtRoot = juce::ValueTree{"RootTree"};
         *     juce::ValueTree sliderTree{"SliderTree"};
         *
         *     sliderTree.setProperty("SliderValue", 0.7, &this->undoManager);
         *
         *     vtRoot.appendChild(sliderTree, &this->undoManager);
         * }
         * @endcode
         */
    virtual void create() = 0;

    /** @return If the root tree is valid. */
    bool isValid()
    {
        return vtRoot.isValid();
    }

    /** Adds a listener to the root tree. */
    void addListener(juce::ValueTree::Listener *listener)
    {
        vtRoot.addListener(listener);
    }

    /** @return The undo manager used for all things. Could be nullptr */
    juce::UndoManager *getUndoManager()
    {
        return &undoManager;
    }

    /** Replaces the current tree with the tree found in the data. @return true when the tree is valid.*/
    bool copyFrom(const void *data, int sizeInBytes)
    {
        vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
        return vtRoot.isValid();
    }

    /** Writes the current tree to an output stream. 
     *  @param stream The stream to write the data to.
     * */
    void writeToStream(juce::OutputStream &stream) const
    {
        vtRoot.writeToStream(stream);
    }

    /** Writes the root tree structure to an XML file. Example path: "C:/Dev/tree.xml" */
    void createXML(const juce::String &pathToFile) const
    {
        auto xml = vtRoot.toXmlString();
        juce::XmlDocument doc{xml};
        juce::File file(pathToFile);
        file.create();
        file.replaceWithText(xml);
    }

    /** @return The root vtRoot */
    const juce::ValueTree &getRoot() const { return vtRoot; }

    /** Recursively looks for a sub-tree matching the ID (not optimized)
     *  @param id The id to look for.
     *  @param tree The parent to start searching from.
     *  @return The tree if found, else nullptr.
     *  !!! This function doesn't work right now !!!
     * */
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

    /** Removes the first child matching the type, and replaces it with tree if possible. Otherwise it makes a new child.
     *
     * This happens recursively through all children (not optimized) !!! Currently only looks at the direct children of the tree !!!
     *
     */
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
    /** The root value tree. */
    juce::ValueTree vtRoot;
    /** The undomanager associated with the vtRoot root tree. */
    juce::UndoManager undoManager{};
};

} // namespace
