/**
 * @file main.cpp
 * @brief OpenGL Graphics Engine - Main Application
    struct AppState {
        bool wireframe = false;
        bool use_texture = false;
        bool running = true;
        float delta_time = 0.0f;
        float last_frame = 0.0f;
    } app_state_;or Modular Architecture
 * @version 2.0
 *
 * Nueva aplicación principal usando arquitectura modular completa.
 * Separación limpia de responsabilidades entre sistemas.
 */

#include <iostream>
#include <memory>
#include <chrono>
#include <random>

// Core System
#include "core/opengl_context.h"

// Graphics Systems
#include "graphics/shaders/shader_manager.h"
#include "graphics/textures/texture_manager.h"
#include "graphics/rendering/buffer_objects.h"
#include "graphics/skybox/skybox.h"
#include "graphics/lighting/light_manager.h"

// Scene System
#include "scene/mesh.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/chunked_terrain.h"
#include "scene/model.h"
#include "utils/assimp_loader.h"

// Input System
#include "input/input_manager.h"

// UI System
#include "ui/bank_angle.h"
#include "ui/pitch_ladder.h"

// Physics System
#include "physics/flight_dynamics.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Graphics::Core;
using namespace Graphics::Shaders;
using namespace Graphics::Textures;
using namespace Graphics::Rendering;
using namespace Graphics::Lighting;
using namespace UI;
using namespace Graphics::Skybox;
using namespace Scene;
using namespace Input;

/**
 * @class GraphicsEngine
 * @brief Clase principal del motor gráfico modular
 */
class GraphicsEngine
{
private:
    // Core Systems
    std::unique_ptr<OpenGLContext> context_;

    // Scene Objects
    std::unique_ptr<Mesh> cube_mesh_;
    std::unique_ptr<Model> plane_model_;
    std::unique_ptr<CameraController> camera_controller_;
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<ChunkedTerrain> chunked_terrain_;

    // Lighting System
    std::unique_ptr<LightManager> light_manager_;

    // UI Systems
    std::unique_ptr<BankAngleIndicator> bank_angle_indicator_;
    std::unique_ptr<PitchLadder> pitch_ladder_;

    // Physics System
    std::unique_ptr<Physics::FlightDynamicsManager> flight_dynamics_;

    // Application State
    struct AppState
    {
        bool wireframe_mode = false;
        bool use_texture = true;
        bool fog_enabled = true;
        bool running = true;
        float delta_time = 0.0f;
        float last_frame = 0.0f;
        int terrain_size = 3;
        bool use_textured_terrain = true;
    } app_state_;

    // Third-person camera state
    bool third_person_mode_ = false;
    float third_person_distance_ = 80.0f; // distancia detrás del avión (alejada)
    float third_person_height_ = 10.0f;   // altura por encima del avión
    // Sin offsets de orientación del modelo: la orientación proviene del import (Assimp)

    struct AircraftState
    {
        glm::vec3 position = glm::vec3(0.0f, 50.0f, 0.0f);
        glm::vec3 velocity = glm::vec3(0.0f, 0.0f, -50.0f);
    } aircraft_;

    struct InputState
    {
        bool g_pressed = false;
        bool t_pressed = false;
        bool r_pressed = false;
        bool e_pressed = false;
        bool f_pressed = false;
        bool f1_pressed = false;
        bool num2_pressed = false;
        bool c_pressed = false;
        bool x_pressed = false; // (sin uso)
        bool y_pressed = false; // (sin uso)
        bool j_pressed = false; // toggle joystick
    } input_state_;

public:
    GraphicsEngine() = default;
    ~GraphicsEngine() = default;

    /**
     * @brief Maneja el cambio de tamaño de ventana
     */
    void handleWindowResize(int width, int height)
    {
        if (camera_controller_)
        {
            Camera *camera = camera_controller_->getActiveCamera();
            if (camera)
            {
                // Actualizar aspect ratio de la cámara
                float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
                camera->setAspectRatio(new_aspect_ratio);
            }
        }

        // Actualizar HUD
        if (bank_angle_indicator_)
        {
            bank_angle_indicator_->updateScreenSize(width, height);
        }
        if (pitch_ladder_)
        {
            pitch_ladder_->updateScreenSize(width, height);
        }
    }

    /**
     * @brief Inicializa todos los sistemas del motor
     */
    bool initialize()
    {
        std::cout << "=== Initializing OpenGL Graphics Engine ===" << std::endl;

        // 1. Inicializar contexto OpenGL
        if (!initializeOpenGL())
        {
            return false;
        }

        // 2. Inicializar sistemas gráficos
        if (!initializeGraphicsSystems())
        {
            return false;
        }

        // 3. Inicializar sistema de entrada
        if (!initializeInputSystem())
        {
            return false;
        }

        // 4. Inicializar escena
        if (!initializeScene())
        {
            return false;
        }

        // 5. Inicializar física de vuelo
        if (!initializePhysics())
        {
            return false;
        }

        // 6. Inicializar HUD
        if (!initializeUI())
        {
            return false;
        }

        std::cout << "=== Engine initialized successfully! ===" << std::endl;
        return true;
    }

