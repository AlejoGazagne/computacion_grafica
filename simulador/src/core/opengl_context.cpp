#include "opengl_context.h"
#include <iostream>

namespace Graphics {
    namespace Core {

        OpenGLContext::OpenGLContext() 
            : window_(nullptr), initialized_(false), resize_callback_(nullptr) {
        }

        OpenGLContext::~OpenGLContext() {
            shutdown();
        }

        void OpenGLContext::errorCallback(int error, const char* description) {
            std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
        }

        void OpenGLContext::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
            
            // Llamar al callback personalizado si está configurado
            OpenGLContext* context = static_cast<OpenGLContext*>(glfwGetWindowUserPointer(window));
            if (context && context->resize_callback_) {
                context->resize_callback_(width, height);
            }
        }

        bool OpenGLContext::initialize(const WindowConfig& config) {
            if (initialized_) {
                std::cout << "OpenGL Context already initialized!" << std::endl;
                return false;
            }

            config_ = config;

            // Configurar callback de errores
            glfwSetErrorCallback(errorCallback);

            // Inicializar GLFW
            if (!glfwInit()) {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                return false;
            }

            // Configurar versión de OpenGL
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            #ifdef __APPLE__
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            #endif

            // Crear ventana
            GLFWmonitor* monitor = config_.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
            window_ = glfwCreateWindow(config_.width, config_.height, 
                                     config_.title.c_str(), monitor, nullptr);

            if (!window_) {
                std::cerr << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return false;
            }

            // Hacer el contexto actual
            glfwMakeContextCurrent(window_);

            // Configurar VSync
            glfwSwapInterval(config_.vsync ? 1 : 0);

            // Cargar funciones de OpenGL con GLAD
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                std::cerr << "Failed to initialize GLAD" << std::endl;
                glfwDestroyWindow(window_);
                glfwTerminate();
                return false;
            }

            // Configurar viewport
            glViewport(0, 0, config_.width, config_.height);

            // Configurar window user pointer para callbacks
            glfwSetWindowUserPointer(window_, this);
            
            // Configurar callbacks
            glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

            // Configuraciones por defecto de OpenGL
            enableDepthTest(true);
            glClearColor(0.2f, 0.3f, 0.5f, 1.0f);  // Color de fondo azul cielo suave

            initialized_ = true;
            printContextInfo();

            return true;
        }

        void OpenGLContext::shutdown() {
            if (window_) {
                glfwDestroyWindow(window_);
                window_ = nullptr;
            }
            
            if (initialized_) {
                glfwTerminate();
                initialized_ = false;
            }
        }

        bool OpenGLContext::shouldClose() const {
            return window_ ? glfwWindowShouldClose(window_) : true;
        }

        void OpenGLContext::swapBuffers() {
            if (window_) {
                glfwSwapBuffers(window_);
            }
        }

        void OpenGLContext::pollEvents() {
            glfwPollEvents();
        }

        void OpenGLContext::setWindowConfig(const WindowConfig& config) {
            config_ = config;
            if (window_) {
                glfwSetWindowTitle(window_, config_.title.c_str());
                if (!config_.fullscreen) {
                    glfwSetWindowSize(window_, config_.width, config_.height);
                }
                glfwSwapInterval(config_.vsync ? 1 : 0);
            }
        }

        void OpenGLContext::enableDepthTest(bool enable) {
            if (enable) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
        }

        void OpenGLContext::enableFaceCulling(bool enable) {
            if (enable) {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
            } else {
                glDisable(GL_CULL_FACE);
            }
        }

        void OpenGLContext::setWireframeMode(bool enable) {
            if (enable) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        void OpenGLContext::printContextInfo() const {
            const GLubyte* renderer = glGetString(GL_RENDERER);
            const GLubyte* version = glGetString(GL_VERSION);
            const GLubyte* vendor = glGetString(GL_VENDOR);
            const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
            
            std::cout << "=== OpenGL Context Info ===" << std::endl;
            std::cout << "Vendor: " << vendor << std::endl;
            std::cout << "Renderer: " << renderer << std::endl;
            std::cout << "OpenGL Version: " << version << std::endl;
            std::cout << "GLSL Version: " << glsl_version << std::endl;
            std::cout << "============================" << std::endl;
        }
        
        void OpenGLContext::setResizeCallback(const ResizeCallback& callback) {
            resize_callback_ = callback;
        }

    } // namespace Core
} // namespace Graphics