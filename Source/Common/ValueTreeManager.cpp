#include "ValueTreeManager.h"

duck::vt::Tree duck::vt::ValueTree::getTreeType(const juce::ValueTree& tree) {
    const auto type = tree.getType().toString();

    if (type == "HentaiDuckRoot") return duck::vt::Tree::ROOT;
    if (type == "CurveData") return duck::vt::Tree::CD_CURVE_DATA;
    if (type == "NormalizedPoints") return duck::vt::Tree::CD_NORMALIZED_POINTS;
    if (type == "Point") return duck::vt::Tree::CD_POINT;
    if (type == "Coords") return duck::vt::Tree::CD_COORDS;

    if (type == "LS_LengthMS") return duck::vt::Tree::LS_LENGTH_MS;

    return duck::vt::Tree::UNDEFINED;
}


juce::Identifier duck::vt::ValueTree::getIDFromType(const duck::vt::Tree& tree) {
    switch (tree) {
        case duck::vt::Tree::ROOT : return {"HentaiDuckRoot"}; break;

        case duck::vt::Tree::CD_CURVE_DATA : return {"CurveData"}; break;
        case duck::vt::Tree::CD_NORMALIZED_POINTS : return {"NormalizedPoints"}; break;
        case duck::vt::Tree::CD_POINT : return {"Point"}; break;
        case duck::vt::Tree::CD_COORDS : return {"Coords"}; break;

        case duck::vt::Tree::LS_LENGTH_MS : return {"LS_LengthMS"}; break;

        default: return {"Undefined"}; break;
    }
}

duck::vt::Property duck::vt::ValueTree::getPropertyType(const juce::Identifier& property) {
    const auto prop = property.toString();

    if (prop == "Power") return duck::vt::Property::CD_POWER;
    if (prop == "MaxAbsolutePower") return duck::vt::Property::CD_MAX_ABSOLUTE_POWER;
    if (prop == "Size") return duck::vt::Property::CD_SIZE;
    if (prop == "X") return duck::vt::Property::CD_X;
    if (prop == "Y") return duck::vt::Property::CD_Y;

    if (prop == "LS_isMS") return duck::vt::Property::LS_IS_MS;
    if (prop == "LS_RawNormalizedValue") return duck::vt::Property::LS_RAW_NORMALIZED_VALUE;
    if (prop == "LS_DisplayValue") return duck::vt::Property::LS_DISPLAY_VALUE;
    if (prop == "LS_MinValue") return duck::vt::Property::LS_MIN_VALUE;
    if (prop == "LS_MaxValue") return duck::vt::Property::LS_MAX_VALUE;

    return duck::vt::Property::UNDEFINED;
}

juce::Identifier duck::vt::ValueTree::getIDFromType(const duck::vt::Property& property) {
    switch (property) {
        // CurveDisplay
        case duck::vt::Property::CD_POWER : return {"Power"}; break;
        case duck::vt::Property::CD_MAX_ABSOLUTE_POWER : return {"MaxAbsolutePower"}; break;
        case duck::vt::Property::CD_SIZE : return {"Size"}; break;
        case duck::vt::Property::CD_X : return {"X"}; break;
        case duck::vt::Property::CD_Y : return {"Y"}; break;
        
        // lengthMs
        case duck::vt::Property::LS_IS_MS : return {"LS_isMS"}; break;
        case duck::vt::Property::LS_RAW_NORMALIZED_VALUE : return {"LS_RawNormalizedValue"}; break;
        case duck::vt::Property::LS_DISPLAY_VALUE : return {"LS_DisplayValue"}; break;
        case duck::vt::Property::LS_MIN_VALUE : return {"LS_MinValue"}; break;
        case duck::vt::Property::LS_MAX_VALUE : return {"LS_MaxValue"}; break;

        default: return {"Undefined"}; break;
    }
}

duck::vt::ValueTree::ValueTree() {

}

