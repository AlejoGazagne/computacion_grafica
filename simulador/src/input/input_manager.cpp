#include "input_manager.h"
#include <iostream>
#include <algorithm>

namespace Input {

    // Inicialización del singleton
    std::unique_ptr<InputManager> InputManager::instance_ = nullptr;

    // === Implementación de InputManager ===

    InputManager::InputManager() 
                : window_(nullptr), keys_enabled_(true), mouse_enabled_(true), mouse_captured_(false),
                    use_joystick_(false), joystick_id_(GLFW_JOYSTICK_1), joystick_present_(false),
                    joystick_axes_count_(0), joystick_buttons_count_(0), joystick_deadzone_(0.1f) {
    }

    InputManager& InputManager::getInstance() {
        if (!instance_) {
            instance_ = std::make_unique<InputManager>();
        }
        return *instance_;
    }

    bool InputManager::initialize(GLFWwindow* window) {
        if (!window) {
            std::cerr << "ERROR: Cannot initialize InputManager with null window" << std::endl;
            return false;
        }

        window_ = window;
        
        // Configurar callbacks de GLFW
        glfwSetKeyCallback(window_, keyCallbackStatic);
        glfwSetCursorPosCallback(window_, mouseCallbackStatic);
        glfwSetScrollCallback(window_, scrollCallbackStatic);
        glfwSetMouseButtonCallback(window_, mouseButtonCallbackStatic);
        
        // Obtener posición inicial del mouse
        glfwGetCursorPos(window_, &mouse_state_.x, &mouse_state_.y);
        mouse_state_.last_x = mouse_state_.x;
        mouse_state_.last_y = mouse_state_.y;
        
        std::cout << "InputManager initialized successfully" << std::endl;
        // Comprobar joystick inicialmente
        joystick_present_ = glfwJoystickPresent(joystick_id_) == GLFW_TRUE;
        if (joystick_present_) {
            std::cout << "Joystick detected: id=" << joystick_id_ << std::endl;
        }
        return true;
    }

    void InputManager::shutdown() {
        if (window_) {
            // Limpiar callbacks
            glfwSetKeyCallback(window_, nullptr);
            glfwSetCursorPosCallback(window_, nullptr);
            glfwSetScrollCallback(window_, nullptr);
            glfwSetMouseButtonCallback(window_, nullptr);
        }
        
        clearAllCallbacks();
        key_states_.clear();
        previous_key_states_.clear();
        mouse_button_states_.clear();
        previous_mouse_button_states_.clear();
        
        window_ = nullptr;
    }

