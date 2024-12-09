#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <omp.h>

#define MAX_PLAYERS 100
#define GRID_SIZE 100  // Simulating a large game world
#define OBJECT_COUNT 50
#define LIGHT_COUNT 10
#define TEX_SIZE 256

// Class to represent 3D vectors (for positions, light sources, etc.)
class Vector3 {
public:
    float x, y, z;

    // Constructor for Vector3
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

// Class to represent textures (2D arrays)
class Texture {
public:
    float pixels[TEX_SIZE][TEX_SIZE];

    // Initialize the texture with random values
    Texture() {
        omp_set_nested(1);
     #pragma omp parallel for 
        for (int i = 0; i < TEX_SIZE; i++) {
            #pragma omp parallel for 
            for (int j = 0; j < TEX_SIZE; j++) {
                pixels[i][j] = static_cast<float>(rand() % 256) / 255.0f;
            }
        }
    }
};

// Class to represent a 3D object in the game (e.g., a player or an environment element)
class GameObject {
public:
    Vector3 position;
    Vector3 rotation;
    Texture texture;

    // Constructor to initialize a game object with random position and rotation
    GameObject() {
        position = Vector3(static_cast<float>(rand() % GRID_SIZE), 
                           static_cast<float>(rand() % GRID_SIZE), 
                           static_cast<float>(rand() % GRID_SIZE));

        rotation = Vector3(static_cast<float>(rand() % 360), 
                           static_cast<float>(rand() % 360), 
                           static_cast<float>(rand() % 360));
    }

    // Update position based on rotation (dummy transformation)
    void updatePosition() {
        position.x += cosf(rotation.x) * 0.1f;
        position.y += sinf(rotation.y) * 0.1f;
        position.z += cosf(rotation.z) * 0.1f;
    }
};

// Class to represent lighting in the game
class Light {
public:
    Vector3 position;
    float intensity;

    // Constructor to initialize a light source with random position and intensity
    Light() {
        position = Vector3(static_cast<float>(rand() % GRID_SIZE), 
                           static_cast<float>(rand() % GRID_SIZE), 
                           static_cast<float>(rand() % GRID_SIZE));
        intensity = static_cast<float>(rand() % 100) / 100.0f;
    }
};

// Class to represent the game state for each player
class GameState {
public:
    int player_id;
    std::vector<GameObject> objects;
    std::vector<Light> lights;

    GameState(int id) : player_id(id) {
        objects.resize(OBJECT_COUNT);
        lights.resize(LIGHT_COUNT);
    }

   float computeLighting(GameObject &obj, Light &light) {
   
    {
        float dx = obj.position.x - light.position.x;
        float dy = obj.position.y - light.position.y;
        float dz = obj.position.z - light.position.z;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        return light.intensity / (distance + 1.0f);
    }
}
    void updateGameState() {
        #pragma omp parallel for
        for (size_t obj_idx = 0; obj_idx < objects.size(); obj_idx++) {
            GameObject &obj = objects[obj_idx];
            obj.updatePosition();

            float total_light = 0.0f;
            #pragma omp parallel for reduction(+:total_light)
            for (size_t light_idx = 0; light_idx < lights.size(); light_idx++) {
                total_light += computeLighting(obj, lights[light_idx]);
            }

            #pragma omp parallel for collapse(2)
            for (int ti = 0; ti < TEX_SIZE; ti++) {
                for (int tj = 0; tj < TEX_SIZE; tj++) {
                    obj.texture.pixels[ti][tj] *= total_light;
                }
            }
        }
    }

    void printGameState() const {
        std::cout << "Player ID: " << player_id << std::endl;
        for (size_t i = 0; i < objects.size(); i++) {
            const GameObject &obj = objects[i];
            std::cout << "Object " << i << ": Position (" << obj.position.x << ", " 
                      << obj.position.y << ", " << obj.position.z << ")" << std::endl;
        }
    }
};
// Main function to simulate the game loop
int main() {
    std::vector<GameState> game_states;

    // Initialize game state for all players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game_states.emplace_back(i);
    }
  
    // Main game loop (sequential)
    for (int frame = 0; frame < 10; frame++) { // Simulating 10 frames
           double start = omp_get_wtime();  // strat the clock
        for (auto &state : game_states) {
            state.updateGameState();

            // Print game state (simulating rendering)
            state.printGameState();
        }
        double end = omp_get_wtime(); // end clock
        printf("Wall-clock time taken: %f seconds\n", end - start);
       
    }

    return 0;
}
