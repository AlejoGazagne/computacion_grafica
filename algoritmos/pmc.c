#include <stdio.h>

// Función para dibujar un píxel y sus 7 simétricos, centrados en (xc, yc)
void write_pixel_centered(int x, int y, int xc, int yc) {
    printf("%d, %d\n", xc + x, yc + y);   // Octante 1
    printf("%d, %d\n", xc + y, yc + x);   // Octante 2
    printf("%d, %d\n", xc - x, yc + y);   // Octante 3
    printf("%d, %d\n", xc - y, yc + x);   // Octante 4
    printf("%d, %d\n", xc + x, yc - y);   // Octante 5
    printf("%d, %d\n", xc + y, yc - x);   // Octante 6
    printf("%d, %d\n", xc - x, yc - y);   // Octante 7
    printf("%d, %d\n", xc - y, yc - x);   // Octante 8
}

void drawCircleMP(int xc, int yc, int r) {
    int des_var = 5 - 4 * r;
    int dd_ady = 12;
    int dd_ainf = 20 - 8 * r;
    int x = 0;
    int y = r;

    write_pixel_centered(x, y, xc, yc);
    x++;

    while (y > x) {
        if (des_var < 0) {
            des_var += dd_ady;
        } else {
            des_var += dd_ainf;
            y--;
        }
        dd_ady = 8 * x + 12;
        dd_ainf = 8 * x - 8 * y + 20;
        write_pixel_centered(x, y, xc, yc);
        x++;
    }
}

int main() {
    int xc, yc, r;
    
    printf("Ingrese las coordenadas del centro (xc yc): ");
    scanf("%d %d", &xc, &yc);
    printf("Ingrese el radio (r): ");
    scanf("%d", &r);

    printf("\nCircunferencia de radio %d centrada en (%d, %d):\n", r, xc, yc);
    drawCircleMP(xc, yc, r);
    
    return 0;
}