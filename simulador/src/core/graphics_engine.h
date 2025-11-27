/**
 * @file graphics_engine.h
 * @brief OpenGL Graphics Engine - Main Engine Class
 * @version 2.0
 *
 * Motor gráfico principal con arquitectura modular completa.
 * Separación limpia de responsabilidades entre sistemas.
 */

#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

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
#include "graphics/rendering/shadow_map.h"
#include "graphics/skybox/skybox.h"
#include "graphics/lighting/light_manager.h"

// Scene System
#include "scene/mesh.h"
#include "scene/model.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/chunked_terrain.h"

// Input System
#include "input/input_manager.h"

// HUD System
#include "hud/instruments/bank_angle.h"
#include "hud/instruments/pitch_ladder.h"
#include "hud/instruments/altimeter.h"
#include "hud/instruments/speed_indicator.h"
#include "hud/instruments/vertical_speed_indicator.h"
#include "hud/instruments/waypoint_indicator.h"
#include "hud/huddef.h"

// Mission System
#include "mission/mission_definition.h"
#include "mission/mission_runtime.h"
#include "mission/waypoint_system.h"

// Physics System
#include "physics/flight_dynamics.h"

// Utils
#include "utils/model_loader.h"

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
using namespace hud;
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
  std::unique_ptr<Scene::Model> aircraft_model_;
  std::unique_ptr<CameraController> camera_controller_;
  std::unique_ptr<Skybox> skybox_;
  std::unique_ptr<ChunkedTerrain> chunked_terrain_;

  // Lighting System
  std::unique_ptr<LightManager> light_manager_;

  // Shadow Mapping System
  std::unique_ptr<Graphics::Rendering::ShadowMap> shadow_map_;

  // UI Systems
  std::unique_ptr<BankAngleIndicator> bank_angle_indicator_;
  std::unique_ptr<PitchLadder> pitch_ladder_;
  std::unique_ptr<Altimeter> altimeter_;
  std::unique_ptr<SpeedIndicator> speed_indicator_;
  std::unique_ptr<VerticalSpeedIndicator> vsi_;
  std::unique_ptr<WaypointIndicator> waypoint_indicator_;

  // Mission System
  std::unique_ptr<Mission::MissionRuntime> mission_runtime_;
  std::unique_ptr<Mission::WaypointSystem> waypoint_system_;

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
  } app_state_;

  // Parámetros de cámara tercera persona
  bool third_person_mode_ = false;
  float third_person_distance_ = 80.0f;
  float third_person_height_ = 10.0f;

  // Punteros a shaders (no owned)
  Graphics::Shaders::Shader *depth_shader_ = nullptr;

  // Parámetros de cámara cinemática
  bool cinematic_mode_ = false;               // Usar cámara cinemática en lugar de tercera persona
  float cinematic_forward_distance_ = 300.0f; // Distancia por delante del avión
  float cinematic_lateral_offset_ = 50.0f;    // Offset lateral (+ = derecha, - = izquierda)
  float cinematic_height_offset_ = 20.0f;     // Offset en altura
  float cinematic_max_distance_ = 350.0f;     // Distancia máxima antes de reposicionar
  float cinematic_smooth_factor_ = 0.0f;      // Factor de suavizado (0=instantáneo, 1=muy suave)

  // Valores de trim para vuelo estable
  float elevator_trim_ = -0.09024f; // Trim de elevador (ajustado para vuelo nivelado)
  float aileron_trim_ = 0.0f;       // Trim de aileron (generalmente 0)
  float rudder_trim_ = 0.0f;        // Trim de rudder (generalmente 0)

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
    bool c_pressed = false;
    bool x_pressed = false;
    bool y_pressed = false;
    bool j_pressed = false;
    bool v_pressed = false;
    bool num3_pressed = false;
    bool num4_pressed = false;
    bool num5_pressed = false;
    bool num6_pressed = false;
    bool num7_pressed = false;
    bool num8_pressed = false;
    bool num9_pressed = false;
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
        float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        camera->setAspectRatio(new_aspect_ratio);
      }
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
    // printControls();

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
    config.width = 1920;
    config.height = 1080;
    config.title = "Flight Simulator - Physics-based Flight Dynamics";
    config.fullscreen = true;
    config.vsync = true;

    context_ = std::make_unique<OpenGLContext>();

    if (!context_->initialize(config))
    {
      std::cerr << "Failed to initialize OpenGL context" << std::endl;
      return false;
    }

    context_->enableDepthTest(true);
    context_->enableFaceCulling(false);

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

    if (!shader_manager.loadShader("basic_3d", "shaders/vertex_3d.glsl", "shaders/fragment_3d.glsl"))
    {
      std::cerr << "Failed to load basic 3D shader" << std::endl;
      return false;
    }

    if (!shader_manager.loadShader("instanced_3d", "shaders/vertex_instanced.glsl", "shaders/fragment_instanced.glsl"))
    {
      std::cerr << "Failed to load instanced 3D shader" << std::endl;
      return false;
    }

    // Shader para shadow mapping (depth pass)
    if (!shader_manager.loadShader("depth", "shaders/depth.vert", "shaders/depth.frag"))
    {
      std::cerr << "Failed to load depth shader for shadow mapping" << std::endl;
      return false;
    }
    depth_shader_ = shader_manager.getShader("depth");

    if (!texture_manager.loadTexture2D("terrain", "textures/terrain/terrain.jpg", true))
    {
      std::cout << "Warning: Could not load terrain texture, using fallback" << std::endl;
    }

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

    // Inicializar shadow mapping
    shadow_map_ = std::make_unique<Graphics::Rendering::ShadowMap>(4096, 4096);
    std::cout << "Shadow Mapping system initialized" << std::endl;

    return true;
  }

  /**
   * @brief Inicializa el sistema de iluminación
   */
  bool initializeLighting()
  {
    light_manager_ = std::make_unique<LightManager>();

    DirectionalLight sun = DirectionalLight::createSunlight();
    light_manager_->setSunlight(std::move(sun));

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
    chunked_terrain_ = std::make_unique<ChunkedTerrain>("world_terrain");
    {
      Scene::TerrainConfig base_cfg;
      Scene::ChunkedTerrainConfig ctc{};
      ctc.chunk_width = base_cfg.width;
      ctc.chunk_depth = base_cfg.depth;
      ctc.y_position = base_cfg.y_position;
      ctc.width_segments = base_cfg.width_segments;
      ctc.depth_segments = base_cfg.depth_segments;
      ctc.texture_repeat = base_cfg.texture_repeat;
      ctc.use_perlin_noise = base_cfg.use_perlin_noise;
      ctc.noise_scale = 0.0015f;
      ctc.height_multiplier = 1000.0f;
      ctc.noise_octaves = 9;
      ctc.noise_seed = base_cfg.noise_seed;
      ctc.view_radius_chunks = 1;

      if (!chunked_terrain_->initialize(ctc))
      {
        std::cerr << "Failed to create chunked terrain" << std::endl;
        return false;
      }
    }

    float terrain_height_at_origin = chunked_terrain_->getHeightAt(0.0f, 0.0f);

    // Configurar sistema de cámara
    camera_controller_ = std::make_unique<CameraController>();
    camera_controller_->setWindow(context_->getWindow());

    auto camera_config = CameraController::getFirstPersonConfig();
    camera_config.aspect_ratio = static_cast<float>(context_->getConfig().width) /
                                 static_cast<float>(context_->getConfig().height);

    float camera_height_offset = 15.0f;
    float camera_x = 0.0f;
    float camera_z = 100.0f;
    float camera_terrain_height = chunked_terrain_->getHeightAt(camera_x, camera_z);

    camera_config.position = glm::vec3(camera_x, camera_terrain_height + camera_height_offset, camera_z);
    camera_config.target = glm::vec3(0.0f, terrain_height_at_origin + 5.0f, 0.0f);

    auto camera = std::make_unique<Camera>(camera_config);

    // Inicializar parámetros de cámara cinemática
    camera->setCinematicParameters(
        cinematic_forward_distance_,
        cinematic_lateral_offset_,
        cinematic_height_offset_,
        cinematic_max_distance_,
        cinematic_smooth_factor_);

    camera_controller_->addCamera(std::move(camera));
    camera_controller_->setActiveCamera(0);

    // Capturar mouse inicialmente
    camera_controller_->setMouseCaptured(true);

    // Inicializar skybox
    skybox_ = std::make_unique<Skybox>();
    if (!skybox_->initialize())
    {
      std::cerr << "Failed to initialize skybox" << std::endl;
      return false;
    }

    // Cargar modelo de avión
    aircraft_model_ = ::Utils::ModelLoader::loadModel(
        "textures/plane/Jet_Lowpoly.obj",
        "jet_aircraft");

    if (!aircraft_model_)
    {
      std::cerr << "Warning: Failed to load aircraft model" << std::endl;
    }
    else
    {
      aircraft_model_->getTransform().scale = glm::vec3(2.0f);
      std::cout << "Aircraft model loaded successfully" << std::endl;
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

    auto &shader_manager = ShaderManager::getInstance();
    if (!shader_manager.loadShader("bank_angle_shader", "shaders/vertex_bank_angle.glsl", "shaders/fragment_bank_angle.glsl"))
    {
      std::cerr << "Failed to load bank angle shader" << std::endl;
      return false;
    }
    if (!shader_manager.loadShader("pitch_ladder_shader", "shaders/vertex_hud.glsl", "shaders/fragment_hud.glsl"))
    {
      std::cerr << "Failed to load pitch ladder shader" << std::endl;
      return false;
    }

    Shader *bank_shader = shader_manager.getShader("bank_angle_shader");
    Shader *ladder_shader = shader_manager.getShader("pitch_ladder_shader");

    bank_angle_indicator_ = std::make_unique<hud::BankAngleIndicator>(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), bank_shader);
    pitch_ladder_ = std::make_unique<hud::PitchLadder>(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), ladder_shader);
    altimeter_ = std::make_unique<hud::Altimeter>(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), ladder_shader);
    speed_indicator_ = std::make_unique<hud::SpeedIndicator>(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), ladder_shader);
    vsi_ = std::make_unique<hud::VerticalSpeedIndicator>(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), ladder_shader);
    waypoint_indicator_ = std::make_unique<hud::WaypointIndicator>(glm::vec2(-0.25f, 0.50f), glm::vec2(0.35f, 0.35f), ladder_shader);

    if (speed_indicator_)
    {
      speed_indicator_->setAnchorLeftX(-0.92f);
      speed_indicator_->setTickLength(0.035f);
      speed_indicator_->setStepNdc(0.038f);
      speed_indicator_->setVisibleSteps(10);
      speed_indicator_->setBoxSize(0.16f, 0.06f);
    }
    if (altimeter_)
    {
      altimeter_->setAnchorRightX(0.92f);
      altimeter_->setTickLength(0.035f);
      altimeter_->setStepNdc(0.038f);
      altimeter_->setVisibleSteps(10);
      altimeter_->setBoxSize(0.18f, 0.06f);
      altimeter_->setChevronWidth(0.015f);
    }
    if (vsi_)
    {
      vsi_->setLineX(0.46f);
      vsi_->setScaleHeight(0.70f);
      vsi_->setTickLengths(0.018f, 0.035f);
      vsi_->setBoxSize(0.09f, 0.040f);
    }

    bank_angle_indicator_->initialize();
    pitch_ladder_->initialize();
    altimeter_->initialize();
    speed_indicator_->initialize();
    vsi_->initialize();
    waypoint_indicator_->initialize();

    mission_runtime_ = std::make_unique<Mission::MissionRuntime>();
    waypoint_system_ = std::make_unique<Mission::WaypointSystem>();
    waypoint_system_->initialize();

    Mission::MissionDefinition test_mission;
    test_mission.id = "nav_01";
    test_mission.name = "Circuito de Navegación Extremo";
    test_mission.description = "Misión de larga distancia: 10 waypoints dispersos en un área de 8000+ metros";
    test_mission.category = "navigation";
    test_mission.difficulty = 4;

    // Waypoint 1: Punto de partida (cerca del origen)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(200.0f, 550.0f, -200.0f), "Alpha"));

    // Waypoint 2: Norte muy lejano (2.5km)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(2500.0f, 700.0f, -400.0f), "Bravo"));

    // Waypoint 3: Noreste extremo (3.5km, alta altitud)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(3200.0f, 850.0f, -2800.0f), "Charlie"));

    // Waypoint 4: Este muy lejano (4km)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(1800.0f, 750.0f, -4000.0f), "Delta"));

    // Waypoint 5: Sureste extremo (3.8km, descenso)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(-800.0f, 600.0f, -3800.0f), "Echo"));

    // Waypoint 6: Sur muy lejano (3.5km, baja altitud)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(-2800.0f, 550.0f, -2500.0f), "Foxtrot"));

    // Waypoint 7: Suroeste extremo (4km)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(-3500.0f, 650.0f, -800.0f), "Golf"));

    // Waypoint 8: Oeste muy lejano (3.8km, alta altitud)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(-3200.0f, 800.0f, 1500.0f), "Hotel"));

    // Waypoint 9: Noroeste lejano (2.5km, máxima altitud)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(-1200.0f, 900.0f, 2800.0f), "India"));

    // Waypoint 10: Retorno al inicio (vuelo de aproximación)
    test_mission.waypoints.push_back(Mission::WaypointDef(
        glm::vec3(100.0f, 600.0f, 300.0f), "Juliet"));

    mission_runtime_->startMission(test_mission);
    waypoint_system_->loadFromMission(test_mission);

    std::cout << "HUD instruments initialized successfully" << std::endl;
    std::cout << "Mission system initialized with test mission" << std::endl;
    return true;
  }

  /**
   * @brief Configura los callbacks de entrada
   */
  void setupInputCallbacks()
  {
    auto &input_manager = InputManager::getInstance();

    input_manager.addMouseCallback([this](double xpos, double ypos, double /*delta_x*/, double /*delta_y*/)
                                   {
            if (camera_controller_->isMouseCaptured()) {
                camera_controller_->mouseCallback(context_->getWindow(), xpos, ypos);
            } });

    input_manager.addScrollCallback([this](double /*xoffset*/, double yoffset)
                                    {
            if (third_person_mode_) {
                third_person_distance_ -= static_cast<float>(yoffset) * 5.0f;
                third_person_distance_ = glm::clamp(third_person_distance_, 10.0f, 500.0f);
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

    const float throttle_rate = 0.3f * app_state_.delta_time;
    const float elevator_rate = glm::radians(30.0f) * app_state_.delta_time;
    const float aileron_rate = glm::radians(45.0f) * app_state_.delta_time;
    const float rudder_rate = glm::radians(30.0f) * app_state_.delta_time;

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

    // Arriba - Abajo: Elevador
    if (input_manager.isKeyPressed(InputManager::KEY_UP))
    {
      flight_dynamics_->adjustElevator(-elevator_rate);
    }
    if (input_manager.isKeyPressed(InputManager::KEY_DOWN))
    {
      flight_dynamics_->adjustElevator(elevator_rate);
    }

    // Izquierda - Derecha: Alerones
    if (input_manager.isKeyPressed(InputManager::KEY_LEFT))
    {
      flight_dynamics_->adjustAileron(-aileron_rate);
    }
    if (input_manager.isKeyPressed(InputManager::KEY_RIGHT))
    {
      flight_dynamics_->adjustAileron(aileron_rate);
    }

    // Izquierda - Derecha: Timon
    if (input_manager.isKeyPressed(InputManager::KEY_A))
    {
      flight_dynamics_->adjustRudder(-rudder_rate);
    }
    if (input_manager.isKeyPressed(InputManager::KEY_D))
    {
      flight_dynamics_->adjustRudder(rudder_rate);
    }

    // Cuando no hay input, los controles vuelven gradualmente al TRIM
    const float return_speed = 0.1f;

    if (!input_manager.isKeyPressed(InputManager::KEY_UP) &&
        !input_manager.isKeyPressed(InputManager::KEY_DOWN))
    {
      auto &controls = flight_dynamics_->getControls();
      // Interpolar gradualmente hacia el valor de trim
      controls.elevator += (elevator_trim_ - controls.elevator) * return_speed;
      // Si está muy cerca del trim, establecerlo exactamente
      if (std::abs(controls.elevator - elevator_trim_) < 0.001f)
        controls.elevator = elevator_trim_;
    }

    if (!input_manager.isKeyPressed(InputManager::KEY_LEFT) &&
        !input_manager.isKeyPressed(InputManager::KEY_RIGHT))
    {
      auto &controls = flight_dynamics_->getControls();
      // Interpolar gradualmente hacia el valor de trim
      controls.aileron += (aileron_trim_ - controls.aileron) * return_speed;
      if (std::abs(controls.aileron - aileron_trim_) < 0.001f)
        controls.aileron = aileron_trim_;
    }

    if (!input_manager.isKeyPressed(InputManager::KEY_A) &&
        !input_manager.isKeyPressed(InputManager::KEY_D))
    {
      auto &controls = flight_dynamics_->getControls();
      // Interpolar gradualmente hacia el valor de trim
      controls.rudder += (rudder_trim_ - controls.rudder) * return_speed;
      if (std::abs(controls.rudder - rudder_trim_) < 0.001f)
        controls.rudder = rudder_trim_;
    }
  }

  /**
   * @brief Procesa teclas especiales (toggles, etc.)
   */
  void processSpecialKeys()
  {
    auto &input_manager = InputManager::getInstance();

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

    // T - Toggle Textures
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

    // J - Toggle Joystick Control
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

    // 1 - Mostrar controles
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

    // Tecla C: Cambiar entre primera persona y tercera persona clásica
    if (input_manager.isKeyPressed(InputManager::KEY_C))
    {
      if (!input_state_.c_pressed)
      {
        third_person_mode_ = !third_person_mode_;

        if (third_person_mode_)
        {
          std::cout << "Third-person camera: ON" << std::endl;
        }
        else
        {
          std::cout << "First-person camera: ON" << std::endl;
        }

        input_state_.c_pressed = true;
      }
    }
    else
    {
      input_state_.c_pressed = false;
    }

    // Tecla V: Cambiar entre cámara cinemática y tercera persona clásica
    if (input_manager.isKeyPressed(InputManager::KEY_V))
    {
      if (!input_state_.v_pressed)
      {
        cinematic_mode_ = !cinematic_mode_;

        if (cinematic_mode_)
        {
          std::cout << "Camera mode: CINEMATIC (spectator)" << std::endl;
        }
        else
        {
          std::cout << "Camera mode: THIRD PERSON (classic)" << std::endl;
        }

        input_state_.v_pressed = true;
      }
    }
    else
    {
      input_state_.v_pressed = false;
    }

    // Tecla 3: Ajustar distancia delantera de cámara cinemática
    if (input_manager.isKeyPressed(InputManager::KEY_3))
    {
      if (!input_state_.num3_pressed)
      {
        cinematic_forward_distance_ += 20.0f;
        cinematic_forward_distance_ = glm::clamp(cinematic_forward_distance_, 50.0f, 300.0f);
        std::cout << "Cinematic forward distance: " << cinematic_forward_distance_ << "m" << std::endl;

        // Actualizar parámetros en la cámara y forzar reposicionamiento
        Camera *cam = camera_controller_->getActiveCamera();
        if (cam)
        {
          cam->setCinematicParameters(cinematic_forward_distance_, cinematic_lateral_offset_,
                                      cinematic_height_offset_, cinematic_max_distance_,
                                      cinematic_smooth_factor_);
        }

        input_state_.num3_pressed = true;
      }
    }
    else
    {
      input_state_.num3_pressed = false;
    }

    // Tecla 4: Ajustar offset lateral de cámara cinemática
    if (input_manager.isKeyPressed(InputManager::KEY_4))
    {
      if (!input_state_.num4_pressed)
      {
        cinematic_lateral_offset_ += 10.0f;
        cinematic_lateral_offset_ = glm::clamp(cinematic_lateral_offset_, -100.0f, 100.0f);
        std::cout << "Cinematic lateral offset: " << cinematic_lateral_offset_ << "m" << std::endl;

        // Actualizar parámetros en la cámara y forzar reposicionamiento
        Camera *cam = camera_controller_->getActiveCamera();
        if (cam)
        {
          cam->setCinematicParameters(cinematic_forward_distance_, cinematic_lateral_offset_,
                                      cinematic_height_offset_, cinematic_max_distance_,
                                      cinematic_smooth_factor_);
        }

        input_state_.num4_pressed = true;
      }
    }
    else
    {
      input_state_.num4_pressed = false;
    }

    // Tecla 5: Ajustar offset de altura de cámara cinemática
    if (input_manager.isKeyPressed(InputManager::KEY_5))
    {
      if (!input_state_.num5_pressed)
      {
        cinematic_height_offset_ += 5.0f;
        cinematic_height_offset_ = glm::clamp(cinematic_height_offset_, 0.0f, 100.0f);
        std::cout << "Cinematic height offset: " << cinematic_height_offset_ << "m" << std::endl;

        // Actualizar parámetros en la cámara y forzar reposicionamiento
        Camera *cam = camera_controller_->getActiveCamera();
        if (cam)
        {
          cam->setCinematicParameters(cinematic_forward_distance_, cinematic_lateral_offset_,
                                      cinematic_height_offset_, cinematic_max_distance_,
                                      cinematic_smooth_factor_);
        }

        input_state_.num5_pressed = true;
      }
    }
    else
    {
      input_state_.num5_pressed = false;
    }

    // Tecla 6: Ajustar distancia máxima de recolocación de cámara cinemática
    if (input_manager.isKeyPressed(InputManager::KEY_6))
    {
      if (!input_state_.num6_pressed)
      {
        cinematic_max_distance_ += 20.0f;
        cinematic_max_distance_ = glm::clamp(cinematic_max_distance_, 100.0f, 500.0f);
        std::cout << "Cinematic max relocate distance: " << cinematic_max_distance_ << "m" << std::endl;

        // Actualizar parámetros en la cámara (NO fuerza reposicionamiento, solo cambia umbral)
        Camera *cam = camera_controller_->getActiveCamera();
        if (cam)
        {
          cam->setCinematicParameters(cinematic_forward_distance_, cinematic_lateral_offset_,
                                      cinematic_height_offset_, cinematic_max_distance_,
                                      cinematic_smooth_factor_);
        }

        input_state_.num6_pressed = true;
      }
    }
    else
    {
      input_state_.num6_pressed = false;
    }

    // Tecla 7: Ajustar trim del elevador hacia arriba (nariz arriba)
    if (input_manager.isKeyPressed(InputManager::KEY_7))
    {
      if (!input_state_.num7_pressed)
      {
        elevator_trim_ -= 0.01f;
        elevator_trim_ = glm::clamp(elevator_trim_, -0.3f, 0.3f);
        std::cout << "Elevator trim: " << elevator_trim_ << " (nose "
                  << (elevator_trim_ < 0 ? "UP" : "DOWN") << ")" << std::endl;
        input_state_.num7_pressed = true;
      }
    }
    else
    {
      input_state_.num7_pressed = false;
    }

    // Tecla 8: Ajustar trim del elevador hacia abajo (nariz abajo)
    if (input_manager.isKeyPressed(InputManager::KEY_8))
    {
      if (!input_state_.num8_pressed)
      {
        elevator_trim_ += 0.01f; // Positivo = nariz abajo
        elevator_trim_ = glm::clamp(elevator_trim_, -0.3f, 0.3f);
        std::cout << "Elevator trim: " << elevator_trim_ << " (nose "
                  << (elevator_trim_ < 0 ? "UP" : "DOWN") << ")" << std::endl;
        input_state_.num8_pressed = true;
      }
    }
    else
    {
      input_state_.num8_pressed = false;
    }

    // Tecla 9: Resetear trim a valor por defecto
    if (input_manager.isKeyPressed(InputManager::KEY_9))
    {
      if (!input_state_.num9_pressed)
      {
        elevator_trim_ = -0.09024f;
        aileron_trim_ = 0.0f;
        rudder_trim_ = 0.0f;
        std::cout << "Trim reset to default (level flight)" << std::endl;
        input_state_.num9_pressed = true;
      }
    }
    else
    {
      input_state_.num9_pressed = false;
    }
  }

  void update()
  {
    if (flight_dynamics_)
    {
      flight_dynamics_->update(app_state_.delta_time);
      Physics::FlightData phys_fd = flight_dynamics_->getFlightData();

      glm::vec3 aircraft_position = flight_dynamics_->getPosition();
      glm::vec3 euler_angles = flight_dynamics_->getEulerAngles();

      if (aircraft_model_)
      {
        float flight_pitch = euler_angles.x;
        float flight_yaw = euler_angles.y;
        float flight_roll = euler_angles.z;

        aircraft_model_->getTransform().rotation = glm::vec3(
            glm::radians(flight_roll),
            glm::radians(-flight_yaw),
            glm::radians(flight_pitch));

        aircraft_model_->getTransform().position = glm::vec3(aircraft_position.x, aircraft_position.y, aircraft_position.z);
      }

      Camera *camera = camera_controller_->getActiveCamera();
      if (camera)
      {
        if (third_person_mode_)
        {
          if (cinematic_mode_)
          {
            // Calcular vectores del avión basados en los ángulos de Euler
            // Los ángulos vienen como: euler_angles = (pitch, yaw, roll) en grados
            float pitch_rad = glm::radians(euler_angles.x);
            float yaw_rad = glm::radians(euler_angles.y);
            float roll_rad = glm::radians(euler_angles.z);

            // Calcular vector forward (dirección hacia donde apunta el avión)
            glm::vec3 aircraft_forward;
            aircraft_forward.x = cos(pitch_rad) * cos(yaw_rad);
            aircraft_forward.y = sin(pitch_rad);
            aircraft_forward.z = cos(pitch_rad) * sin(yaw_rad);
            aircraft_forward = glm::normalize(aircraft_forward);

            // Calcular vector right (derecha del avión)
            glm::vec3 aircraft_right;
            aircraft_right.x = -sin(yaw_rad);
            aircraft_right.y = 0.0f;
            aircraft_right.z = cos(yaw_rad);
            aircraft_right = glm::normalize(aircraft_right);

            // Calcular vector up base y aplicar roll
            glm::vec3 base_up = glm::normalize(glm::cross(aircraft_right, aircraft_forward));
            glm::mat4 roll_matrix = glm::rotate(glm::mat4(1.0f), roll_rad, aircraft_forward);
            glm::vec3 aircraft_up = glm::vec3(roll_matrix * glm::vec4(base_up, 0.0f));
            aircraft_up = glm::normalize(aircraft_up);

            // Recalcular right después de aplicar roll
            aircraft_right = glm::normalize(glm::cross(aircraft_forward, aircraft_up));

            // Actualizar cámara cinemática (los parámetros ya están configurados)
            camera->setCinematicFollow(
                aircraft_position,
                aircraft_forward,
                aircraft_right,
                aircraft_up,
                app_state_.delta_time);
          }
          else
          {
            // Cámara de tercera persona clásica (detrás del avión)
            camera->setThirdPersonFollow(aircraft_position,
                                         euler_angles.y, euler_angles.x, euler_angles.z,
                                         third_person_distance_, third_person_height_);
          }
        }
        else
        {
          // Cámara en primera persona
          camera->setPosition(aircraft_position);
          camera->setRotation(euler_angles.y, euler_angles.x, euler_angles.z);
        }
      }

      // Actualizar HUD
      hud::FlightData hud_fd{};
      hud_fd.pitch = phys_fd.pitch;
      hud_fd.roll = phys_fd.roll;
      hud_fd.heading = phys_fd.heading;
      hud_fd.altitude = phys_fd.altitude;
      hud_fd.speed = phys_fd.speed;
      hud_fd.vertical_speed = phys_fd.vertical_speed;
      hud_fd.airspeed = phys_fd.speed;
      hud_fd.position = aircraft_position;
      hud_fd.waypoint.latitude = phys_fd.waypoint.latitude;
      hud_fd.waypoint.longitud = phys_fd.waypoint.longitude;
      hud_fd.waypoint.altitude = phys_fd.waypoint.altitude;

      // Actualizar sistema de waypoints y misión
      if (waypoint_system_ && mission_runtime_)
      {
        waypoint_system_->update(aircraft_position, hud_fd, *mission_runtime_);
        mission_runtime_->updateProgress(hud_fd, app_state_.delta_time);
        mission_runtime_->updateMetrics(hud_fd, app_state_.delta_time);
      }

      // Actualizar instrumentos de HUD
      if (bank_angle_indicator_)
      {
        bank_angle_indicator_->update(hud_fd);
      }
      if (pitch_ladder_)
      {
        pitch_ladder_->update(hud_fd);
      }
      if (altimeter_)
      {
        altimeter_->update(hud_fd);
      }
      if (speed_indicator_)
      {
        speed_indicator_->update(hud_fd);
      }
      if (vsi_)
      {
        vsi_->update(hud_fd);
      }
      if (waypoint_indicator_)
      {
        waypoint_indicator_->update(hud_fd);
      }
    }
  }

  /**
   * @brief Renderiza la escena desde la perspectiva de la luz (Shadow Pass)
   *
   * Primera pasada del shadow mapping:
   * - Renderiza a un FBO con solo depth attachment
   * - Usa shader simple que solo calcula profundidad
   * - La geometría se transforma con lightSpaceMatrix
   */
  void renderShadowPass()
  {
    if (!shadow_map_ || !depth_shader_ || !light_manager_)
      return;

    auto sunlight = light_manager_->getSunlight();
    if (!sunlight)
      return;

    // Calcular matriz de luz (seguir al avión para mejor cobertura)
    glm::vec3 target_pos = camera_controller_ ? camera_controller_->getActiveCamera()->getPosition() : glm::vec3(0.0f);
    glm::mat4 lightSpaceMatrix = sunlight->getLightSpaceMatrix(target_pos);

    // 1. Activar FBO del shadow map
    shadow_map_->bindForWriting();

    // 2. Configurar shader de profundidad
    depth_shader_->use();
    depth_shader_->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    // 3. Renderizar geometría (terreno y avión)

    // Terreno
    if (chunked_terrain_)
    {
      glm::mat4 terrain_model = glm::mat4(1.0f);
      depth_shader_->setMat4("model", terrain_model);
      chunked_terrain_->draw();
    }

    // Avión (solo en tercera persona)
    if (aircraft_model_ && third_person_mode_)
    {
      // El modelo ya aplica sus transformaciones internas
      aircraft_model_->render(depth_shader_);
    }

    // 4. Desactivar FBO
    shadow_map_->unbind();
  }

  /**
   * @brief Renderiza la escena
   */
  void render()
  {
    // ========== SHADOW PASS ==========
    // Primera pasada: renderizar depth map desde perspectiva de la luz
    renderShadowPass();

    // ========== RENDER PASS ==========
    // Segunda pasada: renderizado normal con sombras

    // Restaurar viewport al tamaño de la ventana
    int width, height;
    glfwGetFramebufferSize(context_->getWindow(), &width, &height);
    glViewport(0, 0, width, height);

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

    // Calcular lightSpaceMatrix para sombras
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    if (light_manager_ && light_manager_->getSunlight())
    {
      lightSpaceMatrix = light_manager_->getSunlight()->getLightSpaceMatrix(camera_pos);
    }

    if (skybox_)
    {
      glm::mat4 skybox_view = glm::mat4(glm::mat3(view_matrix));
      skybox_->render(skybox_view, projection_matrix, app_state_.fog_enabled);
    }
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

    shader->setBool("fogEnabled", app_state_.fog_enabled);
    shader->setFloat("fogDensity", 0.0001f);
    shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

    // Aplicar lighting y shadow mapping
    if (light_manager_)
    {
      light_manager_->applyToShader(shader);
    }

    // Configurar shadow map
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    if (shadow_map_)
    {
      shadow_map_->bindForReading(GL_TEXTURE0 + 5); // Usar texture unit 5
      shader->setInt("shadowMap", 5);
    }

    if (chunked_terrain_)
    {
      shader->use();
      shader->setMat4("view", view_matrix);
      shader->setMat4("projection", projection_matrix);
      shader->setVec3("viewPos", camera_pos);
      shader->setBool("fogEnabled", app_state_.fog_enabled);
      shader->setFloat("fogDensity", 0.00006f); // niebla más blanda en terreno
      shader->setVec3("fogColor", glm::vec3(0.85f, 0.90f, 0.95f));

      light_manager_->applyToShader(shader);

      // Shadow mapping para terreno
      shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
      if (shadow_map_)
      {
        shadow_map_->bindForReading(GL_TEXTURE0 + 5);
        shader->setInt("shadowMap", 5);
      }

      if (app_state_.use_texture)
      {
        Texture *terrain_texture = texture_manager.getTexture("terrain");
        if (!terrain_texture)
        {
          terrain_texture = texture_manager.getTexture("fallback");
        }

        if (terrain_texture)
        {
          terrain_texture->bind(0);
          shader->setInt("ourTexture", 0);
        }
      }

      shader->setBool("useUniformColor", false);

      glm::mat4 terrain_model = glm::mat4(1.0f);
      shader->setMat4("model", terrain_model);

      chunked_terrain_->update(camera_pos);
      chunked_terrain_->draw();
    }

    if (aircraft_model_ && third_person_mode_)
    {
      shader->use();
      aircraft_model_->render(shader);
      shader->unuse();
    }

    if (waypoint_system_ && mission_runtime_)
    {
      waypoint_system_->render(view_matrix, projection_matrix, *mission_runtime_);
    }

    if (!third_person_mode_)
    {
      if (bank_angle_indicator_)
      {
        hud::FlightData flight_data;
        flight_data.roll = camera->getRoll();
        flight_data.pitch = camera->getPitch();
        flight_data.heading = camera->getYaw();

        if (flight_dynamics_)
        {
          flight_data.altitude = flight_dynamics_->getAltitude();
          flight_data.speed = flight_dynamics_->getSpeed();
        }
        else
        {
          flight_data.altitude = 0.0f;
          flight_data.speed = 0.0f;
        }

        if (bank_angle_indicator_)
        {
          bank_angle_indicator_->update(flight_data);
          bank_angle_indicator_->render();
        }

        if (pitch_ladder_)
        {
          pitch_ladder_->update(flight_data);
          pitch_ladder_->render();
        }
      }
      if (altimeter_)
      {
        altimeter_->render();
      }
      if (speed_indicator_)
      {
        speed_indicator_->render();
      }
      if (vsi_)
      {
        vsi_->render();
      }
      if (waypoint_indicator_)
      {
        waypoint_indicator_->render();
      }
    }
  }

  /**
   * @brief Limpia y libera recursos
   */
  void shutdown()
  {
    std::cout << "\n=== Shutting down engine ===" << std::endl;

    auto &input_manager = InputManager::getInstance();
    input_manager.shutdown();

    auto &texture_manager = TextureManager::getInstance();
    texture_manager.clear();

    if (bank_angle_indicator_)
      bank_angle_indicator_->clean();
    if (pitch_ladder_)
      pitch_ladder_->clean();
    if (altimeter_)
      altimeter_->clean();
    if (speed_indicator_)
      speed_indicator_->clean();
    if (vsi_)
      vsi_->clean();
    if (waypoint_indicator_)
      waypoint_indicator_->clean();

    // Clean mission system
    if (waypoint_system_)
      waypoint_system_->cleanup();

    bank_angle_indicator_.reset();
    pitch_ladder_.reset();
    altimeter_.reset();
    speed_indicator_.reset();
    vsi_.reset();
    waypoint_indicator_.reset();

    // After buffers are deleted, clear shaders while context is valid
    auto &shader_manager = ShaderManager::getInstance();
    shader_manager.clear();

    // Limpiar objetos de escena
    camera_controller_.reset();
    skybox_.reset();
    chunked_terrain_.reset();

    // Limpiar contexto al final
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
    std::cout << "TRIM (Stability Control):" << std::endl;
    std::cout << "7             : Trim elevator UP (nose up tendency)" << std::endl;
    std::cout << "8             : Trim elevator DOWN (nose down tendency)" << std::endl;
    std::cout << "9             : Reset trim to default (level flight)" << std::endl;
    std::cout << "                Note: Controls return to trim when released" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "CAMERA & VIEW:" << std::endl;
    std::cout << "C             : Toggle third-person camera" << std::endl;
    std::cout << "V             : Toggle cinematic/classic third-person mode" << std::endl;
    std::cout << "3             : Increase cinematic forward distance" << std::endl;
    std::cout << "4             : Increase cinematic lateral offset" << std::endl;
    std::cout << "5             : Increase cinematic height offset" << std::endl;
    std::cout << "6             : Increase cinematic max relocate distance" << std::endl;
    std::cout << "E             : Toggle mouse capture" << std::endl;
    std::cout << "R             : Reset camera" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "GRAPHICS:" << std::endl;
    std::cout << "G             : Toggle wireframe" << std::endl;
    std::cout << "T             : Toggle texture" << std::endl;
    std::cout << "F             : Toggle fog" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "INFO:" << std::endl;
    std::cout << "1             : Show controls" << std::endl;
    std::cout << "ESC           : Exit" << std::endl;
    std::cout << "J             : Toggle joystick controls (Logitech Extreme 3D Pro)" << std::endl;
    std::cout << "======================================" << std::endl;
  }
};

#endif // GRAPHICS_ENGINE_H