    void InputManager::update(float delta_time) {
        // Guardar estados previos
        previous_key_states_ = key_states_;
        previous_mouse_button_states_ = mouse_button_states_;
        
        // Actualizar estados de teclas basándose en GLFW
        for (auto& [key, state] : key_states_) {
            int glfw_state = glfwGetKey(window_, key);
            
            if (glfw_state == GLFW_PRESS) {
                if (state == KeyState::RELEASED || state == KeyState::JUST_RELEASED) {
                    state = KeyState::JUST_PRESSED;
                } else if (state == KeyState::JUST_PRESSED) {
                    state = KeyState::HELD;
                }
            } else {
                if (state == KeyState::PRESSED || state == KeyState::HELD || state == KeyState::JUST_PRESSED) {
                    state = KeyState::JUST_RELEASED;
                } else if (state == KeyState::JUST_RELEASED) {
                    state = KeyState::RELEASED;
                }
            }
        }
        
        // Actualizar estados de botones del mouse
        for (auto& [button, state] : mouse_button_states_) {
            int glfw_state = glfwGetMouseButton(window_, static_cast<int>(button));
            
            if (glfw_state == GLFW_PRESS) {
                if (state == KeyState::RELEASED || state == KeyState::JUST_RELEASED) {
                    state = KeyState::JUST_PRESSED;
                } else if (state == KeyState::JUST_PRESSED) {
                    state = KeyState::HELD;
                }
            } else {
                if (state == KeyState::PRESSED || state == KeyState::HELD || state == KeyState::JUST_PRESSED) {
                    state = KeyState::JUST_RELEASED;
                } else if (state == KeyState::JUST_RELEASED) {
                    state = KeyState::RELEASED;
                }
            }
        }
        
        // Procesar callbacks de teclas si están habilitadas
        if (keys_enabled_) {
            for (const auto& [key, state] : key_states_) {
                for (const auto& callback : key_callbacks_) {
                    callback(key, state, delta_time);
                }
            }
        }
        
        // Procesar callbacks de mouse si está habilitado
        if (mouse_enabled_) {
            for (const auto& callback : mouse_callbacks_) {
                callback(mouse_state_.x, mouse_state_.y, mouse_state_.delta_x, mouse_state_.delta_y);
            }
            
            for (const auto& [button, state] : mouse_button_states_) {
                for (const auto& callback : mouse_button_callbacks_) {
                    callback(button, state);
                }
            }
        }
        
        // Reset mouse delta y scroll después de procesar
        resetMouseDelta();
        resetScroll();

        // --- Poll joystick si está habilitado ---
        if (use_joystick_) {
            joystick_present_ = (glfwJoystickPresent(joystick_id_) == GLFW_TRUE);
            if (joystick_present_) {
                int axes_count = 0;
                const float* axes = glfwGetJoystickAxes(joystick_id_, &axes_count);
                if (axes && axes_count > 0) {
                    joystick_axes_.assign(axes, axes + axes_count);
                    joystick_axes_count_ = axes_count;
                } else {
                    joystick_axes_.clear();
                    joystick_axes_count_ = 0;
                }

                int buttons_count = 0;
                const unsigned char* buttons = glfwGetJoystickButtons(joystick_id_, &buttons_count);
                if (buttons && buttons_count > 0) {
                    joystick_buttons_.assign(buttons, buttons + buttons_count);
                    joystick_buttons_count_ = buttons_count;
                } else {
                    joystick_buttons_.clear();
                    joystick_buttons_count_ = 0;
                }
            }
        }
    }

    bool InputManager::isKeyPressed(int key) const {
        // Si se está usando joystick, emular ciertas teclas según ejes
        if (use_joystick_) {
            // Mapeo aproximado para Logitech Extreme 3D Pro:
            // axis 0 -> X (roll)  : LEFT/RIGHT
            // axis 1 -> Y (pitch) : UP/DOWN (invertido)
            // axis 2 -> twist (rudder) : A/D
            // axis 3 -> throttle : W/S
            auto checkAxis = [this](int axis_index) -> float {
                if (axis_index < 0 || axis_index >= joystick_axes_count_) return 0.0f;
                return joystick_axes_[axis_index];
            };

            const float dead = joystick_deadzone_;

            if (key == KEY_LEFT) {
                float v = checkAxis(0);
                return v < -dead;
            }
            if (key == KEY_RIGHT) {
                float v = checkAxis(0);
                return v > dead;
            }
            if (key == KEY_UP) {
                float v = checkAxis(1);
                return v < -dead; // joystick up usually negative
            }
            if (key == KEY_DOWN) {
                float v = checkAxis(1);
                return v > dead;
            }
            if (key == KEY_A) {
                float v = checkAxis(2);
                return v < -dead; // twist left
            }
            if (key == KEY_D) {
                float v = checkAxis(2);
                return v > dead; // twist right
            }
            if (key == KEY_W) {
                float v = checkAxis(3);
                return v > dead; // throttle axis mapping (assumed)
            }
            if (key == KEY_S) {
                float v = checkAxis(3);
                return v < -dead;
            }
            // Para otras teclas, fallback a teclado
        }

        auto it = key_states_.find(key);
        return (it != key_states_.end()) && 
               (it->second == KeyState::PRESSED || it->second == KeyState::JUST_PRESSED || it->second == KeyState::HELD);
    }