    /**
     * @brief Ejecuta el loop principal de la aplicación
     */
    void run()
    {
        std::cout << "\n=== Starting main loop ===" << std::endl;
        printControls();

        while (!context_->shouldClose() && app_state_.running)
        {
            updateTiming();
            processInput();
            update();
            render();

            context_->swapBuffers();
            context_->pollEvents();
        }

        shutdown();
    }

private:
    /**
     * @brief Inicializa el contexto OpenGL
     */
    bool initializeOpenGL()
    {
        WindowConfig config;
        config.width = 1920;  // Resolución común de pantalla completa
        config.height = 1080;
        config.title = "Flight Simulator - Physics-based Flight Dynamics";
        config.fullscreen = true;  // Activar pantalla completa
        config.vsync = true;

        context_ = std::make_unique<OpenGLContext>();

        if (!context_->initialize(config))
        {
            std::cerr << "Failed to initialize OpenGL context" << std::endl;
            return false;
        }

        // Configuraciones de OpenGL
        context_->enableDepthTest(true);
        context_->enableFaceCulling(false); // Desactivado para ver el cubo completo

        // Configurar callback de resize para mantener aspect ratio
        context_->setResizeCallback([this](int width, int height)
                                    { handleWindowResize(width, height); });

        return true;
    }

    /**
     * @brief Inicializa los sistemas gráficos (shaders, texturas, etc.)
     */
    bool initializeGraphicsSystems()
    {
        auto &shader_manager = ShaderManager::getInstance();
        auto &texture_manager = TextureManager::getInstance();

        // Cargar shaders
        if (!shader_manager.loadShader("basic_3d", "shaders/vertex_3d.glsl", "shaders/fragment_3d.glsl"))
        {
            std::cerr << "Failed to load basic 3D shader" << std::endl;
            return false;
        }

        // Cargar shader para instancing
        if (!shader_manager.loadShader("instanced_3d", "shaders/vertex_instanced.glsl", "shaders/fragment_instanced.glsl"))
        {
            std::cerr << "Failed to load instanced 3D shader" << std::endl;
            return false;
        }

        // Cargar shader para terreno facetado verde
        if (!shader_manager.loadShader("terrain_faceted_green", "shaders/vertex_terrain_faceted.glsl", "shaders/fragment_terrain_faceted.glsl"))
        {
            std::cerr << "Failed to load terrain faceted green shader" << std::endl;
            return false;
        }

        // Cargar texturas
        if (!texture_manager.loadTexture2D("container", "textures/container.jpg", true))
        {
            std::cout << "Warning: Could not load container texture, using procedural texture" << std::endl;
        }

        // Cargar textura del terrain
        if (!texture_manager.loadTexture2D("terrain", "textures/terrain/terrain.jpg", true))
        {
            std::cout << "Warning: Could not load terrain texture, using fallback" << std::endl;
        }

        // Crear textura procedural como fallback
        if (!texture_manager.createProceduralTexture("fallback", 64, 64, 255, 128, 0, 255))
        {
            std::cerr << "Failed to create procedural texture" << std::endl;
            return false;
        }

        // Inicializar sistema de iluminación
        if (!initializeLighting())
        {
            std::cerr << "Failed to initialize lighting system" << std::endl;
            return false;
        }

        return true;
    }

    /**
     * @brief Inicializa el sistema de iluminación
     */
    bool initializeLighting()
    {
        light_manager_ = std::make_unique<LightManager>();

        // Crear luz solar (luz direccional principal)
        DirectionalLight sun = DirectionalLight::createSunlight();
        light_manager_->addDirectionalLight(std::move(sun));

        std::cout << "Lighting system initialized successfully" << std::endl;
        std::cout << "  - Directional light (Sun) created" << std::endl;

        return true;
    }

    /**
     * @brief Inicializa el sistema de entrada
     */
    bool initializeInputSystem()
    {
        auto &input_manager = InputManager::getInstance();

        if (!input_manager.initialize(context_->getWindow()))
        {
            std::cerr << "Failed to initialize input system" << std::endl;
            return false;
        }

        // Configurar callbacks de entrada
        setupInputCallbacks();

        return true;
    }

