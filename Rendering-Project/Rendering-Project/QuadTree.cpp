#include "QuadTree.hpp"

void QuadTree::AddElement(SceneObject* elementAddress) { this->AddToNode(elementAddress, this->root); }

std::vector<SceneObject*> QuadTree::CheckTree(DirectX::BoundingFrustum& frustum) {
    std::vector<SceneObject*> toReturn;

    this->CheckNode(frustum, this->root, toReturn);

    return toReturn;
}

void QuadTree::AddToNode(SceneObject* elementAddress, std::unique_ptr<Node>& node) {
    bool collision = elementAddress->GetBoundingBox().Intersects(node->boundingBox);
}

void QuadTree::CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
                         std::vector<SceneObject*>& foundObjects) {}
