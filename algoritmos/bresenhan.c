#include <stdio.h>
#include <stdlib.h> // Para abs()

void drawLineBresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1; // Dirección en X
    int sy = (y0 < y1) ? 1 : -1; // Dirección en Y

    int err = dx - dy;
    int e2;

    while (1) {
        printf("%d, %d\n", x0, y0);
        if (x0 == x1 && y0 == y1) break; // Fin de la línea

        e2 = 2 * err;
        
        if (e2 > -dy) { // Avanzar en X
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) { // Avanzar en Y
            err += dx;
            y0 += sy;
        }
    }
}

int main() {
    printf("Línea de (1, 1) a (6, 6):\n");
    drawLineBresenham(-2, 1, 6, -16);
    return 0;
}