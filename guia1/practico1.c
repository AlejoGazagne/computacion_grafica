#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../ppm/ppm.h"

#define MIN_WIDTH 800
#define MIN_HEIGHT 600

// Algoritmo de Bresenham
void drawLineBresenham(PPMImage *img, int x0, int y0, int x1, int y1, int r, int g, int b) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    int err = dx - dy;
    int e2;
    
    while (1) {
        setPixel(img, x0, y0, r, g, b);
        
        if (x0 == x1 && y0 == y1) break;
        
        e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int main(int argc, char *argv[]) {
    int x0, y0, x1, y1;
    int img_width, img_height;
    
    // Verificar argumentos de línea de comandos
    if (argc == 5) {
        x0 = atoi(argv[1]);
        y0 = atoi(argv[2]);
        x1 = atoi(argv[3]);
        y1 = atoi(argv[4]);
        printf("Coordenadas de la línea: (%d, %d) a (%d, %d)\n", x0, y0, x1, y1);
    } else {
        // Valores por defecto
        x0 = 100; y0 = 100;
        x1 = 700; y1 = 500;
        printf("Usando coordenadas por defecto: (%d, %d) a (%d, %d)\n", x0, y0, x1, y1);
        printf("Uso: %s x0 y0 x1 y1\n", argv[0]);
    }
    
    // Calcular dimensiones de la imagen
    calculateImageDimensions(x0, y0, x1, y1, &img_width, &img_height, MIN_WIDTH, MIN_HEIGHT);
    
    // Crear imagen
    PPMImage *img = createPPMImage(img_width, img_height);
    if (img == NULL) {
        printf("Error: No se pudo crear la imagen\n");
        return 1;
    }
    
    // Inicializar con fondo blanco
    initializeImage(img, 255, 255, 255);

    // Dibujar ejes con grid (rojo para ejes, gris claro para grid)
    drawCartesianAxes(img, 255, 0, 0, 200, 200, 200, 1);
    
    // Dibujar línea con Bresenham (color azul)
    drawLineBresenham(img, x0, y0, x1, y1, 0, 0, 255);
    
    // Guardar imagen
    savePPM(img, "linea_bresenham.ppm");
    
    // Liberar memoria
    freePPMImage(img);
    
    return 0;
}