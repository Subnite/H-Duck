#include "ValueTreeManager.h"

duck::vt::Tree duck::vt::ValueTree::getTreeType(const juce::ValueTree& tree) {
    const auto type = tree.getType().toString();

    if (type == "HentaiDuckRoot") return duck::vt::Tree::ROOT;
    if (type == "CurveData") return duck::vt::Tree::CURVE_DATA;
    if (type == "NormalizedPoints") return duck::vt::Tree::NORMALIZED_POINTS;
    if (type == "Point") return duck::vt::Tree::POINT;
    if (type == "Coords") return duck::vt::Tree::COORDS;

    return duck::vt::Tree::UNDEFINED;
}

duck::vt::Property duck::vt::ValueTree::getPropertyType(const juce::Identifier& property) {
    const auto prop = property.toString();

    if (prop == "Power") return duck::vt::Property::POWER;
    if (prop == "MaxAbsolutePower") return duck::vt::Property::MAX_ABSOLUTE_POWER;
    if (prop == "Size") return duck::vt::Property::SIZE;
    if (prop == "X") return duck::vt::Property::X;
    if (prop == "Y") return duck::vt::Property::Y;

    return duck::vt::Property::UNDEFINED;
}

duck::vt::ValueTree::ValueTree() {

}

void duck::vt::ValueTree::create() {
    // create new one
    vtRoot = juce::ValueTree{"HentaiDuckRoot"};
    juce::ValueTree curve{"CurveData"};
    juce::ValueTree points{"NormalizedPoints"};
    curve.appendChild(points, &undoManager);
    vtRoot.appendChild(curve, &undoManager);

    addPoint({0.f, 1.f}, 10.f, 50.f, 20.f);
    addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);

    createXML("C:/Dev/Juce Projects/HentaiDuck/tree.xml");
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
    auto curve = vtRoot.getOrCreateChildWithName("CurveData", &undoManager);
    auto points = curve.getOrCreateChildWithName("NormalizedPoints", &undoManager);
    if (!points.isValid()) return;
    juce::ValueTree point{"Point"};
    juce::ValueTree xy{"Coords"};

    xy.setProperty("X", {double(coords.x)}, &undoManager);
    xy.setProperty("Y", {double(coords.y)}, &undoManager);
    point.setProperty("Power", {double(power)}, &undoManager);
    point.setProperty("MaxAbsolutePower", {double(maxAbsPower)}, &undoManager);
    point.setProperty("Size", {double(size)}, &undoManager);

    point.appendChild(xy, &undoManager);
    points.appendChild(point, &undoManager);
};

const::juce::ValueTree& duck::vt::ValueTree::getRoot() const {
    return vtRoot;
}



bool duck::vt::ValueTree::copyFrom(const void* data, int sizeInBytes){
    vtRoot = juce::ValueTree::readFromData(data, sizeInBytes);
    std::cout << "restored plugin state\n";
    return vtRoot.isValid();
}

void duck::vt::ValueTree::writeToStream(juce::OutputStream& stream) const {
    vtRoot.writeToStream(stream);
    std::cout << "wrote plugin state\n";
}

void duck::vt::ValueTree::setChild(const duck::vt::Tree& type, juce::ValueTree& toTree) {
    
    const auto id = getIDFromType(type);

    auto oldTree = getChildRecursive(id, vtRoot);

    // auto idx = vtRoot.indexOf(vtRoot.getChildWithName(id));
    // vtRoot.removeChild(idx, &undoManager);
    if (oldTree != nullptr && oldTree->isValid()){
        auto parent = oldTree->getParent();
        if (parent.isValid()){
            auto idx = parent.indexOf(*oldTree);
            if (idx >= 0){
                parent.removeChild(idx, &undoManager);
                parent.appendChild(toTree, &undoManager);
            }
        }
        // oldTree->copyPropertiesAndChildrenFrom(toTree, &undoManager);
    }
}

juce::ValueTree* duck::vt::ValueTree::getChildRecursive(const juce::Identifier& id, juce::ValueTree& tree) {
    if (tree.hasType(id)) return &tree;

    juce::ValueTree* oldTree = nullptr;

    // auto& would be better but this might be fine
    for (auto child : tree) {
        if (child.hasType(id)) {
            oldTree = &child;
            break;
        } else {
            oldTree = getChildRecursive(id, tree);
            if (oldTree != nullptr) break;
        }
    }
    
    return oldTree;
}

juce::Identifier duck::vt::ValueTree::getIDFromType(const duck::vt::Tree& tree) {
    switch (tree) {
        case duck::vt::Tree::ROOT : return {"Root"}; break;
        case duck::vt::Tree::CURVE_DATA : return {"CurveData"}; break;
        case duck::vt::Tree::NORMALIZED_POINTS : return {"NormalizedPoints"}; break;
        case duck::vt::Tree::POINT : return {"Point"}; break;
        case duck::vt::Tree::COORDS : return {"Coords"}; break;
        default: return {"Undefined"}; break;
    }
}

juce::Identifier duck::vt::ValueTree::getIDFromType(const duck::vt::Property& property) {
    switch (property) {
        case duck::vt::Property::POWER : return {"Power"}; break;
        case duck::vt::Property::MAX_ABSOLUTE_POWER : return {"MaxAbsolutePower"}; break;
        case duck::vt::Property::SIZE : return {"Size"}; break;
        case duck::vt::Property::X : return {"X"}; break;
        case duck::vt::Property::Y : return {"Y"}; break;
        default: return {"Undefined"}; break;
    }
}