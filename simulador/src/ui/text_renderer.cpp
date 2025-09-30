#include "text_renderer.h"
#include <cmath>

namespace UI {

    std::vector<float> TextRenderer::generateNumberVertices(
        int number, 
        float x, 
        float y, 
        float digit_width, 
        float digit_height
    ) {
        std::vector<float> vertices;
        
        // Convertir el número a string para procesarlo dígito por dígito - solo valores positivos
        std::string number_str = std::to_string(abs(number));
        
        float current_x = x - (number_str.length() * digit_width * 0.6f); // Centrar texto
        
        for (char c : number_str) {
            int digit = c - '0';
            
            // Generar vértices para este dígito
            std::vector<float> digit_vertices = generateDigitVertices(
                digit, current_x, y, digit_width, digit_height
            );
            
            // Agregar los vértices del dígito al vector total
            vertices.insert(vertices.end(), digit_vertices.begin(), digit_vertices.end());
            
            current_x += digit_width * 1.2f; // Espaciado entre caracteres
        }
        
        return vertices;
    }

    std::vector<float> TextRenderer::generateDigitVertices(
        int digit, 
        float x, 
        float y, 
        float width, 
        float height
    ) {
        std::vector<float> vertices;
        
        switch (digit) {
            case 0:
                // Rectángulo (0)
                vertices.insert(vertices.end(), {
                    x, y, x + width, y,
                    x + width, y, x + width, y + height,
                    x + width, y + height, x, y + height,
                    x, y + height, x, y
                });
                break;
            case 1:
                // Línea vertical (1)
                vertices.insert(vertices.end(), {
                    x + width*0.5f, y, x + width*0.5f, y + height
                });
                break;
            case 2:
                // Número 2
                vertices.insert(vertices.end(), {
                    x, y + height, x + width, y + height,  // Línea superior
                    x + width, y + height, x + width, y + height*0.5f,  // Lado derecho superior
                    x + width, y + height*0.5f, x, y + height*0.5f,  // Línea media
                    x, y + height*0.5f, x, y,  // Lado izquierdo inferior
                    x, y, x + width, y  // Línea inferior
                });
                break;
            case 3:
                // Número 3
                vertices.insert(vertices.end(), {
                    x, y + height, x + width, y + height,  // Línea superior
                    x + width, y + height, x + width, y + height*0.5f,  // Lado derecho superior
                    x + width*0.5f, y + height*0.5f, x + width, y + height*0.5f,  // Línea media
                    x + width, y + height*0.5f, x + width, y,  // Lado derecho inferior
                    x + width, y, x, y  // Línea inferior
                });
                break;
            case 4:
                // Número 4
                vertices.insert(vertices.end(), {
                    x, y + height, x, y + height*0.5f,  // Lado izquierdo superior
                    x, y + height*0.5f, x + width, y + height*0.5f,  // Línea media
                    x + width, y + height, x + width, y  // Lado derecho completo
                });
                break;
            case 5:
                // Número 5
                vertices.insert(vertices.end(), {
                    x + width, y + height, x, y + height,  // Línea superior
                    x, y + height, x, y + height*0.5f,  // Lado izquierdo superior
                    x, y + height*0.5f, x + width, y + height*0.5f,  // Línea media
                    x + width, y + height*0.5f, x + width, y,  // Lado derecho inferior
                    x + width, y, x, y  // Línea inferior
                });
                break;
            case 6:
                // Número 6
                vertices.insert(vertices.end(), {
                    x + width, y + height, x, y + height,  // Línea superior
                    x, y + height, x, y,  // Lado izquierdo completo
                    x, y, x + width, y,  // Línea inferior
                    x + width, y, x + width, y + height*0.5f,  // Lado derecho inferior
                    x + width, y + height*0.5f, x, y + height*0.5f  // Línea media
                });
                break;
            case 7:
                // Número 7
                vertices.insert(vertices.end(), {
                    x, y + height, x + width, y + height,  // Línea superior
                    x + width, y + height, x + width, y  // Lado derecho completo
                });
                break;
            case 8:
                // Número 8
                vertices.insert(vertices.end(), {
                    x, y, x + width, y,  // Línea inferior
                    x + width, y, x + width, y + height,  // Lado derecho
                    x + width, y + height, x, y + height,  // Línea superior
                    x, y + height, x, y,  // Lado izquierdo
                    x, y + height*0.5f, x + width, y + height*0.5f  // Línea media
                });
                break;
            case 9:
                // Número 9
                vertices.insert(vertices.end(), {
                    x, y + height, x + width, y + height,  // Línea superior
                    x + width, y + height, x + width, y,  // Lado derecho completo
                    x + width, y, x, y,  // Línea inferior
                    x, y, x, y + height*0.5f,  // Lado izquierdo superior
                    x, y + height*0.5f, x + width, y + height*0.5f  // Línea media
                });
                break;
            default:
                // Para dígitos desconocidos, no renderizar nada
                break;
        }
        
        return vertices;
    }

} // namespace UI