    bool InputManager::isKeyHeld(int key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) && (it->second == KeyState::HELD);
    }

    bool InputManager::isKeyJustPressed(int key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) && (it->second == KeyState::JUST_PRESSED);
    }

    bool InputManager::isKeyJustReleased(int key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) && (it->second == KeyState::JUST_RELEASED);
    }

    KeyState InputManager::getKeyState(int key) const {
        auto it = key_states_.find(key);
        return (it != key_states_.end()) ? it->second : KeyState::RELEASED;
    }

    bool InputManager::isMouseButtonPressed(MouseButton button) const {
        auto it = mouse_button_states_.find(button);
        return (it != mouse_button_states_.end()) && 
               (it->second == KeyState::PRESSED || it->second == KeyState::JUST_PRESSED || it->second == KeyState::HELD);
    }

    bool InputManager::isMouseButtonHeld(MouseButton button) const {
        auto it = mouse_button_states_.find(button);
        return (it != mouse_button_states_.end()) && (it->second == KeyState::HELD);
    }

    bool InputManager::isMouseButtonJustPressed(MouseButton button) const {
        auto it = mouse_button_states_.find(button);
        return (it != mouse_button_states_.end()) && (it->second == KeyState::JUST_PRESSED);
    }

    bool InputManager::isMouseButtonJustReleased(MouseButton button) const {
        auto it = mouse_button_states_.find(button);
        return (it != mouse_button_states_.end()) && (it->second == KeyState::JUST_RELEASED);
    }

    KeyState InputManager::getMouseButtonState(MouseButton button) const {
        auto it = mouse_button_states_.find(button);
        return (it != mouse_button_states_.end()) ? it->second : KeyState::RELEASED;
    }

    void InputManager::setMouseCaptured(bool captured) {
        mouse_captured_ = captured;
        
        if (window_) {
            if (captured) {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    }

    void InputManager::addKeyCallback(const KeyCallback& callback) {
        key_callbacks_.push_back(callback);
    }

    void InputManager::addMouseCallback(const MouseCallback& callback) {
        mouse_callbacks_.push_back(callback);
    }

    void InputManager::addScrollCallback(const ScrollCallback& callback) {
        scroll_callbacks_.push_back(callback);
    }

    void InputManager::addMouseButtonCallback(const MouseButtonCallback& callback) {
        mouse_button_callbacks_.push_back(callback);
    }

    void InputManager::clearAllCallbacks() {
        clearKeyCallbacks();
        clearMouseCallbacks();
        clearScrollCallbacks();
        clearMouseButtonCallbacks();
    }

    void InputManager::resetMouseDelta() {
        mouse_state_.delta_x = 0.0;
        mouse_state_.delta_y = 0.0;
    }

    void InputManager::resetScroll() {
        mouse_state_.scroll_x = 0.0;
        mouse_state_.scroll_y = 0.0;
    }

    // Callbacks estáticos para GLFW
    void InputManager::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        InputManager& input = getInstance();
        
        if (action == GLFW_PRESS) {
            input.key_states_[key] = KeyState::JUST_PRESSED;
        } else if (action == GLFW_RELEASE) {
            input.key_states_[key] = KeyState::JUST_RELEASED;
        }
    }

    void InputManager::setUseJoystick(bool use) {
        use_joystick_ = use;
        joystick_present_ = (glfwJoystickPresent(joystick_id_) == GLFW_TRUE);
        if (use_joystick_) {
            std::cout << "Joystick mode enabled" << std::endl;
        } else {
            std::cout << "Joystick mode disabled" << std::endl;
        }
    }

    bool InputManager::isUsingJoystick() const {
        return use_joystick_;
    }

    void InputManager::mouseCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
        InputManager& input = getInstance();
        
        if (input.mouse_state_.first_movement) {
            input.mouse_state_.last_x = xpos;
            input.mouse_state_.last_y = ypos;
            input.mouse_state_.first_movement = false;
        }
        
        input.mouse_state_.delta_x = xpos - input.mouse_state_.last_x;
        input.mouse_state_.delta_y = input.mouse_state_.last_y - ypos; // Invertido para coordenadas de pantalla
        
        input.mouse_state_.last_x = input.mouse_state_.x;
        input.mouse_state_.last_y = input.mouse_state_.y;
        input.mouse_state_.x = xpos;
        input.mouse_state_.y = ypos;
    }

    void InputManager::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
        InputManager& input = getInstance();
        
        input.mouse_state_.scroll_x = xoffset;
        input.mouse_state_.scroll_y = yoffset;
        
        // Procesar callbacks de scroll inmediatamente
        if (input.mouse_enabled_) {
            for (const auto& callback : input.scroll_callbacks_) {
                callback(xoffset, yoffset);
            }
        }
    }

    void InputManager::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        InputManager& input = getInstance();
        
        MouseButton mouse_button = static_cast<MouseButton>(button);
        
        if (action == GLFW_PRESS) {
            input.mouse_button_states_[mouse_button] = KeyState::JUST_PRESSED;
        } else if (action == GLFW_RELEASE) {
            input.mouse_button_states_[mouse_button] = KeyState::JUST_RELEASED;
        }
    }

    // === Implementación de ActionManager ===

    ActionManager::ActionManager() : input_manager_(&InputManager::getInstance()) {
    }

    void ActionManager::bindAction(const std::string& name, const std::vector<int>& keys, const std::function<void()>& callback) {
        Action action;
        action.name = name;
        action.keys = keys;
        action.callback = callback;
        actions_[name] = action;
    }

    void ActionManager::bindAction(const std::string& name, int key, const std::function<void()>& callback) {
        bindAction(name, std::vector<int>{key}, callback);
    }

    void ActionManager::unbindAction(const std::string& name) {
        actions_.erase(name);
    }

    void ActionManager::clearActions() {
        actions_.clear();
    }

    bool ActionManager::isActionTriggered(const std::string& name) const {
        auto it = actions_.find(name);
        return (it != actions_.end()) && it->second.triggered_this_frame;
    }

    bool ActionManager::isActionActive(const std::string& name) const {
        auto it = actions_.find(name);
        if (it == actions_.end()) return false;
        
        const Action& action = it->second;
        
        // Verificar si alguna de las teclas asociadas está presionada
        for (int key : action.keys) {
            if (input_manager_->isKeyPressed(key)) {
                return true;
            }
        }
        
        return false;
    }

    void ActionManager::update() {
        for (auto& [name, action] : actions_) {
            action.triggered_this_frame = false;
            
            // Verificar si alguna tecla fue just pressed
            for (int key : action.keys) {
                if (input_manager_->isKeyJustPressed(key)) {
                    action.triggered_this_frame = true;
                    if (action.callback) {
                        action.callback();
                    }
                    break;
                }
            }
        }
    }

    void ActionManager::bindMovementActions(const std::function<void(float, float)>& movement_callback) {
        // No implementamos el callback directo aquí, solo configuramos las acciones individuales
        bindAction("move_forward", InputManager::KEY_W, [this, movement_callback]() {
            // Esta acción se manejará en el update del juego principal
        });
        
        bindAction("move_backward", InputManager::KEY_S, [this, movement_callback]() {
            // Esta acción se manejará en el update del juego principal
        });
        
        bindAction("move_left", InputManager::KEY_A, [this, movement_callback]() {
            // Esta acción se manejará en el update del juego principal
        });
        
        bindAction("move_right", InputManager::KEY_D, [this, movement_callback]() {
            // Esta acción se manejará en el update del juego principal
        });
    }

    void ActionManager::bindCameraActions(const std::function<void()>& reset_callback,
                                        const std::function<void()>& toggle_mouse_callback) {
        bindAction("reset_camera", InputManager::KEY_R, reset_callback);
        bindAction("toggle_mouse", InputManager::KEY_E, toggle_mouse_callback);
    }

} // namespace Input