    /**
     * @brief Inicializa la escena (cámara, objetos, etc.)
     */
    bool initializeScene()
    {
        // Crear mesh del cubo
        cube_mesh_ = MeshFactory::createCube(1.0f, "main_cube");
        if (!cube_mesh_)
        {
            std::cerr << "Failed to create cube mesh" << std::endl;
            return false;
        }

        // Crear terreno por chunks (cada chunk del tamaño definido en TerrainConfig)
        chunked_terrain_ = std::make_unique<ChunkedTerrain>("world_terrain");
        {
            // Tomamos los parámetros por defecto de TerrainConfig como base del chunk
            Scene::TerrainConfig base_cfg; // usa los defaults de terrain.h
            Scene::ChunkedTerrainConfig ctc{};
            ctc.chunk_width = base_cfg.width;
            ctc.chunk_depth = base_cfg.depth;
            ctc.y_position = base_cfg.y_position;
            ctc.width_segments = base_cfg.width_segments;
            ctc.depth_segments = base_cfg.depth_segments;
            ctc.texture_repeat = base_cfg.texture_repeat;
            ctc.use_perlin_noise = base_cfg.use_perlin_noise;
            // Ajustes para más montañas y más grandes (en escala de mundo masivo)
            ctc.noise_scale = 0.0015f;           // más frecuencia (menos liso)
            ctc.height_multiplier = 1000.0f;     // picos más altos
            ctc.noise_octaves = 9;               // más detalle fino
            ctc.noise_seed = base_cfg.noise_seed;
            ctc.view_radius_chunks = 1; // 3x3 chunks alrededor de la cámara

            if (!chunked_terrain_->initialize(ctc))
            {
                std::cerr << "Failed to create chunked terrain" << std::endl;
                return false;
            }
        }

        // Obtener altura del terreno en el origen (donde queremos el cubo)
        float terrain_height_at_origin = chunked_terrain_->getHeightAt(0.0f, 0.0f);

        // Configurar sistema de cámara
        camera_controller_ = std::make_unique<CameraController>();
        camera_controller_->setWindow(context_->getWindow());

        // Crear cámara principal - POSICIONADA SOBRE EL TERRENO
        auto camera_config = CameraController::getFirstPersonConfig();
        camera_config.aspect_ratio = static_cast<float>(context_->getConfig().width) /
                                     static_cast<float>(context_->getConfig().height);

        // Posicionar cámara sobre el terreno + offset
        float camera_height_offset = 15.0f; // Altura extra sobre el terreno
        float camera_x = 0.0f;
        float camera_z = 100.0f; // Un poco atrás del origen
    float camera_terrain_height = chunked_terrain_->getHeightAt(camera_x, camera_z);

        camera_config.position = glm::vec3(camera_x, camera_terrain_height + camera_height_offset, camera_z);
        camera_config.target = glm::vec3(0.0f, terrain_height_at_origin + 5.0f, 0.0f); // Mirar al cubo

        auto camera = std::make_unique<Camera>(camera_config);
        camera_controller_->addCamera(std::move(camera));
        camera_controller_->setActiveCamera(0);

        // Capturar mouse inicialmente
        camera_controller_->setMouseCaptured(true);

        // ===== CARGAR MODELOS =====

        // Cargar avión usando Assimp (modelo GLB con texturas y colores propios)
        {
            plane_model_ = ::Utils::AssimpLoader::loadModel("textures/plane/f16.glb");
            
            if (plane_model_)
            {
                // Posicionar el avión (seguirá la cámara)
                plane_model_->getTransform().position = glm::vec3(0.0f, 5.0f, 0.0f);
                plane_model_->getTransform().scale = glm::vec3(1.0f);
                // Ocultar el avión en primera persona
                plane_model_->setVisible(false);
                std::cout << "Plane model (F16 GLB) loaded successfully with Assimp" << std::endl;
            }
            else
            {
                std::cerr << "Failed to load plane model (F16 GLB) with Assimp" << std::endl;
            }
        }

        // Inicializar skybox
        skybox_ = std::make_unique<Skybox>();
        if (!skybox_->initialize())
        {
            std::cerr << "Failed to initialize skybox" << std::endl;
            return false;
        }

        std::cout << "Scene initialized:" << std::endl;
        std::cout << "  Terrain height at origin: " << terrain_height_at_origin << std::endl;
        std::cout << "  Camera position: (" << camera_config.position.x << ", "
                  << camera_config.position.y << ", " << camera_config.position.z << ")" << std::endl;

        return true;
    }

    /**
     * @brief Inicializar sistema de física de vuelo
     */
    bool initializePhysics()
    {
        flight_dynamics_ = std::make_unique<Physics::FlightDynamicsManager>();
        flight_dynamics_->initialize();

        std::cout << "Flight dynamics initialized successfully" << std::endl;
        return true;
    }

