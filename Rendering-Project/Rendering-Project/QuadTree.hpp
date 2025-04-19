#pragma once
#include <DirectXCollision.h>
#include <array>
#include <iostream>
#include <memory>

template <typename T>
class QuadTree {
  public:
    void AddElement(T* elementAddress, const DirectX::BoundingBox& boundingBox);
    std::vector<T*> CheckTree(const DirectX::BoundingFrustum& frustum);

  private:
    struct Node {
        static constexpr size_t MAX_ELEMENTS = 4;
        DirectX::BoundingBox boundingBox;
        std::array<T*, MAX_ELEMENTS> elements;
        std::array<DirectX::BoundingBox, MAX_ELEMENTS> elementBoundingBoxes;
        size_t elementsCount = 0;
        std::unique_ptr<Node> children[4];
    };

    void AddToNode(T* elementAddress, const DirectX::BoundingBox& boundingBox, std::unique_ptr<Node>& node);
    void CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node,
                   std::vector<T*>& foundObjects);

    std::unique_ptr<Node> root;
};

template <typename T>
inline void QuadTree<T>::AddElement(T* elementAddress, const DirectX::BoundingBox& boundingBox) {
    this->AddToNode(elementAddress, boundingBox, this->root);
}

template <typename T>
inline std::vector<T*> QuadTree<T>::CheckTree(const DirectX::BoundingFrustum& frustum) {
    std::vector<T*> toReturn;

    CheckNode(frustum, root, toReturn);

    return toReturn;
}

template <typename T>
inline void QuadTree<T>::AddToNode(T* elementAddress, const DirectX::BoundingBox& boundingBox,
                                   std::unique_ptr<Node>& node) {
    if (!node) {
        node                          = std::make_unique<Node>();
        node->boundingBox             = boundingBox; // Initialize with element's bounding box
        node->elements[0]             = elementAddress;
        node->elementBoundingBoxes[0] = boundingBox;
        node->elementsCount           = 1;
        return;
    }

    // Check for collision with this node's bounding volume
    if (!boundingBox.Intersects(node->boundingBox)) {
        return; // No collision, this branch is not relevant
    }

    // Check if this is a leaf node (no children)
    bool isLeaf = true;
    for (const auto& child : node->children) {
        if (child) {
            isLeaf = false;
            break;
        }
    }

    if (isLeaf) {
        // Check if node has capacity
        if (node->elementsCount < Node::MAX_ELEMENTS) {
            // Add element to this node
            node->elements[node->elementsCount]             = elementAddress;
            node->elementBoundingBoxes[node->elementsCount] = boundingBox;
            node->elementsCount++;
            return;
        } else {
            // Node is full, subdivide it
            // Define child quadrants based on parent's bounding box
            DirectX::XMFLOAT3 parentCenter  = node->boundingBox.Center;
            DirectX::XMFLOAT3 parentExtents = node->boundingBox.Extents;
            DirectX::XMFLOAT3 childExtents(parentExtents.x * 0.5f, parentExtents.y * 0.5f, parentExtents.z * 0.5f);

            DirectX::XMFLOAT3 childCenters[4] = {
                // Northwest quadrant
                DirectX::XMFLOAT3(parentCenter.x - childExtents.x, parentCenter.y, parentCenter.z - childExtents.z),
                // Northeast quadrant
                DirectX::XMFLOAT3(parentCenter.x + childExtents.x, parentCenter.y, parentCenter.z - childExtents.z),
                // Southwest quadrant
                DirectX::XMFLOAT3(parentCenter.x - childExtents.x, parentCenter.y, parentCenter.z + childExtents.z),
                // Southeast quadrant
                DirectX::XMFLOAT3(parentCenter.x + childExtents.x, parentCenter.y, parentCenter.z + childExtents.z),
            };

            // Create child nodes
            for (int i = 0; i < 4; ++i) {
                node->children[i]                = std::make_unique<Node>();
                node->children[i]->boundingBox   = DirectX::BoundingBox(childCenters[i], childExtents);
                node->children[i]->elementsCount = 0;
            }

            // Store the existing elements
            std::array<T*, Node::MAX_ELEMENTS> oldElements                = node->elements;
            std::array<DirectX::BoundingBox, Node::MAX_ELEMENTS> oldBoxes = node->elementBoundingBoxes;
            size_t oldCount                                               = node->elementsCount;

            // Reset parent node's element count
            node->elementsCount = 0;

            // Redistribute existing elements to children
            for (size_t i = 0; i < oldCount; ++i) {
                bool placed = false;
                for (int j = 0; j < 4; ++j) {
                    if (oldBoxes[i].Intersects(node->children[j]->boundingBox)) {
                        // Recursively add to appropriate child
                        AddToNode(oldElements[i], oldBoxes[i], node->children[j]);
                        placed = true;
                        break;
                    }
                }

                // If element couldn't be placed in any child, keep it in this node
                if (!placed && node->elementsCount < Node::MAX_ELEMENTS) {
                    node->elements[node->elementsCount]             = oldElements[i];
                    node->elementBoundingBoxes[node->elementsCount] = oldBoxes[i];
                    node->elementsCount++;
                }
            }
        }
    }

    // Try to add the element to each child node
    bool placed = false;
    for (auto& child : node->children) {
        if (child && boundingBox.Intersects(child->boundingBox)) {
            // Recursively try to add to children
            AddToNode(elementAddress, boundingBox, child);
            placed = true;
            break;
        }
    }

    // If the element doesn't fit into any child, keep it in this node
    if (!placed && !isLeaf && node->elementsCount < Node::MAX_ELEMENTS) {
        node->elements[node->elementsCount]             = elementAddress;
        node->elementBoundingBoxes[node->elementsCount] = boundingBox;
        node->elementsCount++;
    }
}

template <typename T>
inline void QuadTree<T>::CheckNode(const DirectX::BoundingFrustum& frustum, const std::unique_ptr<Node>& node,
                                   std::vector<T*>& foundObjects) {
    // Exit early if node is null
    if (!node) {
        return;
    }

    // Check if there is a collision between the frustum and the node's bounding volume
    bool collision = frustum.Intersects(node->boundingBox);
    if (!collision) {
        // No collision found, node and potential children not relevant
        return;
    }

    // Check if node is a leaf (no children)
    bool isLeaf = true;
    for (const auto& child : node->children) {
        if (child) {
            isLeaf = false;
            break;
        }
    }

    // Process elements in this node
    for (size_t i = 0; i < node->elementsCount; ++i) {
        collision = frustum.Intersects(node->elementBoundingBoxes[i]);
        if (collision) {
            // Check if the object is already present in the return vector
            T* element        = node->elements[i];
            bool alreadyAdded = false;

            for (T* found : foundObjects) {
                if (found == element) {
                    alreadyAdded = true;
                    break;
                }
            }

            if (!alreadyAdded) {
                // Add to the result vector if not already present
                foundObjects.push_back(element);
            }
        }
    }

    // If not a leaf, recurse into children
    if (!isLeaf) {
        // Recursively check each child node
        for (const auto& child : node->children) {
            if (child) {
                CheckNode(frustum, child, foundObjects);
            }
        }
    }
}
