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

juce::Identifier duck::vt::ValueTree::getIDFromType(const duck::vt::Tree& tree) {
    switch (tree) {
        case duck::vt::Tree::ROOT : return {"HentaiDuckRoot"}; break;
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

duck::vt::ValueTree::ValueTree() {

}

void duck::vt::ValueTree::create() {
    // create new one
    vtRoot = juce::ValueTree{getIDFromType(duck::vt::Tree::ROOT)};
    juce::ValueTree curve{getIDFromType(duck::vt::Tree::CURVE_DATA)};
    juce::ValueTree points{getIDFromType(duck::vt::Tree::NORMALIZED_POINTS)};
    curve.appendChild(points, &undoManager);
    vtRoot.appendChild(curve, &undoManager);

    addPoint({0.f, 0.f}, -10.f, 50.f, 20.f);
    addPoint({0.002f, 1.f}, 8.f, 50.f, 20.f);
    addPoint({0.3f, 0.3f}, -8.f, 50.f, 20.f);
    addPoint({0.5f, 0.f}, 0.f, 50.f, 20.f);
    addPoint({1.f, 0.f}, 0.f, 50.f, 20.f);
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

    auto curve = vtRoot.getOrCreateChildWithName(getIDFromType(tree::CURVE_DATA), &undoManager);
    auto points = curve.getOrCreateChildWithName(getIDFromType(tree::NORMALIZED_POINTS), &undoManager);
    if (!points.isValid()) return;
    juce::ValueTree point{getIDFromType(tree::POINT)};
    juce::ValueTree xy{getIDFromType(tree::COORDS)};

    xy.setProperty(getIDFromType(prop::X), {double(coords.x)}, &undoManager);
    xy.setProperty(getIDFromType(prop::Y), {double(coords.y)}, &undoManager);
    point.setProperty(getIDFromType(prop::POWER), {double(power)}, &undoManager);
    point.setProperty(getIDFromType(prop::MAX_ABSOLUTE_POWER), {double(maxAbsPower)}, &undoManager);
    point.setProperty(getIDFromType(prop::SIZE), {double(size)}, &undoManager);

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
    auto oldTree = getChildRecursive(id, vtRoot);

    if (oldTree != nullptr && oldTree->isValid()){
        auto parent = oldTree->getParent();
        if (parent.isValid()){
            auto idx = parent.indexOf(*oldTree);
            if (idx >= 0){
                parent.removeChild(idx, &undoManager);
                parent.appendChild(toTree, &undoManager);
            }
        }
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

