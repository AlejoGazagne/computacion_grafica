#ifndef OPENGL_CONTEXT_H
#define OPENGL_CONTEXT_H

extern "C" {
    #include <glad/glad.h>
    #include <GLFW/glfw3.h>
}

#include <string>
#include <memory>
#include <functional>

namespace Graphics {
    namespace Core {
        
        struct WindowConfig {
            int width = 800;
            int height = 600;
            std::string title = "OpenGL Application";
            bool fullscreen = false;
            bool vsync = true;
        };

        // Tipo de callback para cambio de tamaño de ventana
        using ResizeCallback = std::function<void(int, int)>;

        class OpenGLContext {
        private:
            GLFWwindow* window_;
            WindowConfig config_;
            bool initialized_;
            ResizeCallback resize_callback_;

            // Callbacks estáticas
            static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
            static void errorCallback(int error, const char* description);

        public:
            OpenGLContext();
            ~OpenGLContext();

            // No permitir copia
            OpenGLContext(const OpenGLContext&) = delete;
            OpenGLContext& operator=(const OpenGLContext&) = delete;

            bool initialize(const WindowConfig& config);
            void shutdown();
            
            bool shouldClose() const;
            void swapBuffers();
            void pollEvents();
            
            GLFWwindow* getWindow() const { return window_; }
            const WindowConfig& getConfig() const { return config_; }
            
            void setWindowConfig(const WindowConfig& config);
            void enableDepthTest(bool enable = true);
            void enableFaceCulling(bool enable = true);
            void setWireframeMode(bool enable);
            
            // Configurar callback de resize personalizado
            void setResizeCallback(const ResizeCallback& callback);
            
            // Información del contexto
            void printContextInfo() const;
        };
        
    } // namespace Core
} // namespace Graphics

#endif // OPENGL_CONTEXT_H