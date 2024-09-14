#include "ValueTreeManager.h"

// ======================= IMPLEMENTATION ==========================






template <typename E_ID>
std::optional<E_ID> subnite::vt::IDMap<E_ID>::getTypeFromID(const juce::Identifier &id) const
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

template <typename E_ID>
std::optional<juce::Identifier> subnite::vt::IDMap<E_ID>::getIDFromType(const E_ID &id) const
{
    auto it = map.find(id);
    if (it != map.end())
    {
        return it->second; // Return value if key is found
    }
    return std::nullopt;
}

template <typename E_ID>
subnite::vt::ValueTree<E_ID>::ValueTree()
    : subnite::vt::ValueTreeBase(), subnite::vt::IDMap<E_ID>()
{
}






// ==================================== BASE CLASS =================================






void subnite::vt::ValueTreeBase::addListener(juce::ValueTree::Listener *listener)
{
    vtRoot.addListener(listener);
}

void subnite::vt::ValueTreeBase::createXML(const juce::String &pathToFile) const
{
    auto xml = vtRoot.toXmlString();
    juce::XmlDocument doc{xml};
    juce::File file(pathToFile);
    file.create();
    file.replaceWithText(xml);
}

const juce::ValueTree& subnite::vt::ValueTreeBase::getRoot() const
{
    return vtRoot;
}

bool subnite::vt::ValueTreeBase::copyFrom(const void *data, int sizeInBytes)
{
    vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
    return vtRoot.isValid();
}

void subnite::vt::ValueTreeBase::writeToStream(juce::OutputStream &stream) const
{
    vtRoot.writeToStream(stream);
}

void subnite::vt::ValueTreeBase::setChild(const juce::Identifier &id, juce::ValueTree &toTree)
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
                parent.appendChild(toTree, &undoManager);
            }
        }
    }
    else
    {
        // it didn't exist so create one
        vtRoot.appendChild(toTree, &undoManager);
    }
}

// this doesn't work when using indexOf because it needs a ref, not a pointer. Can remake in the future, but don't use for now.
juce::ValueTree *subnite::vt::ValueTreeBase::getChildRecursive(const juce::Identifier &id, juce::ValueTree &tree)
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