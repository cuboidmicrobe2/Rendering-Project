# Rendering-Project

School project by Sebastian Einarsson and Simon F. Nilsson.

## Showcase

This segment displays the keybinds for the window and what techniques are found in each scene.

### Controls

- WASD
  - Standard movement
- SPACE
  - Up
- CTRL
  - Down
- 1
  - Switches to Scene 1.
- 2
  - Switches to Scene 2.
- E
  - Pauses the mouse movement on the camera.
- ESC
  - Closes down the application.
- F11
  - Fullscreen toggle

### Deferred rendering

- Everything is rendered deferred, this can be confirmed through RenderDoc by checking Either PS stage output or, probably even better, checking CS stage input. It can also be checked by viewing the code for the [main PS](./Rendering-Project/Rendering-Project/deferredPS.hlsl) and the [dcem PS](./Rendering-Project/Rendering-Project/DCEMPS.hlsl) (handles the special sampling for Dynamic Cube Environment Mapped objects). You could also check the [compute shader](./Rendering-Project/Rendering-Project/ComputeShader.hlsl) that writes the final output for a camera.

### Shadow mapping

- You can confirm that shadow mapping works in both scenes however scene one is intended specifically for, among other things this. There are three spotlights in this scene with the light colors red, green and blue that shines on a few cubes. There is also a directional light that shines down on the scene, each of these lights casts shadows on the scene. The lights positions are marked with a sphere including the directional light (Although the directional light in theory has no position I figured it would still be helpfull to see the position of the near plane and the center from which the shadow mapping texture will be drawn). You can also se that the shadows are dynamic through the spinning cube in the scene.

### Tessellation

- Can be seen on the sphere in Scene 1.
- Can be seen on the volcano in Scene 2.

### Dynamic cube environment mapping

- In scene one there is a mirrored sphere that is implemented through Dynamic Cube Environment Mapping (DCEM). The textures for this object is rendered through the same pipeline as for the main camera, as such you can see that it contains all the light calculations and shadow calculations that the main camera has by viewing it in a way that the spinning cube is visible.

### Frustum culling using a quadtree

- The culling frustum has been made smaller so the culling effect can be seen during runtime.

### GPU-based billboarded particle system

- Can be seen used for the smoke effect from the volcano in Scene 2.

## Camera

The [Camera](./Rendering-Project/Rendering-Project/Camera.hpp) class is a way to keep track of view point from which rendering can happen, it also tracks a place that should be rendered to. To help with what resources should be used when rendering it also contains a Rendering Resources helper class that has the gbuffers and depthstencil that should be used when rendering for that camera to ensure correct settings for those textures. The camera also contains a "parent" pointer to ensure if a object is rendering to it's own resources the renderer can notice this and not allow it to render itself leading to a simultanious read / write warning.

## Scene Management

The [BaseScene](./Rendering-Project/Rendering-Project/BaseScene.hpp) class makes building scenes easy by packaging all the necessary functions for creating your scene into a single class. Then you can just inherit from this class and call those functions in the constructor function to build the scene. Scene swapping is configured in the main file and simply swaps the scene that is being rendered to another scene.

## Deferred rendering

Deferred rendering simply means that the lighting calculations are rendered after the main pipeline is finished in a seperate compute shader pass, this means that everything not in the viewfrustrum has been culled and leads to more efficient lighting calculations.

## Shadow mapping

Shadow mapping is implemented by having each light make a view/projection matrix that it sends to the Vertex Shader in a shadow pass, during this pass nothing else is bound to the pipeline and the output is only the depthstencil that is then saved on a Texture2DArray on the same index as the light. This is then passed along to the compute shader that can then check if something is in shadow or not.

## Tessellation

Level of detail using tessellation.

### Core Components

- Object Configuration
  - SceneObject instances can be created with tessellation enabled or disabled with the constructor parameters "shouldBeTessellated" and "showTessellation".
- Distance-Based Tessellation
  - The renderer calculates distance between camera and objects and passes this data to the hull shader to determine the tessellation factors for the objects.
- [Hull Shader](./Rendering-Project/Rendering-Project/TessellationHS.hlsl)
  - Determines tessellation levels with these features:
    - Uses distance-based level of detail (15.0 at close range, 0.01 at 50+ units).
    - Implements "fractional_odd" partitioning for smooth transitions.
    - Configures triangle patches with clockwise winding with "triangle_cw".
- [Domain Shader](./Rendering-Project/Rendering-Project/TessellationDS.hlsl)
  - Places new vertices using Phong tessellation:
    - Calculates both linear interpolation positions and Phong positions.
    - Blends between linear and Phong tessellation (70% Phong bias).
    - Projects final positions to screen space using view-projection matrix.

### Runtime

The renderer manages the tessellation state through the SetTessellation() method:

