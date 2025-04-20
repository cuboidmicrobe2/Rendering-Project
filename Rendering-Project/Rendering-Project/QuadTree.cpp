#include "QuadTree.hpp"
#include <iomanip>
#include <iostream>
#include <queue>

QuadTree::QuadTree(DirectX::XMVECTOR volume, int maxDepth, int maxElements)
    : maxDepth(maxDepth), maxElements(maxElements) {
    // Initialize the root node
    this->root = std::make_unique<Node>();

    // Define the center and extents of the bounding box
    DirectX::XMVECTOR center  = DirectX::XMVectorZero(); // Assuming the root is centered at the origin
    DirectX::XMVECTOR extents = DirectX::XMVectorScale(volume, 0.5f);

    // Calculate min and max points
    DirectX::XMVECTOR minPoint = DirectX::XMVectorSubtract(center, extents);
    DirectX::XMVECTOR maxPoint = DirectX::XMVectorAdd(center, extents);

    // Create the bounding box
    DirectX::BoundingBox::CreateFromPoints(this->root->boundingBox, minPoint, maxPoint);
}

void QuadTree::PrintTree(const QuadTree& tree) {
    std::cout << "QuadTree Structure:\n";
    this->PrintNode(tree.root);
}

void QuadTree::AddElement(SceneObject* elementAddress) { this->AddToNode(elementAddress, this->root, 0); }

std::vector<SceneObject*> QuadTree::CheckTree(DirectX::BoundingFrustum& frustum) {
    std::vector<SceneObject*> toReturn;

    this->CheckNode(frustum, this->root, toReturn);

    return toReturn;
}

void QuadTree::PrintNode(const std::unique_ptr<Node>& node, int depth) {
    if (!node) return;

    std::queue<std::pair<Node*, int>> queue;
    queue.push({node.get(), depth});

    while (!queue.empty()) {
        auto [currentNode, currentDepth] = queue.front();
        queue.pop();

        // Indentation based on depth
        std::cout << std::string(currentDepth * 2, ' ') << "Node at depth " << currentDepth << ":\n";

        // Bounding box details
        std::cout << std::string(currentDepth * 2, ' ') << "  BoundingBox Center: ("
                  << currentNode->boundingBox.Center.x << ", " << currentNode->boundingBox.Center.y << ", "
                  << currentNode->boundingBox.Center.z << ")\n";
        std::cout << std::string(currentDepth * 2, ' ') << "  BoundingBox Extents: ("
                  << currentNode->boundingBox.Extents.x << ", " << currentNode->boundingBox.Extents.y << ", "
                  << currentNode->boundingBox.Extents.z << ")\n";

        // Print element address if present
        if (currentNode->elementAddress) {
            std::cout << std::string(currentDepth * 2, ' ') << " Element Address: " << currentNode->elementAddress
                      << "\n";
        } else {
            std::cout << std::string(currentDepth * 2, ' ') << " Element Address: None\n";
        }

        for (int i = 0; i < 4; ++i) {
            if (currentNode->children[i]) {
                queue.push({currentNode->children[i].get(), currentDepth + 1});
            }
        }
    }
}

void QuadTree::AddToNode(SceneObject* elementAddress, std::unique_ptr<Node>& node, int currentDepth) {
    // Base case: Stop recursion if the maximum depth is reached
    if (currentDepth >= maxDepth) {
        if (node->elementAddress == nullptr) {
            node->elementAddress = elementAddress; // Store the element if the node is empty
        }
        return;
    }

    // Check for collision between the element's bounding box and the node's bounding box
    if (!elementAddress->GetBoundingBox().Intersects(node->boundingBox)) {
        return; // No collision, exit early
    }

    // Check if the current node is a leaf node (no children)
    bool isLeaf = (node->children[0] == nullptr);
    if (isLeaf) {
        // Check if the node can store more elements
        if (node->elementAddress == nullptr) {
            // Add the object to the node
            node->elementAddress = elementAddress;
            return;
        } else {
            // The node is a leaf node, but it is full
            // Subdivide the node
            this->SubdivideNode(node);

            // Redistribute the current element to the appropriate child node
            for (int i = 0; i < 4; ++i) {
                this->AddToNode(node->elementAddress, node->children[i], currentDepth + 1);
            }

            // Clear the current node's element
            node->elementAddress = nullptr;
        }
    }

    // The current node is now a parent node
    // Recursively add the element to the appropriate child nodes
    for (int i = 0; i < 4; ++i) {
        this->AddToNode(elementAddress, node->children[i], currentDepth + 1);
    }
}

void QuadTree::SubdivideNode(std::unique_ptr<Node>& node) {
    DirectX::XMVECTOR center      = DirectX::XMLoadFloat3(&node->boundingBox.Center);
    DirectX::XMVECTOR extents     = DirectX::XMLoadFloat3(&node->boundingBox.Extents);
    DirectX::XMVECTOR halfExtents = DirectX::XMVectorMultiply(extents, {0.5f, 1.0f, 0.5f});

    // Define offsets for the 4 child nodes
    const DirectX::XMVECTOR offsets[4] = {
        // -x -z quadrant
        DirectX::XMVectorSet(-DirectX::XMVectorGetX(halfExtents), 0.0f, -DirectX::XMVectorGetZ(halfExtents), 0.0f),
        // Bottom-right
        DirectX::XMVectorSet(DirectX::XMVectorGetX(halfExtents), 0.0f, -DirectX::XMVectorGetZ(halfExtents), 0.0f),
        // Top-left
        DirectX::XMVectorSet(-DirectX::XMVectorGetX(halfExtents), 0.0f, DirectX::XMVectorGetZ(halfExtents), 0.0f),
        // Top-right
        DirectX::XMVectorSet(DirectX::XMVectorGetX(halfExtents), 0.0f, DirectX::XMVectorGetZ(halfExtents), 0.0f)};

    for (int i = 0; i < 4; ++i) {
        node->children[i] = std::make_unique<Node>();

        // Calculate the center of the child node
        DirectX::XMVECTOR childCenter = DirectX::XMVectorAdd(center, offsets[i]);

        // Create the bounding box for the child node
        DirectX::BoundingBox::CreateFromPoints(node->children[i]->boundingBox,
                                               DirectX::XMVectorSubtract(childCenter, halfExtents),
                                               DirectX::XMVectorAdd(childCenter, halfExtents));
    }
}

void QuadTree::CheckNode(DirectX::BoundingFrustum& frustum, std::unique_ptr<Node>& node,
                         std::vector<SceneObject*>& foundObjects) {
    // Check if there is a collision between the frustum and the node's bounding volume
    bool collision = frustum.Intersects(node->boundingBox);
    if (!collision) // No collision found, node and potential children not relevant
    {
        return;
    }

    // Check if the current node is a leaf node (no children)
    bool isLeaf = (node->children[0] == nullptr);
    if (isLeaf) {
        // Each element in the node
        if (node->elementAddress != nullptr) {
            // Check if there is a collision between the frustum and the object's bounding volume
            collision = frustum.Intersects(node->elementAddress->GetBoundingBox());
            if (collision) {
                // Check if the object is already present in the return vector
                auto it = std::find(foundObjects.begin(), foundObjects.end(), node->elementAddress);
                if (it == foundObjects.end()) {
                    // Add the object if not already present
                    foundObjects.push_back(node->elementAddress);
                }
            }
        }
    } else {
        // Recursively run this function for each of the child nodes of this node
        for (int i = 0; i < 4; ++i) {
            if (node->children[i] != nullptr) {
                this->CheckNode(frustum, node->children[i], foundObjects);
            }
        }
    }
}