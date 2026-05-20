/*****************************************************************************
 *  SUBMERSIBLE SIMULATION
 *
 *  Author: Amir Tarbiyat - B00882695
 *  Date: 2024/12/3
 *
 *  Description:
 *  This OpenGL program simulates an underwater scene featuring a submarine,
 *  dynamic water surface, coral reefs, schools of fish, and environmental
 *  effects like fog and lighting. The submarine can be moved using keyboard
 *  controls, and the camera view can be adjusted with the mouse. The scene
 *  includes animated elements such as fish swimming in wavy circles and a
 *  water surface that simulates waves.
 *
 *  Functions:
 *  - main(): Entry point of the program. Initializes OpenGL, loads models
 *            and textures, and sets up callbacks.
 *
 *  - printControls(): Displays the user controls and author information.
 *
 *  - loadOBJ(): Loads 3D models from .obj files into Model structures.
 *
 *  - cleanupModel(): Frees memory allocated for a single model.
 *
 *  - cleanup(): Frees memory for all models and is registered to run at exit.
 *
 *  - initWaterSurface(): Initializes the vertices and normals for the water surface mesh.
 *
 *  - updateWaterSurface(): Animates the water surface over time to simulate waves.
 *
 *  - initFish(): Initializes fish groups and individual fish properties.
 *
 *  - updateParticles(): Manages the emission and updating of particles for the
 *                       submarine's turbine effect.
 *
 *  - initParticles(): Initializes the particle system.
 *
 *  - display(): Renders the entire scene, including the submarine, water surface,
 *               coral, fish, and particles.
 *
 *  - reshape(): Handles window resizing and updates the viewport and projection matrix.
 *
 *  - idle(): Updates the scene elements between frames, such as moving the submarine,
 *            animating the water surface, updating fish positions, and particles.
 *
 *  - keyDown(), keyUp(): Handle keyboard input for submarine movement and other controls.
 *
 *  - specialKeyDown(), specialKeyUp(): Handle special keys like the arrow keys for vertical movement.
 *
 *  - mouseMotion(): Adjusts the camera angle based on mouse movement.
 *
 *  - setupFog(): Configures the fog effect in the scene.
 *
 *  - loadPPM(): Loads a P6 PPM image file to use as a texture.
 *
 *  Data Structures:
 *  - Vertex: Represents a 3D point or normal vector.
 *  - Face: Represents a triangle face in a 3D model, storing indices to vertices and normals.
 *  - Model: Stores the vertices, normals, and faces of a 3D model.
 *  - CoralInstance: Stores information about a coral object's model index, position, and scale.
 *  - Fish: Represents an individual fish with properties for position, movement, and group association.
 *  - FishGroup: Stores properties for a group of fish moving together.
 *  - Particle: Represents a particle in the submarine's turbine effect with properties for
 *              position, velocity, life, and size.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <freeglut.h>
#include <time.h>  // For random seed

#define DEG2RAD (3.14159265f / 180.0f)

// Movement state variables
int move_forward = 0;
int move_backward = 0;
int move_left = 0;
int move_right = 0;
int move_up = 0;
int move_down = 0;

// Submarine position
float submarine_pos[3] = { 0.0f, 0.0f, 0.0f };

// Camera angles
float camera_angle_azimuth = 0.0f;    // Horizontal angle in degrees
float camera_angle_elevation = 0.0f;  // Vertical angle in degrees
float camera_distance = 5.0f;         // Distance from the submarine

// Mouse position
int last_mouse_x = -1;
int last_mouse_y = -1;

int wireframe = 0;    // 0 for solid, 1 for wireframe
int fullscreen = 0;   // 0 for windowed, 1 for fullscreen
int window_width = 800;
int window_height = 600;

// Fog
int fog_enabled = 1; // 0 for off, 1 for on

// Water surface mesh
#define WATER_GRID_SIZE 100
#define WATER_SIZE 100.0f  // Width and depth of the water surface
float water_vertices[WATER_GRID_SIZE][WATER_GRID_SIZE][3];  // x, y, z positions
float water_normals[WATER_GRID_SIZE][WATER_GRID_SIZE][3];   // normals for lighting

// Light intensity factors
float ambient_intensity = 0.2f;
float diffuse_intensity = 0.8f;
float specular_intensity = 1.0f;

#define NUM_CORAL_MODELS 14

// Texture Mapping
GLuint sand_texture;
int texture_width, texture_height;

#define NUM_CORAL_INSTANCES 30

typedef struct {
    int model_index;     // Index into coral_models array
    float position[3];   // x, y, z position
    float scale;
    //float rotation;      // Angle in degrees
} CoralInstance;

CoralInstance coral_instances[NUM_CORAL_INSTANCES];


// Structures for the submarine model
typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    int v[3]; // indices of vertices
    int n[3]; // indices of normals
} Face;

// Model structure to hold model data
typedef struct {
    Vertex* vertices;
    int vertex_count;
    int vertex_capacity;

    Vertex* normals;
    int normal_count;
    int normal_capacity;

    Face* faces;
    int face_count;
    int face_capacity;
} Model;

Model submarine_model;                 // Model for the submarine
Model coral_models[NUM_CORAL_MODELS];  // Array of coral models

#define NUM_FISH 10

typedef struct {
    float position[3];     // x, y, z position
    float speed;           // Movement speed (not used individually anymore)
    float size;            // Scale of the fish
    float angle_offset;    // Offset angle from group's angle
    float wave_frequency;  // Frequency of the wavy motion (optional per fish)
    float wave_amplitude;  // Amplitude of the wavy motion (optional per fish)
    int group_id;          // ID of the group the fish belongs to
    // New variables for horizontal wavy circle motion
    float horizontal_wave_amplitude;
    float horizontal_wave_frequency;
    float horizontal_wave_phase;
} Fish;

Fish fish_list[NUM_FISH];

#define NUM_FISH_GROUPS 3

typedef struct {
    float angle;             // Current angle around the circle
    float speed;             // Speed of the group's movement
    float circle_radius;     // Radius of the circular path
    float height;            // Height of the group
    float wave_frequency;    // Frequency of the wavy motion
    float wave_amplitude;    // Amplitude of the wavy motion
} FishGroup;

FishGroup fish_groups[NUM_FISH_GROUPS];

// Particle system for submarine's turbine effect
#define MAX_PARTICLES 500

typedef struct {
    int active;          // Is the particle active?
    float position[3];   // Position of the particle
    float velocity[3];   // Velocity of the particle
    float life;          // Remaining life of the particle
    float size;          // Size of the particle
} Particle;

Particle particles[MAX_PARTICLES];

// Global arrays for model data
Vertex* vertices = NULL;
int vertex_count = 0;
int vertex_capacity = 0;

Vertex* normals = NULL;
int normal_count = 0;
int normal_capacity = 0;

Face* faces = NULL;
int face_count = 0;
int face_capacity = 0;

// Function to load the .obj file into a Model structure
void loadOBJ(const char* filename, Model* model) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "r");
    if (err != 0 || !file) {
        printf("Failed to open file %s\n", filename);
        exit(1);
    }

    // Initialize model counts and capacities
    model->vertex_count = 0;
    model->vertex_capacity = 0;
    model->vertices = NULL;

    model->normal_count = 0;
    model->normal_capacity = 0;
    model->normals = NULL;

    model->face_count = 0;
    model->face_capacity = 0;
    model->faces = NULL;

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'g') {
            // Ignore group names
            continue;
        }
        else if (line[0] == 'v' && line[1] == ' ') {
            // Vertex line
            if (model->vertex_count >= model->vertex_capacity) {
                model->vertex_capacity += 1000;
                model->vertices = realloc(model->vertices, model->vertex_capacity * sizeof(Vertex));
                if (!model->vertices) {
                    printf("Failed to allocate memory for vertices\n");
                    exit(1);
                }
            }
            Vertex v;
            sscanf_s(line, "v %f %f %f", &v.x, &v.y, &v.z);
            model->vertices[model->vertex_count++] = v;
        }
        else if (line[0] == 'v' && line[1] == 'n') {
            // Normal line
            if (model->normal_count >= model->normal_capacity) {
                model->normal_capacity += 1000;
                model->normals = realloc(model->normals, model->normal_capacity * sizeof(Vertex));
                if (!model->normals) {
                    printf("Failed to allocate memory for normals\n");
                    exit(1);
                }
            }
            Vertex n;
            sscanf_s(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            model->normals[model->normal_count++] = n;
        }
        else if (line[0] == 'f') {
            // Face line
            if (model->face_count >= model->face_capacity) {
                model->face_capacity += 1000;
                model->faces = realloc(model->faces, model->face_capacity * sizeof(Face));
                if (!model->faces) {
                    printf("Failed to allocate memory for faces\n");
                    exit(1);
                }
            }
            Face f;
            int matches = sscanf_s(line, "f %d//%d %d//%d %d//%d",
                &f.v[0], &f.n[0],
                &f.v[1], &f.n[1],
                &f.v[2], &f.n[2]);
            if (matches != 6) {
                printf("Error reading face data in %s\n", filename);
                continue;
            }
            // OBJ indices start at 1
            f.v[0]--; f.v[1]--; f.v[2]--;
            f.n[0]--; f.n[1]--; f.n[2]--;
            model->faces[model->face_count++] = f;
        }
    }

    fclose(file);
    printf("Loaded %s: %d vertices, %d normals, %d faces\n", filename, model->vertex_count, model->normal_count, model->face_count);
}

void cleanupModel(Model* model) {
    if (model->vertices) free(model->vertices);
    if (model->normals) free(model->normals);
    if (model->faces) free(model->faces);
    model->vertices = NULL;
    model->normals = NULL;
    model->faces = NULL;
    model->vertex_count = model->normal_count = model->face_count = 0;
    model->vertex_capacity = model->normal_capacity = model->face_capacity = 0;
}

// Function to clean up allocated memory
void cleanup() {
    int i;
    for (i = 0; i < NUM_CORAL_MODELS; i++) {
        cleanupModel(&coral_models[i]);
    }
}

void initFish() {
    // Initialize fish groups
    for (int g = 0; g < NUM_FISH_GROUPS; g++) {
        FishGroup* group = &fish_groups[g];
        group->angle = ((float)rand() / (float)RAND_MAX) * 360.0f;
        group->speed = 0.0f + ((float)rand() / (float)RAND_MAX) * 0.4f;
        group->circle_radius = 18.0f + g * 2.0f; // Vary radius per group
        group->height = 2.0f + g * 2.0f;       // Vary height per group
        group->wave_frequency = 1.0f + ((float)rand() / (float)RAND_MAX);
        group->wave_amplitude = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.2f;
    }

    // Initialize fish
    for (int i = 0; i < NUM_FISH; i++) {
        Fish* fish = &fish_list[i];

        // Assign fish to a group
        fish->group_id = i % NUM_FISH_GROUPS;
        FishGroup* group = &fish_groups[fish->group_id];

        // Random offset angle within the group to avoid overlapping
        fish->angle_offset = ((float)rand() / (float)RAND_MAX) * 20.0f - 10.0f; // Between -10 and +10 degrees

        // Random size between 0.5 and 1.0
        fish->size = 0.3f + ((float)rand() / (float)RAND_MAX) * 0.5f;

        // Vertical wave parameters (optional per fish)
        fish->wave_frequency = group->wave_frequency;
        fish->wave_amplitude = group->wave_amplitude;

        // Initialize horizontal wave parameters
        fish->horizontal_wave_amplitude = 1.0f + ((float)rand() / (float)RAND_MAX) * 1.0f; // Amplitude between 1.0 and 2.0
        fish->horizontal_wave_frequency = 0.5f + ((float)rand() / (float)RAND_MAX) * 1.0f; // Frequency between 0.5 and 1.5
        fish->horizontal_wave_phase = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159265f; // Phase between 0 and 2π

        // Initial position (will be updated in idle function)
        fish->position[0] = group->circle_radius * cosf((group->angle + fish->angle_offset) * DEG2RAD);
        fish->position[1] = group->height;
        fish->position[2] = group->circle_radius * sinf((group->angle + fish->angle_offset) * DEG2RAD);
    }
}

void initParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = 0;
    }
}

// Display callback function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Compute camera position based on mouse movement
    float camX = submarine_pos[0] + camera_distance * cosf(camera_angle_elevation * DEG2RAD) * sinf(camera_angle_azimuth * DEG2RAD);
    float camY = submarine_pos[1] + camera_distance * sinf(camera_angle_elevation * DEG2RAD);
    float camZ = submarine_pos[2] + camera_distance * cosf(camera_angle_elevation * DEG2RAD) * cosf(camera_angle_azimuth * DEG2RAD);

    // Camera setup
    gluLookAt(camX, camY, camZ,   // Eye position
        submarine_pos[0], submarine_pos[1], submarine_pos[2],   // Look-at point (submarine)
        0.0f, 1.0f, 0.0f);  // Up vector

    // Set up lighting (after gluLookAt)
    GLfloat light_ambient[] = { ambient_intensity, ambient_intensity, ambient_intensity, 1.0f };
    GLfloat light_diffuse[] = { diffuse_intensity, diffuse_intensity, diffuse_intensity, 1.0f };
    GLfloat light_specular[] = { specular_intensity, specular_intensity, specular_intensity, 1.0f };
    GLfloat light_direction[] = { -1.0f, -1.0f, -1.0f, 0.0f }; // Directional light

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_direction);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);


    // Disable depth mask to handle transparency correctly
    glDepthMask(GL_FALSE);

    // Set material properties for water (adjusted for better visibility)
    GLfloat water_ambient[] = { 0.0f, 0.0f, 0.6f, 0.7f };
    GLfloat water_diffuse[] = { 0.0f, 0.0f, 0.8f, 0.7f };
    GLfloat water_specular[] = { 0.8f, 0.8f, 1.0f, 0.7f };
    GLfloat water_shininess[] = { 100.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, water_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, water_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, water_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, water_shininess);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Position the water surface above the cylinder
    glPushMatrix();
    glTranslatef(0.0f, 20.0f, 0.0f);  // Adjust Y position as needed to sit on top of the cylinder

    // Rotate to match the disk orientation if necessary
    //glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Draw the water surface
    for (int i = 0; i < WATER_GRID_SIZE - 1; ++i) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j < WATER_GRID_SIZE; ++j) {
            // First vertex
            glNormal3fv(water_normals[i][j]);
            glVertex3fv(water_vertices[i][j]);
            // Second vertex
            glNormal3fv(water_normals[i + 1][j]);
            glVertex3fv(water_vertices[i + 1][j]);
        }
        glEnd();
    }

    glPopMatrix();

    // Disable blending
    glDisable(GL_BLEND);

    // Re-enable depth mask
    glDepthMask(GL_TRUE);


    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);

    // Bind the sand texture
    glBindTexture(GL_TEXTURE_2D, sand_texture);

    // Set high emissive material
    GLfloat mat_emission[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);

    // Draw the disk
    GLUquadric* quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);

    // Position the disk slightly below the origin
    glPushMatrix();
    glTranslatef(0.0f, -5.0f, 0.0f); // Adjust Y position as needed
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluQuadricOrientation(quadric, GLU_OUTSIDE); // Ensure normals point up
    gluDisk(quadric, 0.0f, 30.0f, 35, 1); // Inner radius, outer radius, slices, loops
    glPopMatrix();


    // Draw the cylinder
    glPushMatrix();
    glTranslatef(0.0f, -5.1f, 0.0f); // Slightly lower than the disk
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluQuadricOrientation(quadric, GLU_OUTSIDE); // Ensure normals point outwards
    gluCylinder(quadric, 29.0f, 29.0f, 25.0f, 100, 10); // Base radius, top radius, height, slices, stacks
    glPopMatrix();

    gluDeleteQuadric(quadric);

    // Disable texture mapping
    glDisable(GL_TEXTURE_2D);

    // Reset emissive material to zero
    GLfloat mat_no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_no_emission);

    // Set polygon mode (wireframe or solid)
    if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Disable lighting before drawing axes
    glDisable(GL_LIGHTING);

    // Draw axes
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    // X-axis in red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);

    // Y-axis in green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    // Z-axis in blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glEnd();

    // Draw white sphere at origin
    glColor3f(1.0f, 1.0f, 1.0f);
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, 0.01, 20, 20);
    gluDeleteQuadric(quad);

    // Re-enable lighting
    glEnable(GL_LIGHTING);

    // Apply yellow material to submarine
    GLfloat mat_ambient[] = { 0.7f, 0.7f, 0.0f, 1.0f };
    GLfloat mat_diffuse[] = { 0.7f, 0.7f, 0.0f, 1.0f };
    GLfloat mat_specular[] = { 0.9f, 0.9f, 0.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // Transform to submarine position
    glPushMatrix();
    glTranslatef(submarine_pos[0], submarine_pos[1], submarine_pos[2]);

    // Rotate the submarine 90 degrees to the right around the Y-axis
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    // Apply scaling to the submarine model
    float scale_factor = 0.005f; // Adjusted scale factor to reduce size
    glScalef(scale_factor, scale_factor, scale_factor);

    // Get a pointer to the submarine model
    Model* model = &submarine_model;

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < model->face_count; i++) {
        Face f = model->faces[i];
        for (int j = 0; j < 3; j++) {
            if (f.n[j] >= 0 && f.n[j] < model->normal_count) {
                glNormal3f(model->normals[f.n[j]].x, model->normals[f.n[j]].y, model->normals[f.n[j]].z);
            }
            if (f.v[j] >= 0 && f.v[j] < model->vertex_count) {
                glVertex3f(model->vertices[f.v[j]].x, model->vertices[f.v[j]].y, model->vertices[f.v[j]].z);
            }
        }
    }
    glEnd();

    glPopMatrix();


    // Apply green material to coral
    GLfloat coral_ambient[] = { 0.0f, 0.5f, 0.0f, 1.0f };
    GLfloat coral_diffuse[] = { 0.0f, 0.8f, 0.0f, 1.0f };
    GLfloat coral_specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat coral_shininess[] = { 10.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, coral_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, coral_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, coral_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, coral_shininess);

    // Draw coral instances
    for (int i = 0; i < NUM_CORAL_INSTANCES; i++) {
        CoralInstance* instance = &coral_instances[i];
        Model* model = &coral_models[instance->model_index];

        glPushMatrix();

        // Position the coral
        glTranslatef(instance->position[0], instance->position[1], instance->position[2]);

        // Rotate to match the disk's orientation
        //glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        // Apply random rotation around the new y-axis (original z-axis)
        //glRotatef(instance->rotation, 0.0f, 0.0f, 1.0f);

        // Apply scaling
        glScalef(instance->scale, instance->scale, instance->scale);

        // Draw the coral model
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < model->face_count; j++) {
            Face f = model->faces[j];
            for (int k = 0; k < 3; k++) {
                if (f.n[k] >= 0 && f.n[k] < model->normal_count) {
                    glNormal3f(model->normals[f.n[k]].x, model->normals[f.n[k]].y, model->normals[f.n[k]].z);
                }
                if (f.v[k] >= 0 && f.v[k] < model->vertex_count) {
                    glVertex3f(model->vertices[f.v[k]].x, model->vertices[f.v[k]].y, model->vertices[f.v[k]].z);
                }
            }
        }
        glEnd();

        glPopMatrix();
    }

    // Set fish material properties
    GLfloat fish_ambient[] = { 0.8f, 0.5f, 0.0f, 1.0f };   // Orange color
    GLfloat fish_diffuse[] = { 0.8f, 0.5f, 0.0f, 1.0f };
    GLfloat fish_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat fish_shininess[] = { 20.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, fish_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, fish_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, fish_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, fish_shininess);

    for (int i = 0; i < NUM_FISH; i++) {
        Fish* fish = &fish_list[i];
        FishGroup* group = &fish_groups[fish->group_id];

        glPushMatrix();

        // Position the fish
        glTranslatef(fish->position[0], fish->position[1], fish->position[2]);

        // Orient the fish to face along the path
        float angle = group->angle + fish->angle_offset;
        glRotatef(-angle, 0.0f, 1.0f, 0.0f);

        // Rotate the pyramid so the apex points forward
        glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

        // Simulate fluttering by rotating the fish slightly over time
        float flutter = sinf(glutGet(GLUT_ELAPSED_TIME) / 100.0f + fish->angle_offset * DEG2RAD) * 10.0f;
        glRotatef(flutter, 0.0f, 0.0f, 1.0f);

        // Scale the fish
        glScalef(fish->size, fish->size, fish->size);

        // Draw the fish as a square-based pyramid
        glBegin(GL_TRIANGLES);

        // Front face
        glNormal3f(0.0f, 0.707f, 0.707f); // Approximate normal
        glVertex3f(0.0f, 0.5f, 0.0f);     // Apex
        glVertex3f(-0.5f, -0.5f, 0.5f);   // Base left
        glVertex3f(0.5f, -0.5f, 0.5f);    // Base right

        // Right face
        glNormal3f(0.707f, 0.707f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);

        // Back face
        glNormal3f(0.0f, 0.707f, -0.707f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);

        // Left face
        glNormal3f(-0.707f, 0.707f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        glEnd();

        glPopMatrix();
    }


    // Disable lighting for subsequent objects
    glDisable(GL_LIGHTING);

    // Render particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(9.0f);

    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &particles[i];
        if (p->active) {
            // Set color with alpha based on particle life
            float alpha = p->life / 2.0f; // Assuming max life is around 2.0
            glColor4f(1.0f, 1.0f, 1.0f, alpha);

            glVertex3fv(p->position);
        }
    }
    glEnd();

    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_BLEND);

    // Swap buffers (double buffering)
    glutSwapBuffers();
}

// Reshape callback function (handles window re-size)
void reshape(int width, int height) {
    window_width = width;
    window_height = height;

    // Set the viewport to cover the new window size
    glViewport(0, 0, width, height);

    // Set the projection matrix (perspective projection)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 100.0);

    // Return to modelview matrix
    glMatrixMode(GL_MODELVIEW);
}

void initWaterSurface() {
    float half_size = WATER_SIZE / 2.0f;
    float step = WATER_SIZE / (WATER_GRID_SIZE - 1);
    for (int i = 0; i < WATER_GRID_SIZE; ++i) {
        for (int j = 0; j < WATER_GRID_SIZE; ++j) {
            water_vertices[i][j][0] = -half_size + j * step;   // x
            water_vertices[i][j][1] = 0.0f;                    // y (height)
            water_vertices[i][j][2] = -half_size + i * step;   // z
            // Initial normal pointing up
            water_normals[i][j][0] = 0.0f;
            water_normals[i][j][1] = 1.0f;
            water_normals[i][j][2] = 0.0f;
        }
    }
}

void updateWaterSurface(float time) {
    float waveAmplitude1 = 0.6f;
    float waveFrequency1 = 0.2f;
    float waveAmplitude2 = 0.2f;
    float waveFrequency2 = 0.3f;

    for (int i = 0; i < WATER_GRID_SIZE; ++i) {
        for (int j = 0; j < WATER_GRID_SIZE; ++j) {
            float x = water_vertices[i][j][0];
            float z = water_vertices[i][j][2];

            // Calculate height
            float wave1 = sinf(waveFrequency1 * (x + z) + time) * waveAmplitude1;
            float wave2 = sinf(waveFrequency2 * (x - z) + time * 1.5f) * waveAmplitude2;
            float height = wave1 + wave2;

            water_vertices[i][j][1] = height;

            // Compute normals using central differences
            float dx = 0.0f;
            float dz = 0.0f;

            if (j > 0 && j < WATER_GRID_SIZE - 1)
                dx = (water_vertices[i][j + 1][1] - water_vertices[i][j - 1][1]) / (2.0f * (WATER_SIZE / WATER_GRID_SIZE));
            if (i > 0 && i < WATER_GRID_SIZE - 1)
                dz = (water_vertices[i + 1][j][1] - water_vertices[i - 1][j][1]) / (2.0f * (WATER_SIZE / WATER_GRID_SIZE));

            // Normal vector
            float nx = -dx;
            float ny = 1.0f;
            float nz = -dz;

            // Normalize the normal
            float length = sqrtf(nx * nx + ny * ny + nz * nz);
            water_normals[i][j][0] = nx / length;
            water_normals[i][j][1] = ny / length;
            water_normals[i][j][2] = nz / length;
        }
    }
}

void updateParticles(int emit) {
    float timeStep = 0.02f; // Time step for particle movement

    // Update existing particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &particles[i];
        if (p->active) {
            // Update particle position
            p->position[0] += p->velocity[0] * timeStep;
            p->position[1] += p->velocity[1] * timeStep;
            p->position[2] += p->velocity[2] * timeStep;

            // Decrease life
            p->life -= timeStep;

            // Deactivate particle if life is over
            if (p->life <= 0.0f) {
                p->active = 0;
            }
        }
    }

    // Emit new particles if submarine is moving
    if (emit) {
        int particlesToEmit = 10; // Number of particles to emit per frame
        for (int i = 0; i < MAX_PARTICLES && particlesToEmit > 0; i++) {
            Particle* p = &particles[i];
            if (!p->active) {
                // Activate particle
                p->active = 1;
                p->life = 1.0f + ((float)rand() / (float)RAND_MAX) * 1.0f; // Life between 1.0 and 2.0 seconds
                p->size = 0.05f + ((float)rand() / (float)RAND_MAX) * 0.05f; // Size between 0.05 and 0.1

                // Set initial position at the back of the submarine
                float zOffset = 0.5f; // Adjust this value as needed
                float yOffset = 0.1f;

                p->position[0] = submarine_pos[0];
                p->position[1] = submarine_pos[1] + yOffset;
                p->position[2] = submarine_pos[2] + zOffset;

                // Set initial velocity
                p->velocity[0] = ((float)rand() / (float)RAND_MAX) * 0.02f - 0.01f; // Small random spread
                p->velocity[1] = ((float)rand() / (float)RAND_MAX) * 0.02f - 0.01f;
                p->velocity[2] = 0.05f + ((float)rand() / (float)RAND_MAX) * 0.05f; // Move backward

                particlesToEmit--;
            }
        }
    }
}




// Idle function to update submarine position
void idle() {
    int need_redisplay = 0;
    float movement_speed = 0.02f; // Adjusted movement speed

    int submarine_moving = 0; // Flag to check if submarine is moving

    // Update submarine position
    if (move_forward) {
        submarine_pos[2] -= movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }
    if (move_backward) {
        submarine_pos[2] += movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }
    if (move_left) {
        submarine_pos[0] -= movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }
    if (move_right) {
        submarine_pos[0] += movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }
    if (move_up) {
        submarine_pos[1] += movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }
    if (move_down) {
        submarine_pos[1] -= movement_speed;
        need_redisplay = 1;
        submarine_moving = 1;
    }

    // Update water surface animation
    float timeValue = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;  // Time in seconds
    updateWaterSurface(timeValue);

    // Update fish group angles
    for (int g = 0; g < NUM_FISH_GROUPS; g++) {
        FishGroup* group = &fish_groups[g];
        group->angle += group->speed;
        if (group->angle > 360.0f) group->angle -= 360.0f;
    }

    // Get the current time in seconds
    //float timeValue = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Update fish positions
    for (int i = 0; i < NUM_FISH; i++) {
        Fish* fish = &fish_list[i];
        FishGroup* group = &fish_groups[fish->group_id];

        // Compute fish position based on group's angle plus offset
        float angle = group->angle + fish->angle_offset;

        // Calculate horizontal wave perturbation
        float radius_perturbation = fish->horizontal_wave_amplitude * sinf(fish->horizontal_wave_frequency * timeValue + fish->horizontal_wave_phase);

        // Apply perturbation to radius
        float current_radius = group->circle_radius + radius_perturbation;

        fish->position[0] = current_radius * cosf(angle * DEG2RAD);
        fish->position[2] = current_radius * sinf(angle * DEG2RAD);

        // Apply wavy vertical motion
        fish->position[1] = group->height + sinf(fish->wave_frequency * timeValue + angle * DEG2RAD) * fish->wave_amplitude;
    }

    updateParticles(submarine_moving);

    glutPostRedisplay();
}

// Keyboard callback function for key presses
void keyDown(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'W':
        move_forward = 1;
        break;
    case 's':
    case 'S':
        move_backward = 1;
        break;
    case 'a':
    case 'A':
        move_left = 1;
        break;
    case 'd':
    case 'D':
        move_right = 1;
        break;
    case 'u':
    case 'U':
        // Toggle wireframe mode
        wireframe = !wireframe;
        glutPostRedisplay();
        break;
    case 'f':
    case 'F':
        // Toggle fullscreen mode
        if (!fullscreen) {
            glutFullScreen();
            fullscreen = 1;
        }
        else {
            glutReshapeWindow(800, 600);
            glutPositionWindow(100, 100);
            fullscreen = 0;
        }
        break;
    case 'q':
    case 'Q':
        // Exit the application
        exit(0);
        break;
    case 'b':
    case 'B':
        fog_enabled = !fog_enabled; // Toggle fog
        if (fog_enabled) {
            glEnable(GL_FOG);
        }
        else {
            glDisable(GL_FOG);
        }
        glutPostRedisplay();
        break;

    case 'r':
    case 'R':
        ambient_intensity += 0.1f;
        diffuse_intensity += 0.1f;
        specular_intensity += 0.1f;
        if (ambient_intensity > 1.0f) ambient_intensity = 1.0f;
        if (diffuse_intensity > 1.0f) diffuse_intensity = 1.0f;
        if (specular_intensity > 1.0f) specular_intensity = 1.0f;
        glutPostRedisplay();
        break;
    case 't':
    case 'T':
        ambient_intensity -= 0.1f;
        diffuse_intensity -= 0.1f;
        specular_intensity -= 0.1f;
        if (ambient_intensity < 0.0f) ambient_intensity = 0.0f;
        if (diffuse_intensity < 0.0f) diffuse_intensity = 0.0f;
        if (specular_intensity < 0.0f) specular_intensity = 0.0f;
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

// Keyboard callback function for key releases
void keyUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
    case 'W':
        move_forward = 0;
        break;
    case 's':
    case 'S':
        move_backward = 0;
        break;
    case 'a':
    case 'A':
        move_left = 0;
        break;
    case 'd':
    case 'D':
        move_right = 0;
        break;
    default:
        break;
    }
}

// Special keys callback function for key presses
void specialKeyDown(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        move_up = 1;
        break;
    case GLUT_KEY_DOWN:
        move_down = 1;
        break;
    default:
        break;
    }
}

// Special keys callback function for key releases
void specialKeyUp(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        move_up = 0;
        break;
    case GLUT_KEY_DOWN:
        move_down = 0;
        break;
    default:
        break;
    }
}

void setupFog() {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP); // Exponential fog
    GLfloat fogColor[4] = { 0.0f, 0.0f, 0.8f, 1.0f }; // Blue fog
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.02f); // Adjust density as needed
    glHint(GL_FOG_HINT, GL_NICEST); // High-quality fog
    //glDisable(GL_FOG); // Start with fog disabled
}


// Mouse motion callback function
void mouseMotion(int x, int y) {
    if (last_mouse_x < 0 || last_mouse_y < 0) {
        last_mouse_x = x;
        last_mouse_y = y;
        return;
    }

    int dx = x - last_mouse_x;
    int dy = y - last_mouse_y;

    // Adjust sensitivity as needed
    float sensitivity = 0.2f;

    camera_angle_azimuth += dx * sensitivity;
    camera_angle_elevation += dy * sensitivity;

    // Clamp camera_angle_elevation to avoid flipping
    if (camera_angle_elevation > 89.0f) camera_angle_elevation = 89.0f;
    if (camera_angle_elevation < -89.0f) camera_angle_elevation = -89.0f;

    last_mouse_x = x;
    last_mouse_y = y;

    glutPostRedisplay();
}

// Function to print controls and author information
void printControls() {
    printf("\n");
    printf("=========================================\n");
    printf("                SUBMERSIBLE              \n");
    printf("=========================================\n");
    printf("\n");
    printf("Controls:\n");
    printf("-----------------------------------------\n");
    printf("  W           : Move Forward             \n");
    printf("  S           : Move Backward            \n");
    printf("  A           : Move Left                \n");
    printf("  D           : Move Right               \n");
    printf("  Up Arrow    : Move Up                  \n");
    printf("  Down Arrow  : Move Down                \n");
    printf("  U           : Toggle Wireframe Mode    \n");
    printf("  F           : Toggle Fullscreen Mode   \n");
    printf("  B           : Toggle Fog Effect        \n");
    printf("  R           : Increase Light Intensity \n");
    printf("  T           : Decrease Light Intensity \n");
    printf("  Q           : Quit the Program         \n");
    printf("-----------------------------------------\n");
    printf("\n");
    printf("Enjoy exploring the underwater world!\n");
    printf("\n");
    printf("=========================================\n");
    printf("          Author: Amir Tarbiyat          \n");
    printf("=========================================\n");
    printf("Note: This project was developed on Windows 11 and is only tested on Windows Operating System.\n");
    printf("\n");
}




// Function to load a P6 PPM image
unsigned char* loadPPM(const char* filename, int* width, int* height) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Failed to open PPM file %s\n", filename);
        return NULL;
    }

    char buff[16];
    // Read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror("Reading PPM file");
        fclose(fp);
        return NULL;
    }

    // Check the image format
    if (strncmp(buff, "P6", 2) != 0) {
        fprintf(stderr, "Invalid PPM format (must be 'P6')\n");
        fclose(fp);
        return NULL;
    }

    // Read image size information
    int max_color;
    int c;
    // Skip comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n');
        c = getc(fp);
    }
    ungetc(c, fp);

    // Read width, height
    if (fscanf_s(fp, "%d %d", width, height) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        fclose(fp);
        return NULL;
    }

    // Read max color value
    if (fscanf_s(fp, "%d", &max_color) != 1) {
        fprintf(stderr, "Invalid max color value (error loading '%s')\n", filename);
        fclose(fp);
        return NULL;
    }

    // Check max color value
    if (max_color != 255) {
        fprintf(stderr, "Max color value is not 255 in '%s'\n", filename);
        fclose(fp);
        return NULL;
    }

    while (fgetc(fp) != '\n'); // Skip to the end of the line

    // Allocate memory for pixel data
    int size = (*width) * (*height) * 3; // 3 bytes per pixel (RGB)
    unsigned char* data = (unsigned char*)malloc(size);
    if (!data) {
        fprintf(stderr, "Unable to allocate memory for image data\n");
        fclose(fp);
        return NULL;
    }

    // Read pixel data
    if (fread(data, 1, size, fp) != size) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        free(data);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return data;
}


int main(int argc, char** argv) {
    printControls();
    atexit(cleanup);  // Register cleanup function to free allocated memory

    // Initialize GLUT
    glutInit(&argc, argv);

    // Enable double buffering and depth buffering
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Initial window size and position
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);

    // Create the window
    glutCreateWindow("OpenGL Submarine");

    // Set the background color to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Enable depth testing for proper z-ordering
    glEnable(GL_DEPTH_TEST);

    // Enable two-sided lighting
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Disable face culling to render both front and back faces
    glDisable(GL_CULL_FACE);

    // Initialize water surface
    initWaterSurface();

    // fog
    setupFog();


    // Initialize the submarine model
    memset(&submarine_model, 0, sizeof(Model));
    loadOBJ("submarine.obj", &submarine_model);

    // Initialize coral models
    int i; 
    for (i = 0; i < NUM_CORAL_MODELS; i++) {
        memset(&coral_models[i], 0, sizeof(Model));
        char filename[256];
        sprintf_s(filename, sizeof(filename), "coral/coral_%d.obj", i + 1);
        loadOBJ(filename, &coral_models[i]);
    }

    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Generate coral instances
    for (int i = 0; i < NUM_CORAL_INSTANCES; i++) {
        CoralInstance* instance = &coral_instances[i];
        // Randomly select a coral model
        instance->model_index = rand() % NUM_CORAL_MODELS;

        // Random position within the disk
        float radius = 29.0f; // Slightly smaller than the disk radius
        float angle = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159265f;
        float distance = ((float)rand() / (float)RAND_MAX) * radius;

        instance->position[0] = distance * cosf(angle);
        instance->position[2] = distance * sinf(angle);
        instance->position[1] = -5.0f; // Y position matching the disk surface

        // Random scale between 0.5 and 1.5
        instance->scale = 2.5f + ((float)rand() / (float)RAND_MAX) * 1.0f;

        // Random rotation angle between 0 and 360 degrees
        //instance->rotation = ((float)rand() / (float)RAND_MAX) * 360.0f;
    }


    // Load the texture image
    unsigned char* texture_data = loadPPM("sand.ppm", &texture_width, &texture_height);
    if (!texture_data) {
        printf("Failed to load texture image\n");
        exit(1);
    }

    // Generate texture object
    glGenTextures(1, &sand_texture);
    glBindTexture(GL_TEXTURE_2D, sand_texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap texture horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap texture vertically
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Magnification filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Minification filter with mipmapping

    // Build mipmaps
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, texture_width, texture_height, GL_RGB, GL_UNSIGNED_BYTE, texture_data);

    // Free the texture data (it's now on the GPU)
    free(texture_data);

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);           // Handles window reshape
    glutIdleFunc(idle);                 // Idle function for movement
    glutKeyboardFunc(keyDown);          // Key press
    glutKeyboardUpFunc(keyUp);          // Key release
    glutSpecialFunc(specialKeyDown);    // Special key press
    glutSpecialUpFunc(specialKeyUp);    // Special key release
    glutPassiveMotionFunc(mouseMotion); // Mouse movement

    // Initialize fish
    initFish();

    //initialize particles
    initParticles();

    // Enter the GLUT main loop
    glutMainLoop();

    return 0;
}