    /**
     * @brief Inicializar sistema de UI/HUD
     */
    bool initializeUI()
    {
        // Obtener dimensiones de ventana
        int width, height;
        glfwGetWindowSize(context_->getWindow(), &width, &height);

        // Crear indicador de bank angle
        bank_angle_indicator_ = std::make_unique<BankAngleIndicator>(width, height);

        if (!bank_angle_indicator_->isInitialized())
        {
            std::cerr << "Failed to initialize Bank Angle HUD" << std::endl;
            return false;
        }

        // Crear pitch ladder
        pitch_ladder_ = std::make_unique<PitchLadder>(width, height);

        if (!pitch_ladder_->isInitialized())
        {
            std::cerr << "Failed to initialize Pitch Ladder HUD" << std::endl;
            return false;
        }

        std::cout << "Bank Angle HUD and Pitch Ladder initialized successfully" << std::endl;
        return true;
    }

    /**
     * @brief Configura los callbacks de entrada
     */
    void setupInputCallbacks()
    {
        auto &input_manager = InputManager::getInstance();

        // Configurar callback de mouse para la cámara
        input_manager.addMouseCallback([this](double xpos, double ypos, double delta_x, double delta_y)
                                       {
            if (camera_controller_->isMouseCaptured()) {
                camera_controller_->mouseCallback(context_->getWindow(), xpos, ypos);
            } });

        // Rueda del mouse: ajustar distancia en tercera persona
        input_manager.addScrollCallback([this](double xoffset, double yoffset)
                                        {
            if (third_person_mode_) {
                third_person_distance_ -= static_cast<float>(yoffset) * 5.0f; // acercar/alejar
                third_person_distance_ = glm::clamp(third_person_distance_, 10.0f, 500.0f);
            }
        });
    }

