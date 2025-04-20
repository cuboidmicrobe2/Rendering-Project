#pragma once
#include "SceneObject.hpp"
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <array>
#include <iostream>
#include <memory>

class QuadTree {
  public:
    QuadTree(DirectX::XMVECTOR volume, int maxDepth, int maxElements);
    void PrintTree(const QuadTree& tree);
    void AddElement(SceneObject* elementAddress);
    std::vector<SceneObject*> CheckTree(DirectX::BoundingFrustum& frustum);

  private:
    struct Node {
        DirectX::BoundingBox boundingBox;
        SceneObject* elementAddress;
        std::unique_ptr<Node> children[4];
    };

    void PrintNode(const std::unique_ptr<Node>& node, int depth = 0);
    void AddToNode(SceneObject* elementAddress, std::unique_ptr<Node>& node, int currentDepth);
    void SubdivideNode(std::unique_ptr<Node>& node);
    void CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
                   std::vector<SceneObject*>& foundObjects);

    int maxDepth;
    int maxElements;
    std::unique_ptr<Node> root;
};