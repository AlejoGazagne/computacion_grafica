#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <chrono>
#include <iostream>

namespace Utils {

    /**
     * @brief Clase para medición de tiempo de rendimiento
     */
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time_;
        std::string name_;

    public:
        Timer(const std::string& name = "Timer") : name_(name) {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

        ~Timer() {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
            std::cout << "[" << name_ << "] " << duration.count() << "μs" << std::endl;
        }

        double getElapsedMilliseconds() const {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time_);
            return duration.count() / 1000.0;
        }
    };

    /**
     * @brief Utilidades para archivos
     */
    namespace FileUtils {
        bool fileExists(const std::string& filepath);
        std::string getFileExtension(const std::string& filepath);
        std::string getFileDirectory(const std::string& filepath);
        std::string getFileName(const std::string& filepath);
        std::vector<std::string> listFilesInDirectory(const std::string& directory, const std::string& extension = "");
    }

    /**
     * @brief Utilidades matemáticas
     */
    namespace MathUtils {
        constexpr float PI = 3.14159265359f;
        constexpr float TWO_PI = 2.0f * PI;
        constexpr float HALF_PI = PI * 0.5f;
        
        float radians(float degrees);
        float degrees(float radians);
        float clamp(float value, float min, float max);
        float lerp(float a, float b, float t);
        bool isPowerOfTwo(unsigned int value);
        unsigned int nextPowerOfTwo(unsigned int value);
    }

    /**
     * @brief Logging simple
     */
    namespace Log {
        enum class Level {
            DEBUG,
            INFO,
            WARNING,
            ERROR
        };

        void setLevel(Level level);
        void debug(const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);
    }

    /**
     * @brief Utilidades de strings
     */
    namespace StringUtils {
        std::vector<std::string> split(const std::string& str, char delimiter);
        std::string trim(const std::string& str);
        std::string toLower(const std::string& str);
        std::string toUpper(const std::string& str);
        bool startsWith(const std::string& str, const std::string& prefix);
        bool endsWith(const std::string& str, const std::string& suffix);
    }

} // namespace Utils

// Macro para medición rápida de tiempo
#define MEASURE_TIME(name) Utils::Timer timer(name)

#endif // UTILS_H