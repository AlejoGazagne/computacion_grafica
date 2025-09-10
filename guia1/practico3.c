#include <stdio.h>
#include <stdlib.h>
#include "../ppm/ppm.h"  // Usa la misma librería ppm que en tus ejercicios anteriores

#define WIDTH 800
#define HEIGHT 600
#define MARGIN 20

// Dibuja un píxel y sus 7 simétricos, centrados en (xc, yc)
void drawSymmetricPixels(PPMImage *img, int x, int y, int xc, int yc, int r, int g, int b) {
    setPixel(img, xc + x, yc + y, r, g, b);   // Octante 1
    setPixel(img, xc + y, yc + x, r, g, b);   // Octante 2
    setPixel(img, xc - x, yc + y, r, g, b);   // Octante 3
    setPixel(img, xc - y, yc + x, r, g, b);   // Octante 4
    setPixel(img, xc + x, yc - y, r, g, b);   // Octante 5
    setPixel(img, xc + y, yc - x, r, g, b);   // Octante 6
    setPixel(img, xc - x, yc - y, r, g, b);   // Octante 7
    setPixel(img, xc - y, yc - x, r, g, b);   // Octante 8
}

// Algoritmo de punto medio extendido para circunferencia
void drawCircleMP(PPMImage *img, int xc, int yc, int r, int red, int green, int blue) {
    int x = 0;
    int y = r;
    int d = 1 - r;   // Variable de decisión inicial

    drawSymmetricPixels(img, x, y, xc, yc, red, green, blue);

    while (x < y) {
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        drawSymmetricPixels(img, x, y, xc, yc, red, green, blue);
    }
}

int main() {
    int xc, yc, r;

    printf("Ingrese las coordenadas del centro (xc yc): ");
    scanf("%d %d", &xc, &yc);
    printf("Ingrese el radio (r): ");
    scanf("%d", &r);

    // Crear imagen
    PPMImage *img = createPPMImage(WIDTH, HEIGHT);
    if (img == NULL) {
        printf("Error al crear la imagen\n");
        return 1;
    }

    // Inicializar con fondo blanco
    initializeImage(img, 255, 255, 255);

    // Dibujar sistema de coordenadas
    drawCoordinateSystem(img);

    // Dibujar circunferencia en negro
    setPixel(img, xc, yc, 0, 0, 0); // Centro
    drawCircleMP(img, xc, yc, r, 0, 0, 0);

    // Guardar imagen
    savePPM(img, "circunferencia.ppm");

    // Liberar memoria
    freePPMImage(img);

    printf("Circunferencia guardada en 'circunferencia.ppm'\n");

    return 0;
}
