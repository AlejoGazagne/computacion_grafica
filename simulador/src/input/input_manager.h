#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

extern "C" {
    #include <GLFW/glfw3.h>
}

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <string>

namespace Input {

    enum class KeyState {
        RELEASED,
        PRESSED,
        HELD,
        JUST_PRESSED,
        JUST_RELEASED
    };

    enum class MouseButton {
        LEFT = GLFW_MOUSE_BUTTON_LEFT,
        RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
        MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
    };

    struct MouseState {
        double x = 0.0;
        double y = 0.0;
        double last_x = 0.0;
        double last_y = 0.0;
        double delta_x = 0.0;
        double delta_y = 0.0;
        double scroll_x = 0.0;
        double scroll_y = 0.0;
        bool first_movement = true;
    };

    // Callbacks tipados
    using KeyCallback = std::function<void(int key, KeyState state, float delta_time)>;
    using MouseCallback = std::function<void(double xpos, double ypos, double delta_x, double delta_y)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
    using MouseButtonCallback = std::function<void(MouseButton button, KeyState state)>;

    class InputManager {
    private:
        GLFWwindow* window_;
        
        // Estado de teclas
        std::unordered_map<int, KeyState> key_states_;
        std::unordered_map<int, KeyState> previous_key_states_;
        
        // Estado de botones del mouse
        std::unordered_map<MouseButton, KeyState> mouse_button_states_;
        std::unordered_map<MouseButton, KeyState> previous_mouse_button_states_;
        
        // Estado del mouse
        MouseState mouse_state_;
        
        // Callbacks
        std::vector<KeyCallback> key_callbacks_;
        std::vector<MouseCallback> mouse_callbacks_;
        std::vector<ScrollCallback> scroll_callbacks_;
        std::vector<MouseButtonCallback> mouse_button_callbacks_;
        
        // Configuración
        bool keys_enabled_;
        bool mouse_enabled_;
        bool mouse_captured_;
        
        // Singleton
        static std::unique_ptr<InputManager> instance_;
        