    /**
     * @brief Actualiza el timing de la aplicación
     */
    void updateTiming()
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        static auto last_time = current_time;

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time);
        app_state_.delta_time = duration.count() / 1000000.0f; // Convertir a segundos

        last_time = current_time;
    }

    /**
     * @brief Procesa la entrada del usuario
     */
    void processInput()
    {
        auto &input_manager = InputManager::getInstance();
        input_manager.update(app_state_.delta_time);

        // Procesar entrada de la cámara
        if (camera_controller_->isMouseCaptured())
        {
            camera_controller_->processInput(app_state_.delta_time);
        }

        // Procesar controles de vuelo
        processFlightControls();

        // Procesar teclas especiales
        processSpecialKeys();
    }

    /**
     * @brief Procesa los controles de vuelo del avión
     */
    void processFlightControls()
    {
        if (!flight_dynamics_)
            return;

        auto &input_manager = InputManager::getInstance();
        
        // Sensibilidades de control (ajustables)
        const float throttle_rate = 0.3f * app_state_.delta_time;  // 30% por segundo
        const float elevator_rate = glm::radians(30.0f) * app_state_.delta_time;  // 30 grados/seg
        const float aileron_rate = glm::radians(45.0f) * app_state_.delta_time;   // 45 grados/seg
        const float rudder_rate = glm::radians(30.0f) * app_state_.delta_time;    // 30 grados/seg
        
        // THROTTLE (Potencia del motor)
        // W - Aumentar potencia
        if (input_manager.isKeyPressed(InputManager::KEY_W))
        {
            flight_dynamics_->adjustThrottle(throttle_rate);
        }
        // S - Disminuir potencia
        if (input_manager.isKeyPressed(InputManager::KEY_S))
        {
            flight_dynamics_->adjustThrottle(-throttle_rate);
        }
        
        // ELEVATOR (Control de Pitch - arriba/abajo)
        // Flecha Arriba - Levantar morro (pitch up)
        if (input_manager.isKeyPressed(InputManager::KEY_UP))
        {
            flight_dynamics_->adjustElevator(-elevator_rate);  // Negativo para pitch up
        }
        // Flecha Abajo - Bajar morro (pitch down)
        if (input_manager.isKeyPressed(InputManager::KEY_DOWN))
        {
            flight_dynamics_->adjustElevator(elevator_rate);   // Positivo para pitch down
        }
        
        // AILERON (Control de Roll - inclinación lateral)
        // Flecha Izquierda - Inclinar a la izquierda
        if (input_manager.isKeyPressed(InputManager::KEY_LEFT))
        {
            flight_dynamics_->adjustAileron(-aileron_rate);  // Negativo para roll left
        }
        // Flecha Derecha - Inclinar a la derecha
        if (input_manager.isKeyPressed(InputManager::KEY_RIGHT))
        {
            flight_dynamics_->adjustAileron(aileron_rate);   // Positivo para roll right
        }
        
        // RUDDER (Control de Yaw - dirección izquierda/derecha)
        // A - Girar a la izquierda
        if (input_manager.isKeyPressed(InputManager::KEY_A))
        {
            flight_dynamics_->adjustRudder(-rudder_rate);  // Negativo para yaw left
        }
        // D - Girar a la derecha
        if (input_manager.isKeyPressed(InputManager::KEY_D))
        {
            flight_dynamics_->adjustRudder(rudder_rate);   // Positivo para yaw right
        }

        // Retorno al centro (amortiguación) cuando no se presionan teclas
        const float damping = 0.95f;  // Factor de amortiguación
        
        // Retornar elevator al centro gradualmente
        if (!input_manager.isKeyPressed(InputManager::KEY_UP) && 
            !input_manager.isKeyPressed(InputManager::KEY_DOWN))
        {
            auto& controls = flight_dynamics_->getControls();
            controls.elevator *= damping;
            if (std::abs(controls.elevator) < 0.001f) controls.elevator = 0.0f;
        }
        
        // Retornar aileron al centro gradualmente
        if (!input_manager.isKeyPressed(InputManager::KEY_LEFT) && 
            !input_manager.isKeyPressed(InputManager::KEY_RIGHT))
        {
            auto& controls = flight_dynamics_->getControls();
            controls.aileron *= damping;
            if (std::abs(controls.aileron) < 0.001f) controls.aileron = 0.0f;
        }
        
        // Retornar rudder al centro gradualmente
        if (!input_manager.isKeyPressed(InputManager::KEY_A) && 
            !input_manager.isKeyPressed(InputManager::KEY_D))
        {
            auto& controls = flight_dynamics_->getControls();
            controls.rudder *= damping;
            if (std::abs(controls.rudder) < 0.001f) controls.rudder = 0.0f;
        }
    }

    /**
     * @brief Procesa teclas especiales (toggles, etc.)
     */
    void processSpecialKeys()
    {
        auto &input_manager = InputManager::getInstance();

        // ESC - Salir
        if (input_manager.isKeyPressed(InputManager::KEY_ESCAPE))
        {
            app_state_.running = false;
        }

        // G - Toggle Wireframe
        if (input_manager.isKeyPressed(InputManager::KEY_G))
        {
            if (!input_state_.g_pressed)
            {
                app_state_.wireframe_mode = !app_state_.wireframe_mode;
                context_->setWireframeMode(app_state_.wireframe_mode);
                std::cout << "Wireframe mode: " << (app_state_.wireframe_mode ? "ON" : "OFF") << std::endl;
                input_state_.g_pressed = true;
            }
        }
        else
        {
            input_state_.g_pressed = false;
        }

        // T - Toggle Texture
        if (input_manager.isKeyPressed(InputManager::KEY_T))
        {
            if (!input_state_.t_pressed)
            {
                app_state_.use_texture = !app_state_.use_texture;
                std::cout << "Texture mode: " << (app_state_.use_texture ? "ON" : "OFF") << std::endl;
                input_state_.t_pressed = true;
            }
        }
        else
        {
            input_state_.t_pressed = false;
        }

        // F - Toggle Fog
        if (input_manager.isKeyPressed(InputManager::KEY_F))
        {
            if (!input_state_.f_pressed)
            {
                app_state_.fog_enabled = !app_state_.fog_enabled;
                std::cout << "Fog: " << (app_state_.fog_enabled ? "ON" : "OFF") << std::endl;
                input_state_.f_pressed = true;
            }
        }
        else
        {
            input_state_.f_pressed = false;
        }

        // R - Reset Camera
        if (input_manager.isKeyPressed(InputManager::KEY_R))
        {
            if (!input_state_.r_pressed)
            {
                Camera *active_camera = camera_controller_->getActiveCamera();
                if (active_camera)
                {
                    active_camera->reset();
                    std::cout << "Camera reset" << std::endl;
                }
                input_state_.r_pressed = true;
            }
        }
        else
        {
            input_state_.r_pressed = false;
        }

        // E - Toggle Mouse Capture
        if (input_manager.isKeyPressed(InputManager::KEY_E))
        {
            if (!input_state_.e_pressed)
            {
                bool captured = !camera_controller_->isMouseCaptured();
                camera_controller_->setMouseCaptured(captured);
                std::cout << "Mouse " << (captured ? "captured" : "released") << std::endl;
                input_state_.e_pressed = true;
            }
        }
        else
        {
            input_state_.e_pressed = false;
        }

        // J - Toggle Joystick controls (Logitech Extreme 3D Pro mapping)
        if (input_manager.isKeyPressed(InputManager::KEY_J))
        {
            if (!input_state_.j_pressed)
            {
                bool use_js = !input_manager.isUsingJoystick();
                input_manager.setUseJoystick(use_js);
                std::cout << "Joystick controls " << (use_js ? "ENABLED" : "DISABLED") << std::endl;
                input_state_.j_pressed = true;
            }
        }
        else
        {
            input_state_.j_pressed = false;
        }

        // F1 - Show/Hide Controls
        if (input_manager.isKeyPressed(InputManager::KEY_1))
        {
            if (!input_state_.f1_pressed)
            {
                printControls();
                input_state_.f1_pressed = true;
            }
        }
        else
        {
            input_state_.f1_pressed = false;
        }

        // 2 - Toggle Terrain Texture Mode
        if (input_manager.isKeyPressed(InputManager::KEY_2))
        {
            if (!input_state_.num2_pressed)
            {
                app_state_.use_textured_terrain = !app_state_.use_textured_terrain;
                std::cout << "Terrain mode: " << (app_state_.use_textured_terrain ? "TEXTURED" : "FACETED GREEN") << std::endl;
                input_state_.num2_pressed = true;
            }
        }
        else
        {
            input_state_.num2_pressed = false;
        }

        // C - Toggle tercera persona
        if (input_manager.isKeyPressed(InputManager::KEY_C))
        {
            if (!input_state_.c_pressed)
            {
                third_person_mode_ = !third_person_mode_;
                
                if (third_person_mode_)
                {
                    std::cout << "Third-person camera: ON" << std::endl;
                    if (plane_model_)
                    {
                        plane_model_->setVisible(true);
                    }
                }
                else
                {
                    std::cout << "First-person camera: ON" << std::endl;
                    if (plane_model_)
                    {
                        plane_model_->setVisible(false);
                    }
                }
                
                input_state_.c_pressed = true;
            }
        }
        else
        {
            input_state_.c_pressed = false;
        }

        // Sin ajustes de orientación del modelo en main
    }

    void update()
    {
        // Actualizar física de vuelo
        if (flight_dynamics_)
        {
            flight_dynamics_->update(app_state_.delta_time);
            
            // Obtener datos de vuelo del modelo físico
            Physics::FlightData flight_data = flight_dynamics_->getFlightData();
            glm::vec3 aircraft_position = flight_dynamics_->getPosition();
            glm::vec3 euler_angles = flight_dynamics_->getEulerAngles();
            
            // Actualizar cámara según la posición y orientación del avión
            Camera *camera = camera_controller_->getActiveCamera();
            if (camera)
            {
                if (third_person_mode_)
                {
                    // Modo tercera persona: posicionar avión y cámara
                    if (plane_model_)
                    {
                        auto &t = plane_model_->getTransform();
                        t.position = aircraft_position;
                        t.rotation.x = glm::radians(euler_angles.x);  // pitch
                        t.rotation.y = glm::radians(euler_angles.y);  // yaw
                        t.rotation.z = glm::radians(euler_angles.z);  // roll
                        
                        // Calcular posición de la cámara detrás del avión
                        glm::mat4 aircraft_transform = glm::mat4(1.0f);
                        aircraft_transform = glm::translate(aircraft_transform, aircraft_position);
                        aircraft_transform = glm::rotate(aircraft_transform, glm::radians(euler_angles.y), glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
                        aircraft_transform = glm::rotate(aircraft_transform, glm::radians(euler_angles.x), glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
                        aircraft_transform = glm::rotate(aircraft_transform, glm::radians(euler_angles.z), glm::vec3(0.0f, 0.0f, 1.0f)); // roll
                        
                        // Offset de cámara en coordenadas del avión (detrás y arriba)
                        glm::vec3 camera_offset = glm::vec3(0.0f, third_person_height_, third_person_distance_);
                        glm::vec4 camera_pos_world = aircraft_transform * glm::vec4(camera_offset, 1.0f);
                        
                        camera->setPosition(glm::vec3(camera_pos_world));
                        camera->setRotation(euler_angles.y, euler_angles.x, euler_angles.z);
                    }
                }
                else
                {
                    // Modo primera persona: cámara en posición del avión
                    camera->setPosition(aircraft_position);
                    camera->setRotation(euler_angles.y, euler_angles.x, euler_angles.z);
                    
                    if (plane_model_)
                    {
                        plane_model_->setVisible(false);
                    }
                }
            }
        }
    }

    /**
     * @brief Renderiza la escena
     */
    void render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera *camera = camera_controller_->getActiveCamera();
        if (!camera)
        {
            std::cerr << "Missing camera!" << std::endl;
            return;
        }

        glm::mat4 view_matrix = camera->getViewMatrix();
        glm::mat4 projection_matrix = camera->getProjectionMatrix();
        glm::vec3 camera_pos = camera->getPosition();

        // Renderizar skybox primero (debe estar en el fondo)
        if (skybox_)
        {
            glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix));
            skybox_->render(skybox_view, projection_matrix, app_state_.fog_enabled);
        }

        // Obtener shader y texture manager para objetos normales
        auto &shader_manager = ShaderManager::getInstance();
        auto &texture_manager = TextureManager::getInstance();

        Shader *shader = shader_manager.getShader("basic_3d");

        if (!shader || !camera)
        {
            std::cerr << "Missing shader or camera!" << std::endl;
            return;
        }

        shader->use();

        shader->setMat4("view", view_matrix);
        shader->setMat4("projection", projection_matrix);
        shader->setBool("useTexture", app_state_.use_texture);
        shader->setVec3("viewPos", camera_pos);

        // Configurar niebla
        shader->setBool("fogEnabled", app_state_.fog_enabled);
        shader->setFloat("fogDensity", 0.0001f); // niebla más blanda en objetos/terreno
        // shader->setVec3("fogColor", glm::vec3(0.7f, 0.8f, 0.9f));
        shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

        // Aplicar sistema de iluminación
        if (light_manager_)
        {
            light_manager_->applyToShader(shader);
        }

        // Renderizar terreno por chunks
        if (chunked_terrain_)
        {
            // Seleccionar shader según modo de terreno
            Shader *terrain_shader = shader;
            if (!app_state_.use_textured_terrain)
            {
                // Usar shader facetado verde
                terrain_shader = shader_manager.getShader("terrain_faceted_green");
                if (!terrain_shader)
                {
                    terrain_shader = shader; // Fallback a basic_3d
                }
            }

            terrain_shader->use();
            terrain_shader->setMat4("view", view_matrix);
            terrain_shader->setMat4("projection", projection_matrix);
            terrain_shader->setVec3("viewPos", camera_pos);
            terrain_shader->setBool("fogEnabled", app_state_.fog_enabled);
            terrain_shader->setFloat("fogDensity", 0.00006f); // niebla más blanda en terreno
            terrain_shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

            // Configurar iluminación (solo para shader textured)
            if (app_state_.use_textured_terrain)
            {
                light_manager_->applyToShader(terrain_shader);
            }
            else
            {
                // Para terreno facetado, solo configurar luz direccional manualmente
                if (light_manager_->getMainLight())
                {
                    auto main_light = light_manager_->getMainLight();
                    terrain_shader->setVec3("dirLight.direction", main_light->getDirection());
                    terrain_shader->setVec3("dirLight.ambient", main_light->getAmbient());
                    terrain_shader->setVec3("dirLight.diffuse", main_light->getDiffuse());
                    terrain_shader->setVec3("dirLight.specular", main_light->getSpecular());
                    terrain_shader->setBool("dirLight.enabled", true);
                }
            }

            // Configurar textura del terrain (solo si es texturado)
            if (app_state_.use_texture && app_state_.use_textured_terrain)
            {
                Texture *terrain_texture = texture_manager.getTexture("terrain");
                if (!terrain_texture)
                {
                    terrain_texture = texture_manager.getTexture("fallback");
                }

                if (terrain_texture)
                {
                    terrain_texture->bind(0);
                    terrain_shader->setInt("ourTexture", 0);
                }
            }

            // Desactivar color uniforme para el terreno
            terrain_shader->setBool("useUniformColor", false);

            // Actualizar y dibujar chunks (modelo identidad)
            glm::mat4 terrain_model = glm::mat4(1.0f);
            terrain_shader->setMat4("model", terrain_model);

            // Actualizar grid de chunks alrededor de la cámara
            chunked_terrain_->update(camera_pos);
            chunked_terrain_->draw();
        }

        // Renderizar cubo
        if (cube_mesh_ && chunked_terrain_)
        {
            shader->use();
            
            // Desactivar color uniforme para el cubo
            shader->setBool("useUniformColor", false);

            // Configurar textura del cubo
            if (app_state_.use_texture)
            {
                Texture *cube_texture = texture_manager.getTexture("container");
                if (!cube_texture)
                {
                    cube_texture = texture_manager.getTexture("fallback");
                }

                if (cube_texture)
                {
                    cube_texture->bind(0);
                    shader->setInt("ourTexture", 0);
                }
            }

            // Posicionar el cubo SOBRE el terreno
            float cube_x = 0.0f;
            float cube_z = 0.0f;
            float terrain_height = chunked_terrain_->getHeightAt(cube_x, cube_z);
            float cube_size = 4.0f;                    // Tamaño del cubo escalado
            float cube_y = terrain_height + cube_size; // Cubo sentado sobre el terreno

            // Configurar matriz modelo para el cubo
            glm::mat4 cube_model = glm::mat4(1.0f);
            cube_model = glm::translate(cube_model, glm::vec3(cube_x, cube_y, cube_z));
            cube_model = glm::scale(cube_model, glm::vec3(cube_size));
            shader->setMat4("model", cube_model);

            cube_mesh_->draw();
            shader->unuse();
        }

        // Renderizar avión (solo en tercera persona)
        if (plane_model_ && plane_model_->isVisible())
        {
            // Reutilizamos el shader básico ya configurado (view/projection/fog/lights)
            plane_model_->render(shader);
        }

        // Renderizar HUD (siempre en primera persona)
        if (bank_angle_indicator_ && bank_angle_indicator_->isInitialized())
        {
            float roll_angle = camera->getRoll();
            bank_angle_indicator_->render(roll_angle);
        }

        if (pitch_ladder_ && pitch_ladder_->isInitialized())
        {
            float pitch_angle = camera->getPitch();
            pitch_ladder_->render(pitch_angle);
        }
    }

    /**
     * @brief Limpia y libera recursos
     */
    void shutdown()
    {
        std::cout << "\n=== Shutting down engine ===" << std::endl;

        // Limpiar sistemas
        auto &input_manager = InputManager::getInstance();
        input_manager.shutdown();

        auto &shader_manager = ShaderManager::getInstance();
        shader_manager.clear();

        auto &texture_manager = TextureManager::getInstance();
        texture_manager.clear();

        // Limpiar objetos de escena
        cube_mesh_.reset();
        plane_model_.reset();
        camera_controller_.reset();
        skybox_.reset();
    chunked_terrain_.reset();

        // Limpiar contexto
        context_.reset();

        std::cout << "Engine shutdown complete" << std::endl;
    }

    /**
     * @brief Imprime los controles disponibles
     */
    void printControls() const
    {
        std::cout << "\n===== FLIGHT SIMULATOR CONTROLS =====" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "FLIGHT CONTROLS (Physics-based):" << std::endl;
        std::cout << "W / S         : Throttle Up / Down" << std::endl;
        std::cout << "UP / DOWN     : Pitch Up / Down (Elevator)" << std::endl;
        std::cout << "LEFT / RIGHT  : Roll Left / Right (Aileron)" << std::endl;
        std::cout << "A / D         : Yaw Left / Right (Rudder)" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "CAMERA & VIEW:" << std::endl;
        std::cout << "C             : Toggle third-person camera" << std::endl;
        std::cout << "E             : Toggle mouse capture" << std::endl;
        std::cout << "R             : Reset camera" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "GRAPHICS:" << std::endl;
        std::cout << "G             : Toggle wireframe" << std::endl;
        std::cout << "T             : Toggle texture" << std::endl;
        std::cout << "F             : Toggle fog" << std::endl;
        std::cout << "2             : Toggle terrain mode (textured vs faceted)" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "INFO:" << std::endl;
        std::cout << "1             : Show controls" << std::endl;
        std::cout << "ESC           : Exit" << std::endl;
        std::cout << "J             : Toggle joystick controls (Logitech Extreme 3D Pro)" << std::endl;
        std::cout << "======================================" << std::endl;
    }
};

