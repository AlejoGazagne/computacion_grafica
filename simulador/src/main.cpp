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
#include "scene/model.h"
#include "scene/instancing.h"
#include "utils/obj_loader.h"

// Input System
#include "input/input_manager.h"

// UI System
#include "ui/hud.h"
#include "ui/bank_angle.h"
#include "ui/pitch_ladder.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    std::unique_ptr<Model> tree_model_;
    std::unique_ptr<Model> plane_model_;
    std::unique_ptr<CameraController> camera_controller_;
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Terrain> terrain_;

    // Lighting System
    std::unique_ptr<LightManager> light_manager_;

    // UI Systems
    UI::HUD hud_;
    // Referencias no propietarias para acceso rápido a instrumentos específicos
    UI::BankAngleIndicator *bank_angle_indicator_;
    UI::PitchLadder *pitch_ladder_;

    enum class CameraMode
    {
        THIRD_PERSON = 0,
        FIRST_PERSON = 1,
        CINEMATIC_LATERAL = 2
    };

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
        bool use_billboards = true;
        bool use_textured_terrain = true;
        CameraMode camera_mode = CameraMode::THIRD_PERSON;
    } app_state_;

    struct AircraftState
    {
        glm::vec3 position = glm::vec3(0.0f, 50.0f, 0.0f);
        glm::vec3 velocity = glm::vec3(0.0f, 0.0f, -50.0f);
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 euler_deg = glm::vec3(0.0f);
    } aircraft_;

    struct ThirdPersonCamState
    {
        glm::vec3 camPos = glm::vec3(0.0f);
        glm::vec3 camLook = glm::vec3(0.0f);
        bool initialized = false;

        static constexpr float TP_DIST_BACK = 30.0f;
        static constexpr float TP_DIST_UP = 10.0f;
        static constexpr float TP_DIST_SIDE = 0.0f;
        static constexpr float TP_LEAD_AHEAD = 10.0f;
        static constexpr float TP_POS_SMOOTH = 0.18f;
        static constexpr float TP_LOOK_SMOOTH = 0.18f;
    } third_person_;

    struct CinematicLateralState
    {
        glm::vec3 anchor = glm::vec3(0.0f);
        glm::vec3 look_target = glm::vec3(0.0f);
        bool initialized = false;
        float lock_timer = 0.0f;

        static constexpr float D_ahead = 200.0f; // Más adelante para efecto cinematográfico
        float S_side = 120.0f;
        float H_align = 0.0f; // Variable para altura aleatoria
        static constexpr float D_rear_threshold = 150.0f;
        static constexpr float relock_seconds = 1.2f;
        static constexpr float look_smooth = 0.12f;
        static constexpr float eps_perp = 0.25f;
    } cinematic_;

    struct InputState
    {
        bool g_pressed = false;
        bool t_pressed = false;
        bool r_pressed = false;
        bool e_pressed = false;
        bool f_pressed = false;
        bool f1_pressed = false;
        bool y_pressed = false;
        bool num2_pressed = false;
        bool c_pressed = false;
        bool x_pressed = false;
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

        // Actualizar HUD - propaga el cambio de tamaño a todos los instrumentos
        hud_.updateScreenSize(width, height);
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

        // 5. Inicializar HUD
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
        config.width = 1024;
        config.height = 768;
        config.title = "OpenGL Graphics Engine - Modular Architecture";
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

        // Crear terrain
        terrain_ = std::make_unique<Terrain>("main_terrain");
        if (!terrain_->initialize())
        {
            std::cerr << "Failed to create terrain" << std::endl;
            return false;
        }

        // Obtener altura del terreno en el origen (donde queremos el cubo)
        float terrain_height_at_origin = terrain_->getHeightAt(0.0f, 0.0f);

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
        float camera_terrain_height = terrain_->getHeightAt(camera_x, camera_z);

        camera_config.position = glm::vec3(camera_x, camera_terrain_height + camera_height_offset, camera_z);
        camera_config.target = glm::vec3(0.0f, terrain_height_at_origin + 5.0f, 0.0f); // Mirar al cubo

        auto camera = std::make_unique<Camera>(camera_config);
        camera_controller_->addCamera(std::move(camera));
        camera_controller_->setActiveCamera(0);

        // Capturar mouse inicialmente
        camera_controller_->setMouseCaptured(true);

        // ===== CARGAR MODELOS =====

        // Cargar árbol con instancing
        tree_model_ = std::make_unique<Model>("tree_forest");
        {
            auto tree_data = ::Utils::OBJLoader::loadOBJ("textures/tree/Tree.obj");
            if (!tree_data.vertices.empty())
            {
                auto tree_mesh = std::make_unique<Mesh>(tree_data.vertices, tree_data.indices, "tree_mesh");

                // Generar instancias de árboles
                auto instances = InstanceGenerator::generateTreeInstances(
                    InstanceConfig::TREE_INSTANCE_COUNT,
                    terrain_->getConfig().width,
                    terrain_->getConfig().depth,
                    true, // enable LOD
                    InstanceConfig::LOD_DISTANCE);

                // Convertir a InstanceAttributes
                std::vector<InstanceAttributes> inst_attrs;
                inst_attrs.reserve(instances.size());
                for (const auto &inst : instances)
                {
                    inst_attrs.push_back({inst.position,
                                          inst.scale,
                                          inst.rotationY,
                                          inst.billboard});
                }

                tree_mesh->setInstanceData(inst_attrs);
                tree_model_->addMesh(std::move(tree_mesh));
                std::cout << "Tree model initialized with " << instances.size() << " instances" << std::endl;
            }
            else
            {
                std::cerr << "Failed to load tree model" << std::endl;
            }
        }

        // Cargar avión
        plane_model_ = std::make_unique<Model>("player_plane");
        {
            auto plane_data = ::Utils::OBJLoader::loadOBJ("textures/plane/Jet_Lowpoly.obj");
            if (!plane_data.vertices.empty())
            {
                auto plane_mesh = std::make_unique<Mesh>(plane_data.vertices, plane_data.indices, "plane_mesh");
                plane_model_->addMesh(std::move(plane_mesh));

                // Posicionar el avión (seguirá la cámara)
                plane_model_->getTransform().position = glm::vec3(0.0f, 5.0f, 0.0f);
                plane_model_->getTransform().scale = glm::vec3(0.5f);
                std::cout << "Plane model initialized" << std::endl;
            }
            else
            {
                std::cerr << "Failed to load plane model" << std::endl;
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
     * @brief Inicializar sistema de UI/HUD
     */
    bool initializeUI()
    {
        // Obtener dimensiones de ventana
        int width, height;
        glfwGetWindowSize(context_->getWindow(), &width, &height);

        // Crear Bank Angle Indicator
        auto bank_indicator = std::make_unique<BankAngleIndicator>(width, height);
        bank_angle_indicator_ = bank_indicator.get(); // Guardar referencia antes de mover
        hud_.addInstrument(std::move(bank_indicator));

        // Crear Pitch Ladder
        auto pitch_ladder = std::make_unique<PitchLadder>(width, height);
        pitch_ladder_ = pitch_ladder.get(); // Guardar referencia antes de mover
        hud_.addInstrument(std::move(pitch_ladder));

        // Verificar que todos los instrumentos del HUD estén listos
        if (!hud_.allInstrumentsReady())
        {
            std::cerr << "Failed to initialize HUD instruments" << std::endl;
            return false;
        }

        std::cout << "HUD System initialized successfully with "
                  << hud_.getInstrumentCount() << " instruments" << std::endl;
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

        // Procesar teclas especiales
        processSpecialKeys();
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

        // Y - Toggle Billboard LOD
        if (input_manager.isKeyPressed(InputManager::KEY_Y))
        {
            if (!input_state_.y_pressed)
            {
                app_state_.use_billboards = !app_state_.use_billboards;
                std::cout << "Billboard LOD: " << (app_state_.use_billboards ? "ON" : "OFF") << std::endl;
                input_state_.y_pressed = true;
            }
        }
        else
        {
            input_state_.y_pressed = false;
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

        // C - Toggle Camera Mode
        if (input_manager.isKeyPressed(InputManager::KEY_C))
        {
            if (!input_state_.c_pressed)
            {
                app_state_.camera_mode = static_cast<CameraMode>(
                    (static_cast<int>(app_state_.camera_mode) + 1) % 3);

                const char *mode_names[] = {"THIRD PERSON", "FIRST PERSON", "CINEMATIC LATERAL"};
                std::cout << "Camera mode: " << mode_names[static_cast<int>(app_state_.camera_mode)] << std::endl;

                if (app_state_.camera_mode == CameraMode::THIRD_PERSON)
                {
                    third_person_.initialized = false;
                }
                else if (app_state_.camera_mode == CameraMode::CINEMATIC_LATERAL)
                {
                    cinematic_.initialized = false;
                }

                input_state_.c_pressed = true;
            }
        }
        else
        {
            input_state_.c_pressed = false;
        }
    }

    void updateAircraft(float dt)
    {
        Camera *camera = camera_controller_->getActiveCamera();
        if (!camera)
            return;

        // Aircraft sigue la cámara de control (primera persona), pero NO viceversa
        aircraft_.position = camera->getPosition();
        aircraft_.forward = glm::normalize(camera->getFront());
        aircraft_.up = glm::vec3(0.0f, 1.0f, 0.0f);
        aircraft_.right = glm::normalize(glm::cross(aircraft_.up, aircraft_.forward));
        aircraft_.euler_deg = glm::vec3(-camera->getPitch(), camera->getYaw(), camera->getRoll());
    }

    void updateThirdPerson(float dt)
    {
        glm::vec3 U_world(0.0f, 1.0f, 0.0f);
        glm::vec3 Fh = glm::normalize(glm::vec3(aircraft_.forward.x, 0.0f, aircraft_.forward.z));
        glm::vec3 Rh = glm::normalize(glm::cross(U_world, Fh));

        if (!third_person_.initialized)
        {
            third_person_.camPos = aircraft_.position - Fh * third_person_.TP_DIST_BACK + U_world * third_person_.TP_DIST_UP;
            third_person_.camLook = aircraft_.position + aircraft_.forward * third_person_.TP_LEAD_AHEAD;
            third_person_.initialized = true;
            return;
        }

        glm::vec3 pos_target = aircraft_.position - Fh * third_person_.TP_DIST_BACK + U_world * third_person_.TP_DIST_UP + Rh * third_person_.TP_DIST_SIDE;
        glm::vec3 look_target = aircraft_.position + aircraft_.forward * third_person_.TP_LEAD_AHEAD;

        if (third_person_.TP_POS_SMOOTH > 0.0f)
        {
            float alpha = 1.0f - glm::pow(1.0f - third_person_.TP_POS_SMOOTH, dt * 60.0f);
            third_person_.camPos = glm::mix(third_person_.camPos, pos_target, alpha);
        }
        else
        {
            third_person_.camPos = pos_target;
        }

        if (third_person_.TP_LOOK_SMOOTH > 0.0f)
        {
            float alpha = 1.0f - glm::pow(1.0f - third_person_.TP_LOOK_SMOOTH, dt * 60.0f);
            third_person_.camLook = glm::mix(third_person_.camLook, look_target, alpha);
        }
        else
        {
            third_person_.camLook = look_target;
        }
    }

    void updateCinematicLateral(float dt)
    {
        glm::vec3 U_world(0.0f, 1.0f, 0.0f);
        glm::vec3 Fh = glm::normalize(glm::vec3(aircraft_.forward.x, 0.0f, aircraft_.forward.z));
        glm::vec3 Rh = glm::normalize(glm::cross(U_world, Fh));

        if (!cinematic_.initialized)
        {
            // Random para lado (izquierda o derecha)
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> side_dist(-1.0f, 1.0f);
            cinematic_.S_side = (side_dist(gen) > 0.0f) ? 120.0f : -120.0f;

            // Random para altura (-20 a +20 metros respecto al avión)
            std::uniform_real_distribution<float> height_dist(-20.0f, 20.0f);
            cinematic_.H_align = height_dist(gen);

            cinematic_.anchor = aircraft_.position + Rh * cinematic_.S_side + Fh * cinematic_.D_ahead;
            cinematic_.anchor.y = aircraft_.position.y + cinematic_.H_align;
            cinematic_.look_target = aircraft_.position;
            cinematic_.initialized = true;
            cinematic_.lock_timer = 0.0f;
            return;
        }

        cinematic_.anchor.y = aircraft_.position.y + cinematic_.H_align;

        if (cinematic_.lock_timer > 0.0f)
            cinematic_.lock_timer -= dt;

        bool paso = glm::dot(aircraft_.forward, aircraft_.position - cinematic_.anchor) > 0.0f;
        bool lejos = glm::distance(aircraft_.position, cinematic_.anchor) > cinematic_.D_rear_threshold;

        if (paso && lejos && cinematic_.lock_timer <= 0.0f)
        {
            // Random para lado (izquierda o derecha)
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> side_dist(-1.0f, 1.0f);
            cinematic_.S_side = (side_dist(gen) > 0.0f) ? 120.0f : -120.0f;

            // Random para altura (-20 a +20 metros respecto al avión)
            std::uniform_real_distribution<float> height_dist(-20.0f, 20.0f);
            cinematic_.H_align = height_dist(gen);

            cinematic_.anchor = aircraft_.position + Rh * cinematic_.S_side + Fh * cinematic_.D_ahead;
            cinematic_.anchor.y = aircraft_.position.y + cinematic_.H_align;
            cinematic_.lock_timer = cinematic_.relock_seconds;

            // Aceptación: verificar perpendicularidad
            glm::vec3 dir = glm::normalize(aircraft_.position - cinematic_.anchor);
            float perp = glm::abs(glm::dot(dir, Fh));
            // assert(perp <= cinematic_.eps_perp + 0.05f); // Vista lateral, no detrás
        }

        if (cinematic_.look_smooth > 0.0f)
        {
            float alpha = 1.0f - glm::pow(1.0f - cinematic_.look_smooth, dt * 60.0f);
            cinematic_.look_target = glm::mix(cinematic_.look_target, aircraft_.position, alpha);
        }
        else
        {
            cinematic_.look_target = aircraft_.position;
        }
    }

    void update()
    {
        updateAircraft(app_state_.delta_time);

        if (app_state_.camera_mode == CameraMode::THIRD_PERSON)
        {
            updateThirdPerson(app_state_.delta_time);
        }
        else if (app_state_.camera_mode == CameraMode::CINEMATIC_LATERAL)
        {
            updateCinematicLateral(app_state_.delta_time);
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

        glm::mat4 view_matrix;
        glm::mat4 projection_matrix = camera->getProjectionMatrix();
        glm::vec3 camera_pos;

        if (app_state_.camera_mode == CameraMode::THIRD_PERSON)
        {
            glm::vec3 U_world(0.0f, 1.0f, 0.0f);
            view_matrix = glm::lookAt(third_person_.camPos, third_person_.camLook, U_world);
            camera_pos = third_person_.camPos;
        }
        else if (app_state_.camera_mode == CameraMode::CINEMATIC_LATERAL)
        {
            glm::vec3 U_world(0.0f, 1.0f, 0.0f);
            view_matrix = glm::lookAt(cinematic_.anchor, cinematic_.look_target, U_world);
            camera_pos = cinematic_.anchor;
        }
        else
        {
            view_matrix = camera->getViewMatrix();
            camera_pos = camera->getPosition();
        }

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
        shader->setFloat("fogDensity", 0.0001f);
        // shader->setVec3("fogColor", glm::vec3(0.7f, 0.8f, 0.9f));
        shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

        // Aplicar sistema de iluminación
        if (light_manager_)
        {
            light_manager_->applyToShader(shader);
        }

        // Renderizar terrain
        if (terrain_)
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
            terrain_shader->setFloat("fogDensity", 0.0001f);
            terrain_shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

            // Configurar iluminación
            if (app_state_.use_textured_terrain)
            {
                light_manager_->applyToShader(terrain_shader);
            }
            else
            {
                // Para terreno verde simple, configurar luz direccional
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

            // Configurar matriz modelo para el terrain (sin transformaciones)
            glm::mat4 terrain_model = glm::mat4(1.0f);
            terrain_shader->setMat4("model", terrain_model);

            terrain_->draw();
        }

        // Renderizar árboles de prueba (UN ÁRBOL EN POSICIÓN FIJA para verificar que se ve)
        if (tree_model_)
        {
            shader->use();

            // Posición de prueba del árbol (al lado de la caja)
            float cube_x = 0.0f;
            float cube_z = 0.0f;
            float terrain_height_cube = terrain_->getHeightAt(cube_x, cube_z);
            glm::mat4 tree_model = glm::mat4(1.0f);
            tree_model = glm::translate(tree_model, glm::vec3(8.0f, terrain_height_cube, cube_z));
            tree_model = glm::scale(tree_model, glm::vec3(1.0f));

            shader->setMat4("model", tree_model);
            shader->setMat4("view", view_matrix);
            shader->setMat4("projection", projection_matrix);
            shader->setBool("useTexture", app_state_.use_texture);
            shader->setVec3("viewPos", camera_pos);
            shader->setBool("fogEnabled", app_state_.fog_enabled);
            shader->setFloat("fogDensity", 0.0001f);
            shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

            if (light_manager_)
            {
                light_manager_->applyToShader(shader);
            }

            Texture *tree_texture = texture_manager.getTexture("fallback");
            if (tree_texture)
            {
                tree_texture->bind(0);
                shader->setInt("ourTexture", 0);
            }

            // Renderizar solo el mesh (sin instancing, un solo árbol)
            if (tree_model_->getMeshCount() > 0)
            {
                // Acceder al primer mesh y renderizarlo sin instancing
                // Necesitamos exponer un método en Model para esto
                tree_model_->render(shader);
            }

            shader->unuse();
        }

        // Renderizar cubo
        if (cube_mesh_ && terrain_)
        {
            shader->use();

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
            float terrain_height = terrain_->getHeightAt(cube_x, cube_z);
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

        if (plane_model_ && app_state_.camera_mode != CameraMode::FIRST_PERSON)
        {
            // El avión NO depende de la cámara
            // Usa AircraftState independiente
            shader->use();
            plane_model_->getTransform().position = aircraft_.position;
            plane_model_->getTransform().rotation = glm::radians(aircraft_.euler_deg);
            plane_model_->getTransform().scale = glm::vec3(1.0f);
            plane_model_->render(shader);
            shader->unuse();
        }

        if (app_state_.camera_mode == CameraMode::FIRST_PERSON)
        {
            // Actualizar datos de los instrumentos antes de renderizar
            if (bank_angle_indicator_)
            {
                float roll_angle = camera->getRoll();
                bank_angle_indicator_->setBankAngle(roll_angle);
            }

            if (pitch_ladder_)
            {
                float pitch_angle = camera->getPitch();
                pitch_ladder_->setPitch(pitch_angle);
            }

            // Renderizar todo el HUD de una vez
            hud_.render();
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
        tree_model_.reset();
        plane_model_.reset();
        camera_controller_.reset();
        skybox_.reset();
        terrain_.reset();

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
        std::cout << "W        : Thrust forward (accelerate)" << std::endl;
        std::cout << "A        : Roll left (bank left)" << std::endl;
        std::cout << "D        : Roll right (bank right)" << std::endl;
        std::cout << "Mouse ↕  : Pitch (nose up/down)" << std::endl;
        std::cout << "Mouse ↔  : Yaw (turn left/right)" << std::endl;
        std::cout << "SPACE    : Move up" << std::endl;
        std::cout << "SHIFT    : Move down" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "⚡ No pitch limits - Full 360° loops!" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "G        : Toggle wireframe" << std::endl;
        std::cout << "T        : Toggle texture" << std::endl;
        std::cout << "F        : Toggle fog" << std::endl;
        std::cout << "Y        : Toggle billboard LOD (trees)" << std::endl;
        std::cout << "2        : Toggle terrain mode (textured vs faceted green)" << std::endl;
        std::cout << "R        : Reset camera" << std::endl;
        std::cout << "E        : Toggle mouse capture" << std::endl;
        std::cout << "1        : Show controls" << std::endl;
        std::cout << "ESC      : Exit" << std::endl;
        std::cout << "==============================" << std::endl;
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