void duck::vt::ValueTree::create() {
    using vt = duck::vt::ValueTree;
    using tree = duck::vt::Tree;
    using prop = duck::vt::Property;

    // create new one
    vtRoot = juce::ValueTree{getIDFromType(tree::ROOT)};
    juce::ValueTree curve{getIDFromType(tree::CD_CURVE_DATA)};
    juce::ValueTree points{getIDFromType(tree::CD_NORMALIZED_POINTS)};
    curve.appendChild(points, &undoManager);
    vtRoot.appendChild(curve, &undoManager);

    addPoint({0.f, 0.f}, -10.f, 50.f, 20.f);
    addPoint({0.002f, 1.f}, 8.f, 50.f, 20.f);
    addPoint({0.3f, 0.3f}, -8.f, 50.f, 20.f);
    addPoint({0.5f, 0.f}, 0.f, 50.f, 20.f);
    addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);
    
    // right now the length slider tree is created by the slider class, changing this would also work, but then add all possible values.
    juce::ValueTree lengthSliderTree{getIDFromType(tree::LS_LENGTH_MS)};
    lengthSliderTree.setProperty(vt::getIDFromType(prop::LS_IS_MS), true, nullptr);
    lengthSliderTree.setProperty(vt::getIDFromType(prop::LS_DISPLAY_VALUE), 50.0, nullptr);
    lengthSliderTree.setProperty(vt::getIDFromType(prop::LS_MIN_VALUE), 10.0, nullptr);
    lengthSliderTree.setProperty(vt::getIDFromType(prop::LS_MAX_VALUE), 2000.0, nullptr);
    lengthSliderTree.setProperty(vt::getIDFromType(prop::LS_RAW_NORMALIZED_VALUE), 0.5, nullptr);

    vtRoot.appendChild(lengthSliderTree, &undoManager);
}

void duck::vt::ValueTree::addListener(juce::ValueTree::Listener* listener) {
    vtRoot.addListener(listener);
}

void duck::vt::ValueTree::createXML(const juce::String& pathToFile) const {
    auto xml = vtRoot.toXmlString();
    juce::XmlDocument doc{xml};
    juce::File file(pathToFile);
    file.create();
    file.replaceWithText(xml);
}

void duck::vt::ValueTree::addPoint(const juce::Point<float>& coords, const float& power, const float& maxAbsPower, const float& size) {
    using vt = duck::vt::ValueTree;
    using tree = duck::vt::Tree;
    using prop = duck::vt::Property;

    auto curve = vtRoot.getOrCreateChildWithName(getIDFromType(tree::CD_CURVE_DATA), &undoManager);
    auto points = curve.getOrCreateChildWithName(getIDFromType(tree::CD_NORMALIZED_POINTS), &undoManager);
    if (!points.isValid()) return;
    juce::ValueTree point{getIDFromType(tree::CD_POINT)};
    juce::ValueTree xy{getIDFromType(tree::CD_COORDS)};

    xy.setProperty(getIDFromType(prop::CD_X), {double(coords.x)}, &undoManager);
    xy.setProperty(getIDFromType(prop::CD_Y), {double(coords.y)}, &undoManager);
    point.setProperty(getIDFromType(prop::CD_POWER), {double(power)}, &undoManager);
    point.setProperty(getIDFromType(prop::CD_MAX_ABSOLUTE_POWER), {double(maxAbsPower)}, &undoManager);
    point.setProperty(getIDFromType(prop::CD_SIZE), {double(size)}, &undoManager);

    point.appendChild(xy, &undoManager);
    points.appendChild(point, &undoManager);
};

const::juce::ValueTree& duck::vt::ValueTree::getRoot() const {
    return vtRoot;
}



bool duck::vt::ValueTree::copyFrom(const void* data, int sizeInBytes){
    vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
    return vtRoot.isValid();
}

void duck::vt::ValueTree::writeToStream(juce::OutputStream& stream) const {
    vtRoot.writeToStream(stream);
}

void duck::vt::ValueTree::setChild(const duck::vt::Tree& type, juce::ValueTree& toTree) {
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
juce::ValueTree* duck::vt::ValueTree::getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree) {
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

