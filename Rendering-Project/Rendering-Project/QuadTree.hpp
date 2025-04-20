#pragma once
#include "SceneObject.hpp"
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <array>
#include <iostream>
#include <memory>

class QuadTree {
  public:
    void AddElement(SceneObject* elementAddress);
    std::vector<SceneObject*> CheckTree(DirectX::BoundingFrustum& frustum);

  private:
    struct Node {
        DirectX::BoundingBox boundingBox;
        SceneObject* elementAddress;
        std::unique_ptr<Node> children[4];
    };

    void AddToNode(SceneObject* elementAddress, std::unique_ptr<Node>& node);
    void CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
                   std::vector<SceneObject*>& foundObjects);

    std::unique_ptr<Node> root;
    const DirectX::XMFLOAT3 volume = {1000.0f, 1000.0f, 1000.0f};
};