#include "ValueTreeManager.h"

#pragma region configuration

subnite::vt::Tree subnite::vt::getTreeType(const juce::ValueTree& tree) {
    const auto type = tree.getType().toString();

    if (type == "HentaiDuckRoot") return subnite::vt::Tree::ROOT;
    if (type == "CurveData") return subnite::vt::Tree::CD_CURVE_DATA;
    if (type == "NormalizedPoints") return subnite::vt::Tree::CD_NORMALIZED_POINTS;
    if (type == "Point") return subnite::vt::Tree::CD_POINT;
    if (type == "Coords") return subnite::vt::Tree::CD_COORDS;

    if (type == "LS_LengthMS") return subnite::vt::Tree::LS_LENGTH_MS;

    return subnite::vt::Tree::UNDEFINED;
}

juce::Identifier subnite::vt::getIDFromType(const subnite::vt::Tree& tree) {
    switch (tree) {
        case subnite::vt::Tree::ROOT : return {"HentaiDuckRoot"}; break;

        case subnite::vt::Tree::CD_CURVE_DATA : return {"CurveData"}; break;
        case subnite::vt::Tree::CD_NORMALIZED_POINTS : return {"NormalizedPoints"}; break;
        case subnite::vt::Tree::CD_POINT : return {"Point"}; break;
        case subnite::vt::Tree::CD_COORDS : return {"Coords"}; break;

        case subnite::vt::Tree::LS_LENGTH_MS : return {"LS_LengthMS"}; break;

        default: return {"Undefined"}; break;
    }
}

subnite::vt::Property subnite::vt::getPropertyType(const juce::Identifier& property) {
    using pr = subnite::vt::Property;
    const auto prop = property.toString();

    if (prop == "Power") return pr::CD_POWER;
    if (prop == "MaxAbsolutePower") return pr::CD_MAX_ABSOLUTE_POWER;
    if (prop == "Size") return pr::CD_SIZE;
    if (prop == "X") return pr::CD_X;
    if (prop == "Y") return pr::CD_Y;

    if (prop == "LS_isMS") return pr::LS_IS_MS;
    if (prop == "LS_RawNormalizedValue") return pr::LS_RAW_NORMALIZED_VALUE;
    if (prop == "LS_DisplayValue") return pr::LS_DISPLAY_VALUE;
    if (prop == "LS_MinValue") return pr::LS_MIN_VALUE;
    if (prop == "LS_MaxValue") return pr::LS_MAX_VALUE;

    return pr::UNDEFINED;
}

juce::Identifier subnite::vt::getIDFromType(const subnite::vt::Property& property) {
    using prop = subnite::vt::Property;
    switch (property) {
        // CurveDisplay
        case prop::CD_POWER : return {"Power"}; break;
        case prop::CD_MAX_ABSOLUTE_POWER : return {"MaxAbsolutePower"}; break;
        case prop::CD_SIZE : return {"Size"}; break;
        case prop::CD_X : return {"X"}; break;
        case prop::CD_Y : return {"Y"}; break;
        
        // lengthMs
        case prop::LS_IS_MS : return {"LS_isMS"}; break;
        case prop::LS_RAW_NORMALIZED_VALUE : return {"LS_RawNormalizedValue"}; break;
        case prop::LS_DISPLAY_VALUE : return {"LS_DisplayValue"}; break;
        case prop::LS_MIN_VALUE : return {"LS_MinValue"}; break;
        case prop::LS_MAX_VALUE : return {"LS_MaxValue"}; break;

        default: return {"Undefined"}; break;
    }
}

#pragma endregion configuration

subnite::vt::ValueTree::ValueTree() {

}

void subnite::vt::ValueTree::addListener(juce::ValueTree::Listener* listener) {
    vtRoot.addListener(listener);
}

void subnite::vt::ValueTree::createXML(const juce::String& pathToFile) const {
    auto xml = vtRoot.toXmlString();
    juce::XmlDocument doc{xml};
    juce::File file(pathToFile);
    file.create();
    file.replaceWithText(xml);
}

const::juce::ValueTree& subnite::vt::ValueTree::getRoot() const {
    return vtRoot;
}

bool subnite::vt::ValueTree::copyFrom(const void* data, int sizeInBytes){
    vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
    return vtRoot.isValid();
}

void subnite::vt::ValueTree::writeToStream(juce::OutputStream& stream) const {
    vtRoot.writeToStream(stream);
}

void subnite::vt::ValueTree::setChild(const subnite::vt::Tree& type, juce::ValueTree& toTree) {
    const auto id = getIDFromType(type);
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
juce::ValueTree* subnite::vt::ValueTree::getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree) {
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