# Submersible Simulation

## Author: Amir Tarbiyat 

---
## Introduction

This project is an OpenGL simulation of an underwater scene featuring a controllable submarine, dynamic water surface, coral reefs, schools of fish, and environmental effects such as fog and lighting. The simulation provides an immersive experience with interactive controls, allowing users to explore the underwater world and observe various marine elements in motion.

---

## Features

- **Controllable Submarine**: Navigate a submarine through the underwater environment using keyboard controls.
- **Dynamic Water Surface**: Simulates realistic water waves using procedural generation techniques.
- **Animated Fish**: Schools of fish swim in wavy circular patterns, adding life to the scene. the Speed, Height and amount of wavy movement is randomized. some      	fish may have large waves in their movement while some may not have any waves at all.
- **Coral Reefs**: Multiple coral models are randomly placed to create an authentic underwater landscape. since the OBJ loader I wrote can load many files, I opted 	to load all 14 coral files in the scene for more variety. a total of 30 corals are placed in the scene and they are chosen randomly from those 14 files.
- **Environmental Effects**:
  - **Fog**: Simulates underwater visibility limitations.
  - **Lighting**: Adjustable ambient, diffuse, and specular lighting enhances the visual appeal.
- **Particle System**: A turbine effect at the back of the submarine simulates propulsion when the submarine is moving.
- **User Interaction**:
  - Toggle wireframe mode.
  - Adjust lighting intensity.
  - Switch between fullscreen and windowed modes.
  - Camera control via mouse movement.

---
## Controls

- **Movement**:
  - `W` / `S`: Move Forward / Backward
  - `A` / `D`: Move Left / Right
  - `Up Arrow` / `Down Arrow`: Move Up / Down
- **View Control**:
  - **Mouse Movement**: Adjust camera angle (azimuth and elevation)
- **Modes and Effects**:
  - `U`: Toggle Wireframe Mode
  - `F`: Toggle Fullscreen Mode
  - `B`: Toggle Fog Effect
  - `R` / `T`: Increase / Decrease Lighting Intensity
- **Particle System**:
  - **Automatic**: The turbine effect appears when the submarine is moving.
- **Exit**:
  - `Q`: Quit the Program

---

## Implementation Details

### General Overview

The simulation is built using OpenGL's fixed-function pipeline and GLUT for window management and input handling. It leverages procedural generation and basic physics to create dynamic and interactive elements within the scene.

### Main Components

1. **Model Loading**:
   - **Description**: Parses Wavefront `.obj` files to load 3D models of the submarine and coral.

2. **Water Surface**:
   - **Initialization and Animation**: Creates a grid mesh to represent the water surface and applies mathematical functions to simulate waves.

3. **Fish Simulation**:
   - **Movement Patterns**: Fish swim in wavy circular paths, with both horizontal and vertical wave components for natural motion.

4. **Particle System**:
   - **Turbine Effect**: Simulates the visual effect of the submarine's propulsion when it's moving, enhancing realism.

5. **User Interaction**:
   - **Input Handling**: Keyboard and mouse inputs allow for movement and adjustments within the simulation.

6. **Rendering Pipeline**:
   - **Scene Rendering**: Renders all scene elements, including lighting setup, water surface, submarine, coral, fish, particles, and reference axes.

---

## Extra Features

### Particle System (Submarine Turbine Effect)

**Overview**:

The particle system adds a visual effect that simulates the submarine's propulsion when it moves. When the submarine is in motion, particles resembling bubbles or water disturbances are emitted from the back of the submarine, creating a realistic turbine effect.

**Application in the Scene**:

- **Activation**: The turbine effect automatically activates when the submarine is moving in any direction.
- **Visual Effect**:
  - Particles are emitted from a position offset behind the submarine to represent the turbine's location.
  - The particles move away from the submarine, simulating the force of propulsion.
- **Particle Behavior**:
  - Each particle has a limited lifespan, after which it disappears, ensuring that the effect remains localized and doesn't clutter the scene.
  - The particles disperse slightly as they move, mimicking the natural spread of bubbles underwater.
- **Rendering**:
  - Particles are rendered as small points or spheres, often with transparency to blend naturally with the underwater environment.
  - The size and opacity of particles can be adjusted to enhance the visual appeal.

**Impact on User Experience**:

- **Enhanced Realism**: The turbine effect adds a dynamic element that responds to the player's actions, making the submarine feel more responsive and alive.
- **Feedback Mechanism**: Visually indicates that the submarine is moving, providing immediate feedback to the user.
- **Immersion**: Contributes to the overall immersive experience by adding realistic details that one would expect in an underwater environment.

### Enhanced Water Waves

**Overview**:

The water surface in the simulation is designed to mimic realistic wave patterns, creating a dynamic and visually appealing environment. The waves are generated using mathematical functions that produce natural-looking undulations on the water surface.

**Application in the Scene**:

- **Dynamic Surface**:
  - The water surface is composed of a grid mesh that updates over time to simulate the movement of waves.
  - Waves are generated by combining multiple sine functions, resulting in complex and non-repetitive patterns.
- **Visual Effect**:
  - The water surface reflects and refracts light, enhancing the realism of the scene.
  - The movement of the waves affects the way light interacts with the water, creating shimmering and dynamic lighting effects.
- **Interaction with Other Elements**:
  - The submarine and fish appear to exist within the water, with the waves providing a context for their movements.
  - Shadows and reflections may be influenced by the wave patterns, depending on the rendering settings.

**Impact on User Experience**:

- **Immersion**: The dynamic waves contribute significantly to the feeling of being underwater, as the environment feels alive and constantly changing.
- **Visual Appeal**: Adds complexity and beauty to the scene, making it more engaging for the user.
- **Realism**: The natural motion of the waves enhances the overall realism of the simulation, making it more believable and enjoyable.

---

Thank you for exploring this underwater simulation. Enjoy navigating the depths and observing the marine life!

If you have any questions or feedback, please feel free to reach out.

---
