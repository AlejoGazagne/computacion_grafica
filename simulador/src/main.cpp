/**
 * @file main.cpp
 * @brief OpenGL Graphics Engine - Main Application
 * @author Modular Architecture
 * @version 2.0
 * 
 * Nueva aplicación principal usando arquitectura modular completa.
 * Separación limpia de responsabilidades entre sistemas.
 */

#include <iostream>
#include <memory>
#include <chrono>

// Core System
#include "core/opengl_context.h"

// Graphics Systems
#include "graphics/shaders/shader_manager.h"
#include "graphics/textures/texture_manager.h"
#include "graphics/rendering/buffer_objects.h"
#include "graphics/skybox/skybox.h"

// Scene System
#include "scene/mesh.h"
#include "scene/camera.h"
#include "scene/terrain.h"

// Input System
#include "input/input_manager.h"

// UI System
#include "ui/bank_angle.h"

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Graphics::Core;
using namespace Graphics::Shaders;
using namespace Graphics::Textures;
using namespace Graphics::Rendering;
using namespace UI;
using namespace Graphics::Skybox;
using namespace Scene;
using namespace Input;

/**
 * @class GraphicsEngine
 * @brief Clase principal del motor gráfico modular
 */
class GraphicsEngine {
private:
    // Core Systems
    std::unique_ptr<OpenGLContext> context_;
    
    // Scene Objects
    std::unique_ptr<Mesh> cube_mesh_;
    std::unique_ptr<CameraController> camera_controller_;
    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Terrain> terrain_;
    
    // UI Systems
    std::unique_ptr<BankAngleIndicator> bank_angle_indicator_;
    
    // Application State
    struct AppState {
        bool wireframe_mode = false;
        bool use_texture = true;
        bool running = true;
        float delta_time = 0.0f;
        float last_frame = 0.0f;
        int terrain_size = 0;  // 0=flight_sim, 1=default, 2=large, 3=infinite
    } app_state_;
    
    // Input State (para evitar repetición de teclas)
    struct InputState {
        bool g_pressed = false;
        bool t_pressed = false;
        bool r_pressed = false;
        bool e_pressed = false;
        bool f1_pressed = false;
        bool p_pressed = false;
    } input_state_;

public:
    GraphicsEngine() = default;
    ~GraphicsEngine() = default;

    /**
     * @brief Maneja el cambio de tamaño de ventana
     */
    void handleWindowResize(int width, int height) {
        if (camera_controller_) {
            Camera* camera = camera_controller_->getActiveCamera();
            if (camera) {
                // Actualizar aspect ratio de la cámara
                float new_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
                camera->setAspectRatio(new_aspect_ratio);
            }
        }
        
        // Actualizar HUD
        if (bank_angle_indicator_) {
            bank_angle_indicator_->updateScreenSize(width, height);
        }
    }
    
    /**
     * @brief Cambia el tamaño del terrain
     */
    void switchTerrainSize() {
        app_state_.terrain_size = (app_state_.terrain_size + 1) % 4;
        
        // Recrear terrain con nuevo tamaño
        switch (app_state_.terrain_size) {
            case 0:
                // terrain_ = TerrainFactory::createFlightSimulator("main_terrain");
                std::cout << "Terrain size: Flight Simulator (5000x5000)" << std::endl;
                break;
            case 1:
                terrain_ = TerrainFactory::createFlat("main_terrain");
                std::cout << "Terrain size: Default (2000x2000)" << std::endl;
                break;
            case 2:
                terrain_ = TerrainFactory::createLarge("main_terrain");
                std::cout << "Terrain size: Large (500x500)" << std::endl;
                break;
            case 3:
                terrain_ = TerrainFactory::createInfinite("main_terrain");
                std::cout << "Terrain size: Infinite (1000x1000)" << std::endl;
                break;
        }
        
        if (!terrain_) {
            std::cerr << "Failed to recreate terrain!" << std::endl;
        }
    }

    /**
     * @brief Inicializa todos los sistemas del motor
     */
    bool initialize() {
        std::cout << "=== Initializing OpenGL Graphics Engine ===" << std::endl;
        
        // 1. Inicializar contexto OpenGL
        if (!initializeOpenGL()) {
            return false;
        }
        
        // 2. Inicializar sistemas gráficos
        if (!initializeGraphicsSystems()) {
            return false;
        }
        
        // 3. Inicializar sistema de entrada
        if (!initializeInputSystem()) {
            return false;
        }
        
        // 4. Inicializar escena
        if (!initializeScene()) {
            return false;
        }
        
        // 5. Inicializar HUD
        if (!initializeUI()) {
            return false;
        }
        
        std::cout << "=== Engine initialized successfully! ===" << std::endl;
        return true;
    }
    