/**
 * @brief Punto de entrada principal
 */
int main()
{
    try
    {
        GraphicsEngine engine;

        if (!engine.initialize())
        {
            std::cerr << "Failed to initialize graphics engine" << std::endl;
            return -1;
        }

        engine.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return -1;
    }

    return 0;
}

/*
para la niebla usar la fución
float fog_density = 0.05f;

exp(-pow(distance*fog_density, 1.6))

esta lógica la tenemos que hacer en los shaders.

esto tenemos que hacer para que los elementos que tenemos en el modelo, pero queda mal que
solamente los elementos del mapa se vea la niebla y el cielo se vea nitido
para eso tenemos que aplicarle niebla también al skybox a la parte que se acerca al horizonte
-----------------------------------------------------------------
Para graficar muchas copias del mismo elemento pero con pequeñas variaciones en las posiciones usamos
instancing. La idea para este simulador es dibujar muchos arboles en las distintas zonas del mapa
glDrawElementsInstanced

tener un add_instance_data que haga la instancia de todos lo elementos en el buffer
-----------------------------------------------------------------
para las letras y números con un atlas de letras. Tener una textura con los caracteres que voy a usar
archivo .font con los datos de el char id , la posición en el archivo para que se saquen y se puedan poner en
el cuadrado que se va a dibujar.

Tener un textMesh con un maximo de caracteres que se pueden dibujar para guardarlo en la memoria y no
calcular todo el tiempo el buffer.

bill boarding
-----------------------------------------------------------------
modelo de la nave Assimp

-----------------------------------------------------------------
usar mip maps
facetado de triangulos color verde

*/