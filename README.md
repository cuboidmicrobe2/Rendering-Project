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
- Everything is rendered with this technique.

### Shadow mapping
- Can be seen on all objects in both Scene 1 and 2.

### Tessellation
- Can be seen on the sphere in Scene 1.
- Can be seen on the volcano in Scene 2.

### Dynamic cube environment mapping

### Frustum culling using a quadtree
- The culling frustum has been made smaller so the culling effect can be seen during runtime.

### GPU-based billboarded particle system
- Can be seen as the smoke effect from the volcano in Scene 2.

## Camera

## Scene Management

## Deferred rendering

## Shadow mapping

## Tessellation
Level of detail using tessellation.
### Core components
- Object Configuration
    - SceneObject instances can be created with tessellation enabled or disabled with the constructor parameters "shouldBeTessellated" and "showTessellation".
- Distance-Based Tessellation
    - The renderer calculates distance between camera and objects and passes this data to the hull shader to determine the tessellation factors for the objects.
- [Hull Shader](TessellationHS.hlsl)
    - Determines tessellation levels with these features:
        - Uses distance-based level of detail (15.0 at close range, 0.01 at 50+ units).
        - Implements "fractional_odd" partitioning for smooth transitions.
        - Configures triangle patches with clockwise winding with "triangle_cw".
- [Domain Shader](TessellationDS.hlsl)
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

## Dynamic cube environment mapping

## Frustum culling using a quadtree

## GPU-based billboarded particle system