- When enabled, it switches to 3-control-point patch list topology.
- Activates the hull and domain shaders.
- Binds necessary the distance data and camera information to constant buffers.
- Optionally enables wireframe visualization for debugging.

The renderer handles the pipeline state changes when moving between tessellated and non-tessellated objects during the scene rendering, maintaining correct shader and topology configuration.

### Visualization

The system provides a wireframe visualization mode that shows the tessellated geometry which is helpful for debugging and demonstrating the varying tessellation factors as objects move relative to the camera.

## OBJ-parser

The OBJ parser is downloaded from [github](https://github.com/Bly7/OBJ-Loader) we use this to get all the vertex data and the textures that are bound to the object. We then use the directXTK WicLoader to load most textures. Only the normalmap and parallax map is loaded in another function since the WicLoader does some sRGB correction that breaks normal maps and parallax maps.
## Dynamic cube environment mapping

Dynamic Cube environment mapping is implemented by having an object contain six cameras, one in each direction that then writes the render output to the cubes Texture2DArray. After that, when the cube itself is drawn from another camera it swaps pixel shader to [DCEMPS](./Rendering-Project/Rendering-Project/DCEM.hpp) that samples a cube texture using the reflection vector between the camera view and the object surface. After it has finished drawing it swaps back to the original Pixel shader.
## Frustum culling using a quadtree

The frustum culling system efficiently excludes objects outside the camera's view using hierarchical spatial testing to minimize rendering overhead and improve performance.

### Core Components

- The culling system consists of three elements:
  - QuadTree Structure
    - Spatially organizes scene objects in hierarchical quadrants for efficient visibility queries.
  - Bounding Volume Testing
    - Uses simple geometric shapes to approximate object boundaries for fast intersection tests with the view frustum.
  - Hierarchical Culling
    - Tests scene visibility from top down, eliminating entire branches early when parent nodes are outside the view frustum.

### Pipeline Flow

- The frustum culling pipeline follows this sequence:

  - Scene Organization

    - Each object is placed in appropriate nodes based on its bounding box.
    - Nodes are subdivided when they contain too many objects.
    - Dynamic objects are tracked separately.

  - Frustum Generation

    - Using the camera's view and projection matrices.
    - Transforming the frustum from view space to world space.
    - Creating a DirectX::BoundingFrustum object for intersection tests.

  - Visibility Determination
    - Tests node bouning boxes against the frustum for quick elimination.
    - Checks individual object bounding boxes for potentially visible nodes.
    - Builds a list of visible objects to be rendered.
    - Dynamic objects are excluded from culling.

### Optimization Techniques

- The system implements several optimizations:
  - Early Rejection
    - Entire branches of the quadtree are skipped if their bounding boxes don't intersect the frustum.
  - Spatial Coherence
    - Objects that are close together are grouped in the same node, improving cache efficiency.
  - Hierarchical Testing
    - Tests start with broad node boundaries before testing individual objects.
  - Minimal Per-Frame Cost
    - The quadtree structure is built once at initialization, only traversal happens per frame.

## GPU-based billboarded particle system

The particle system is a GPU-accelerated system that simulates and renders thousands of particles using compute shaders and the graphics pipeline.

### Core Components

- The particle system consists of four parts:
  - Data Structure
  - GPU-Based Simulation
  - Billboard Generation
  - Procedural Effects

### Simulation Pipeline

- The particle simulation happens entirely on the GPU through these stages:
  - [Compute Shader](./Rendering-Project/Rendering-Project/ParticleCS.hlsl)
    - Applies random forces for natural movement.
    - Updates position based on velocities.
    - Manages particle lifecycle with automatic respawning.
    - Runs in thread groups of 32 particles.
  - [Vertex Shader](./Rendering-Project/Rendering-Project/ParticleVS.hlsl)
    - Retrieves particle position and lifetime.
    - Calculates particle size based on remaining lifetime.
    - Passes data to the geometry shader.
  - [Geometry Shader](./Rendering-Project/Rendering-Project/ParticleGS.hlsl)
    - Generates a quad for each particle.
    - Orients billboards to always face the camera.
    - Sets up texture coordinates and color based on particle lifetime.
    - Outputs two triangles per particle.
  - [Pixel Shader](./Rendering-Project/Rendering-Project/ParticlePS.hlsl)
    - Creates patterns based on texture coordinates.
    - Applies solf edge falloff for natural appearance.
    - Outputs color data compatible with the deferred rendering pipeline.

### Integration with Rendering

- The following steps explain how the particle system interacts with the renderer:
  - The renderer calls UpdateParticles() to run the compute shader simulation that updates the particles.
  - A separate draw call handles the particles during the regular scene rendering.
  - The system temporarily switches pipeline states for particles then reverts for normal geometry.
