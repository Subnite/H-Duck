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
        case duck::vt::Property::LS_MIN_VALUE : return {"LS_MinValue"}; break;
        case duck::vt::Property::LS_MAX_VALUE : return {"LS_MaxValue"}; break;

        default: return {"Undefined"}; break;
    }
}

duck::vt::ValueTree::ValueTree() {

}

void duck::vt::ValueTree::create() {
    // create new one
    vtRoot = juce::ValueTree{getIDFromType(duck::vt::Tree::ROOT)};
    juce::ValueTree curve{getIDFromType(duck::vt::Tree::CD_CURVE_DATA)};
    juce::ValueTree points{getIDFromType(duck::vt::Tree::CD_NORMALIZED_POINTS)};
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