    /**
     * @brief Ejecuta el loop principal de la aplicación
     */
    void run() {
        std::cout << "\n=== Starting main loop ===" << std::endl;
        printControls();
        
        while (!context_->shouldClose() && app_state_.running) {
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
    bool initializeOpenGL() {
        WindowConfig config;
        config.width = 1024;
        config.height = 768;
        config.title = "OpenGL Graphics Engine - Modular Architecture";
        config.vsync = true;
        
        context_ = std::make_unique<OpenGLContext>();
        
        if (!context_->initialize(config)) {
            std::cerr << "Failed to initialize OpenGL context" << std::endl;
            return false;
        }
        
        // Configuraciones de OpenGL
        context_->enableDepthTest(true);
        context_->enableFaceCulling(false); // Desactivado para ver el cubo completo
        
        // Configurar callback de resize para mantener aspect ratio
        context_->setResizeCallback([this](int width, int height) {
            handleWindowResize(width, height);
        });
        
        return true;
    }
    
    /**
     * @brief Inicializa los sistemas gráficos (shaders, texturas, etc.)
     */
    bool initializeGraphicsSystems() {
        auto& shader_manager = ShaderManager::getInstance();
        auto& texture_manager = TextureManager::getInstance();
        
        // Cargar shaders
        if (!shader_manager.loadShader("basic_3d", "shaders/vertex_3d.glsl", "shaders/fragment_3d.glsl")) {
            std::cerr << "Failed to load basic 3D shader" << std::endl;
            return false;
        }
        
        // Cargar texturas
        if (!texture_manager.loadTexture2D("container", "textures/container.jpg", true)) {
            std::cout << "Warning: Could not load container texture, using procedural texture" << std::endl;
        }
        
        // Cargar textura del terrain
        if (!texture_manager.loadTexture2D("terrain", "textures/terrain/terrain.jpg", true)) {
            std::cout << "Warning: Could not load terrain texture, using fallback" << std::endl;
        }
        
        // Crear textura procedural como fallback
        if (!texture_manager.createProceduralTexture("fallback", 64, 64, 255, 128, 0, 255)) {
            std::cerr << "Failed to create procedural texture" << std::endl;
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Inicializa el sistema de entrada
     */
    bool initializeInputSystem() {
        auto& input_manager = InputManager::getInstance();
        
        if (!input_manager.initialize(context_->getWindow())) {
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
    bool initializeScene() {
        // Crear mesh del cubo
        cube_mesh_ = MeshFactory::createCube(1.0f, "main_cube");
        if (!cube_mesh_) {
            std::cerr << "Failed to create cube mesh" << std::endl;
            return false;
        }
        
        // Configurar sistema de cámara
        camera_controller_ = std::make_unique<CameraController>();
        camera_controller_->setWindow(context_->getWindow());
        
        // Crear cámara principal
        auto camera_config = CameraController::getFirstPersonConfig();
        // Usar posición predeterminada que ya está configurada para ver el cubo
        camera_config.aspect_ratio = static_cast<float>(context_->getConfig().width) / 
                                   static_cast<float>(context_->getConfig().height);
        
        auto camera = std::make_unique<Camera>(camera_config);
        camera_controller_->addCamera(std::move(camera));
        camera_controller_->setActiveCamera(0);
        
        // Capturar mouse inicialmente
        camera_controller_->setMouseCaptured(true);
        
        // Inicializar skybox
        skybox_ = std::make_unique<Skybox>();
        if (!skybox_->initialize()) {
            std::cerr << "Failed to initialize skybox" << std::endl;
            return false;
        }
        
        // Crear terrain/piso
        terrain_ = TerrainFactory::createFlat("main_terrain");
        if (!terrain_) {
            std::cerr << "Failed to create terrain" << std::endl;
            return false;
        }
        
        return true;
    }
    
    /**
     * @brief Inicializar sistema de UI/HUD
     */
    bool initializeUI() {
        // Obtener dimensiones de ventana
        int width, height;
        glfwGetWindowSize(context_->getWindow(), &width, &height);
        
        // Crear indicador de bank angle
        bank_angle_indicator_ = std::make_unique<BankAngleIndicator>(width, height);
        
        if (!bank_angle_indicator_->isInitialized()) {
            std::cerr << "Failed to initialize Bank Angle HUD" << std::endl;
            return false;
        }
        
        std::cout << "Bank Angle HUD initialized successfully" << std::endl;
        return true;
    }
    
    /**
     * @brief Configura los callbacks de entrada
     */
    void setupInputCallbacks() {
        auto& input_manager = InputManager::getInstance();
        
        // Configurar callback de mouse para la cámara
        input_manager.addMouseCallback([this](double xpos, double ypos, double delta_x, double delta_y) {
            if (camera_controller_->isMouseCaptured()) {
                camera_controller_->mouseCallback(context_->getWindow(), xpos, ypos);
            }
        });

    }
    
    /**
     * @brief Actualiza el timing de la aplicación
     */
    void updateTiming() {
        auto current_time = std::chrono::high_resolution_clock::now();
        static auto last_time = current_time;
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - last_time);
        app_state_.delta_time = duration.count() / 1000000.0f; // Convertir a segundos
        
        last_time = current_time;
    }
    
    /**
     * @brief Procesa la entrada del usuario
     */
    void processInput() {
        auto& input_manager = InputManager::getInstance();
        input_manager.update(app_state_.delta_time);
        
        // Procesar entrada de la cámara
        if (camera_controller_->isMouseCaptured()) {
            camera_controller_->processInput(app_state_.delta_time);
        }
        
        // Procesar teclas especiales
        processSpecialKeys();
    }
    
    /**
     * @brief Procesa teclas especiales (toggles, etc.)
     */
    void processSpecialKeys() {
        auto& input_manager = InputManager::getInstance();
        
        // ESC - Salir
        if (input_manager.isKeyPressed(InputManager::KEY_ESCAPE)) {
            app_state_.running = false;
        }
        
        // G - Toggle Wireframe
        if (input_manager.isKeyPressed(InputManager::KEY_G)) {
            if (!input_state_.g_pressed) {
                app_state_.wireframe_mode = !app_state_.wireframe_mode;
                context_->setWireframeMode(app_state_.wireframe_mode);
                std::cout << "Wireframe mode: " << (app_state_.wireframe_mode ? "ON" : "OFF") << std::endl;
                input_state_.g_pressed = true;
            }
        } else {
            input_state_.g_pressed = false;
        }
        
        // T - Toggle Texture
        if (input_manager.isKeyPressed(InputManager::KEY_T)) {
            if (!input_state_.t_pressed) {
                app_state_.use_texture = !app_state_.use_texture;
                std::cout << "Texture mode: " << (app_state_.use_texture ? "ON" : "OFF") << std::endl;
                input_state_.t_pressed = true;
            }
        } else {
            input_state_.t_pressed = false;
        }
        
        // R - Reset Camera
        if (input_manager.isKeyPressed(InputManager::KEY_R)) {
            if (!input_state_.r_pressed) {
                Camera* active_camera = camera_controller_->getActiveCamera();
                if (active_camera) {
                    active_camera->reset();
                    std::cout << "Camera reset" << std::endl;
                }
                input_state_.r_pressed = true;
            }
        } else {
            input_state_.r_pressed = false;
        }
        
        // E - Toggle Mouse Capture
        if (input_manager.isKeyPressed(InputManager::KEY_E)) {
            if (!input_state_.e_pressed) {
                bool captured = !camera_controller_->isMouseCaptured();
                camera_controller_->setMouseCaptured(captured);
                std::cout << "Mouse " << (captured ? "captured" : "released") << std::endl;
                input_state_.e_pressed = true;
            }
        } else {
            input_state_.e_pressed = false;
        }
        
        // P - Switch Terrain Size
        if (input_manager.isKeyPressed(InputManager::KEY_P)) {
            if (!input_state_.p_pressed) {
                switchTerrainSize();
                input_state_.p_pressed = true;
            }
        } else {
            input_state_.p_pressed = false;
        }
        
        // F1 - Show/Hide Controls
        if (input_manager.isKeyPressed(InputManager::KEY_1)) {
            if (!input_state_.f1_pressed) {
                printControls();
                input_state_.f1_pressed = true;
            }
        } else {
            input_state_.f1_pressed = false;
        }
    }
    
    /**
     * @brief Actualiza la lógica de la aplicación
     */
    void update() {
        // Aquí se pueden agregar actualizaciones de lógica de juego
        // Por ejemplo: física, animaciones, IA, etc.
    }
    
    /**
     * @brief Renderiza la escena
     */
    void render() {
        // Limpiar buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Obtener cámara para las matrices
        Camera* camera = camera_controller_->getActiveCamera();
        if (!camera) {
            std::cerr << "Missing camera!" << std::endl;
            return;
        }
        
        // Renderizar skybox primero (debe estar en el fondo)
        if (skybox_) {
            skybox_->render(camera->getViewMatrix(), camera->getProjectionMatrix());
        }
        
        // Obtener shader y texture manager para objetos normales
        auto& shader_manager = ShaderManager::getInstance();
        auto& texture_manager = TextureManager::getInstance();
        
        Shader* shader = shader_manager.getShader("basic_3d");
        
        if (!shader || !camera) {
            std::cerr << "Missing shader or camera!" << std::endl;
            return;
        }
        
        // Usar shader
        shader->use();
        
        // Configurar matrices comunes
        shader->setMat4("view", camera->getViewMatrix());
        shader->setMat4("projection", camera->getProjectionMatrix());
        shader->setBool("useTexture", app_state_.use_texture);
        
        // Renderizar terrain
        if (terrain_) {
            // Configurar textura del terrain
            if (app_state_.use_texture) {
                Texture* terrain_texture = texture_manager.getTexture("terrain");
                if (!terrain_texture) {
                    terrain_texture = texture_manager.getTexture("fallback");
                }
                
                if (terrain_texture) {
                    terrain_texture->bind(0);
                    shader->setInt("ourTexture", 0);
                }
            }
            
            // Configurar matriz modelo para el terrain (sin transformaciones)
            glm::mat4 terrain_model = glm::mat4(1.0f);
            shader->setMat4("model", terrain_model);
            
            terrain_->draw();
        }
        
        // Renderizar cubo
        if (cube_mesh_) {
            // Configurar textura del cubo
            if (app_state_.use_texture) {
                Texture* cube_texture = texture_manager.getTexture("container");
                if (!cube_texture) {
                    cube_texture = texture_manager.getTexture("fallback");
                }
                
                if (cube_texture) {
                    cube_texture->bind(0);
                    shader->setInt("ourTexture", 0);
                }
            }
            
            // Configurar matriz modelo para el cubo
            glm::mat4 cube_model = glm::mat4(1.0f);
            shader->setMat4("model", cube_model);
            
            cube_mesh_->draw();
        }
        
        shader->unuse();
        
        // Renderizar indicador de bank angle (encima de todo)
        if (bank_angle_indicator_ && bank_angle_indicator_->isInitialized()) {
            float roll_angle = camera->getRoll();
            
            // Imprimir el ángulo para debug (temporal)
            static int frame_count = 0;
            if (++frame_count % 60 == 0) {  // Cada 60 frames
                std::cout << "Bank Angle: " << roll_angle << "°" << std::endl;
            }
            
            bank_angle_indicator_->render(roll_angle);
        }
    }
    
    /**
     * @brief Limpia y libera recursos
     */
    void shutdown() {
        std::cout << "\n=== Shutting down engine ===" << std::endl;
        
        // Limpiar sistemas
        auto& input_manager = InputManager::getInstance();
        input_manager.shutdown();
        
        auto& shader_manager = ShaderManager::getInstance();
        shader_manager.clear();
        
        auto& texture_manager = TextureManager::getInstance();
        texture_manager.clear();
        
        // Limpiar objetos de escena
        cube_mesh_.reset();
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
    void printControls() const {
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
        std::cout << "R        : Reset camera" << std::endl;
        std::cout << "P        : Change terrain size" << std::endl;
        std::cout << "E        : Toggle mouse capture" << std::endl;
        std::cout << "1        : Show controls" << std::endl;
        std::cout << "ESC      : Exit" << std::endl;
        std::cout << "==============================" << std::endl;
    }
};

/**
 * @brief Punto de entrada principal
 */
int main() {
    try {
        GraphicsEngine engine;
        
        if (!engine.initialize()) {
            std::cerr << "Failed to initialize graphics engine" << std::endl;
            return -1;
        }
        
        engine.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
        return -1;
    }
    
    return 0;
}