        // Callbacks estáticos para GLFW
        static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos);
        static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
        static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);

    public:
        InputManager();
        ~InputManager() = default;
        
        // Singleton
        static InputManager& getInstance();
        
        // No permitir copia
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        // Inicialización
        bool initialize(GLFWwindow* window);
        void shutdown();
        
        // Update (llamar cada frame)
        void update(float delta_time);
        
        // Estado de teclas
        bool isKeyPressed(int key) const;
        bool isKeyHeld(int key) const;
        bool isKeyJustPressed(int key) const;
        bool isKeyJustReleased(int key) const;
        KeyState getKeyState(int key) const;
        
        // Estado de botones del mouse
        bool isMouseButtonPressed(MouseButton button) const;
        bool isMouseButtonHeld(MouseButton button) const;
        bool isMouseButtonJustPressed(MouseButton button) const;
        bool isMouseButtonJustReleased(MouseButton button) const;
        KeyState getMouseButtonState(MouseButton button) const;
        
        // Estado del mouse
        const MouseState& getMouseState() const { return mouse_state_; }
        double getMouseX() const { return mouse_state_.x; }
        double getMouseY() const { return mouse_state_.y; }
        double getMouseDeltaX() const { return mouse_state_.delta_x; }
        double getMouseDeltaY() const { return mouse_state_.delta_y; }
        double getScrollX() const { return mouse_state_.scroll_x; }
        double getScrollY() const { return mouse_state_.scroll_y; }
        
        // Configuración del mouse
        void setMouseCaptured(bool captured);
        bool isMouseCaptured() const { return mouse_captured_; }
        
        void setMouseEnabled(bool enabled) { mouse_enabled_ = enabled; }
        bool isMouseEnabled() const { return mouse_enabled_; }
        
        void setKeysEnabled(bool enabled) { keys_enabled_ = enabled; }
        bool areKeysEnabled() const { return keys_enabled_; }
        
        // Callbacks
        void addKeyCallback(const KeyCallback& callback);
        void addMouseCallback(const MouseCallback& callback);
        void addScrollCallback(const ScrollCallback& callback);
        void addMouseButtonCallback(const MouseButtonCallback& callback);
        
        void clearKeyCallbacks() { key_callbacks_.clear(); }
        void clearMouseCallbacks() { mouse_callbacks_.clear(); }
        void clearScrollCallbacks() { scroll_callbacks_.clear(); }
        void clearMouseButtonCallbacks() { mouse_button_callbacks_.clear(); }
        void clearAllCallbacks();
        
        // Utilidades
        void resetMouseDelta();
        void resetScroll();
        
        // Conversiones de teclas comunes
        static constexpr int KEY_ESCAPE = GLFW_KEY_ESCAPE;
        static constexpr int KEY_ENTER = GLFW_KEY_ENTER;
        static constexpr int KEY_SPACE = GLFW_KEY_SPACE;
        static constexpr int KEY_W = GLFW_KEY_W;
        static constexpr int KEY_A = GLFW_KEY_A;
        static constexpr int KEY_S = GLFW_KEY_S;
        static constexpr int KEY_D = GLFW_KEY_D;
        static constexpr int KEY_Q = GLFW_KEY_Q;
        static constexpr int KEY_E = GLFW_KEY_E;
        static constexpr int KEY_R = GLFW_KEY_R;
        static constexpr int KEY_T = GLFW_KEY_T;
        static constexpr int KEY_G = GLFW_KEY_G;
        static constexpr int KEY_P = GLFW_KEY_P;
        static constexpr int KEY_F = GLFW_KEY_F;
        static constexpr int KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
        static constexpr int KEY_LEFT_CTRL = GLFW_KEY_LEFT_CONTROL;
        static constexpr int KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT;
        static constexpr int KEY_TAB = GLFW_KEY_TAB;
        
        // Teclas de flechas
        static constexpr int KEY_UP = GLFW_KEY_UP;
        static constexpr int KEY_DOWN = GLFW_KEY_DOWN;
        static constexpr int KEY_LEFT = GLFW_KEY_LEFT;
        static constexpr int KEY_RIGHT = GLFW_KEY_RIGHT;
        
        // Teclas numéricas
        static constexpr int KEY_0 = GLFW_KEY_0;
        static constexpr int KEY_1 = GLFW_KEY_1;
        static constexpr int KEY_2 = GLFW_KEY_2;
        static constexpr int KEY_3 = GLFW_KEY_3;
        static constexpr int KEY_4 = GLFW_KEY_4;
        static constexpr int KEY_5 = GLFW_KEY_5;
        static constexpr int KEY_6 = GLFW_KEY_6;
        static constexpr int KEY_7 = GLFW_KEY_7;
        static constexpr int KEY_8 = GLFW_KEY_8;
        static constexpr int KEY_9 = GLFW_KEY_9;
    };

    // Clase auxiliar para manejar acciones específicas
    class ActionManager {
    private:
        struct Action {
            std::string name;
            std::vector<int> keys;
            std::function<void()> callback;
            bool triggered_this_frame = false;
        };
        
        std::unordered_map<std::string, Action> actions_;
        InputManager* input_manager_;

    public:
        ActionManager();
        ~ActionManager() = default;

        // Configurar acciones
        void bindAction(const std::string& name, const std::vector<int>& keys, const std::function<void()>& callback);
        void bindAction(const std::string& name, int key, const std::function<void()>& callback);
        
        void unbindAction(const std::string& name);
        void clearActions();
        
        // Verificar acciones
        bool isActionTriggered(const std::string& name) const;
        bool isActionActive(const std::string& name) const;
        
        // Update
        void update();
        
        // Acciones predefinidas comunes
        void bindMovementActions(const std::function<void(float, float)>& movement_callback);
        void bindCameraActions(const std::function<void()>& reset_callback,
                             const std::function<void()>& toggle_mouse_callback);
    };

} // namespace Input

#endif // INPUT_MANAGER_H