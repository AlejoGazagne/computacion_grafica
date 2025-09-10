#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ppm.h"

#define MARGIN 20

int cartesianToImageX(PPMImage *img, int cartesianX) {
    return cartesianX - img->min_x;
}

int cartesianToImageY(PPMImage *img, int cartesianY) {
    // Invertir eje Y
    return img->height - 1 - (cartesianY - img->min_y);
}

PPMImage* createPPMImage(int width, int height) {
    PPMImage *img = (PPMImage*)malloc(sizeof(PPMImage));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->min_x = -(width / 2);
    img->min_y = -(height / 2);
    
    // Asignar memoria para los píxeles
    img->pixels = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int y = 0; y < height; y++) {
        img->pixels[y] = (Pixel*)malloc(width * sizeof(Pixel));
    }
    
    return img;
}

void freePPMImage(PPMImage *img) {
    if (img != NULL) {
        if (img->pixels != NULL) {
            for (int y = 0; y < img->height; y++) {
                free(img->pixels[y]);
            }
            free(img->pixels);
        }
        free(img);
    }
}

void setPixel(PPMImage *img, int cartesianX, int cartesianY, int r, int g, int b) {
    int x = cartesianToImageX(img, cartesianX);
    int y = cartesianToImageY(img, cartesianY);
    
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        img->pixels[y][x] = (Pixel){r, g, b};
    }
}

void initializeImage(PPMImage *img, int r, int g, int b) {
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->pixels[y][x] = (Pixel){r, g, b};
        }
    }
}

void savePPM(PPMImage *img, const char *filename) {
    if (img == NULL || img->pixels == NULL) {
        printf("Error: La imagen no ha sido inicializada\n");
        return;
    }
    
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error al crear el archivo %s\n", filename);
        return;
    }
    
    // Encabezado PPM
    fprintf(file, "P3\n");
    fprintf(file, "%d %d\n", img->width, img->height);
    fprintf(file, "%d\n", MAX_COLOR);
    
    // Datos de la imagen
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            fprintf(file, "%d %d %d ", 
                   img->pixels[y][x].r, 
                   img->pixels[y][x].g, 
                   img->pixels[y][x].b);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("Imagen guardada como %s\n", filename);
    printf("Dimensiones: %d x %d pixels\n", img->width, img->height);
    printf("Rango X: %d a %d\n", img->min_x, img->min_x + img->width - 1);
    printf("Rango Y: %d a %d\n", img->min_y, img->min_y + img->height - 1);
}

void calculateImageDimensions(int x0, int y0, int x1, int y1, int *w, int *h, int min_width, int min_height) {
    // Encontrar los valores mínimos y máximos
    int min_x = (x0 < x1) ? x0 : x1;
    int min_y = (y0 < y1) ? y0 : y1;
    int max_x = (x0 > x1) ? x0 : x1;
    int max_y = (y0 > y1) ? y0 : y1;
    
    // Calcular dimensiones con margen
    int required_width = (max_x - min_x) + 2 * MARGIN;
    int required_height = (max_y - min_y) + 2 * MARGIN;
    
    // Usar el tamaño requerido o el mínimo, el que sea mayor
    *w = (required_width > min_width) ? required_width : min_width;
    *h = (required_height > min_height) ? required_height : min_height;
}


void adjustImageCoordinates(PPMImage *img, int x0, int y0, int x1, int y1) {
    // Encontrar los valores mínimos y máximos de la línea
    int line_min_x = (x0 < x1) ? x0 : x1;
    int line_min_y = (y0 < y1) ? y0 : y1;
    int line_max_x = (x0 > x1) ? x0 : x1;
    int line_max_y = (y0 > y1) ? y0 : y1;
    
    // Calcular dimensiones necesarias con margen
    int required_width = (line_max_x - line_min_x) + 2 * MARGIN;
    int required_height = (line_max_y - line_min_y) + 2 * MARGIN;
    
    // Ajustar min_x y min_y para centrar el contenido si es necesario
    if (required_width < img->width) {
        // Centrar horizontalmente
        img->min_x = line_min_x - (img->width - required_width) / 2;
    } else {
        // Usar margen estándar
        img->min_x = line_min_x - MARGIN;
    }
    
    if (required_height < img->height) {
        // Centrar verticalmente
        img->min_y = line_min_y - (img->height - required_height) / 2;
    } else {
        // Usar margen estándar
        img->min_y = line_min_y - MARGIN;
    }
}
void drawCartesianAxes(PPMImage *img, int axisColorR, int axisColorG, int axisColorB, int gridColorR, int gridColorG, int gridColorB, int drawGrid) {
    if (img == NULL || img->pixels == NULL) {
        printf("Error: La imagen no ha sido inicializada\n");
        return;
    }
    
    // Dibujar ejes principales (X e Y)
    for (int x = img->min_x; x < img->min_x + img->width; x++) {
        // Eje Y (x = 0)
        setPixel(img, 0, x, axisColorR, axisColorG, axisColorB);
        
        // Eje X (y = 0)
        setPixel(img, x, 0, axisColorR, axisColorG, axisColorB);
    }
    
    // Dibujar grid si está habilitado
    if (drawGrid) {
        // Grid vertical (líneas paralelas al eje Y)
        for (int x = img->min_x; x < img->min_x + img->width; x++) {
            if (x % 10 == 0 && x != 0) { // Cada 10 unidades
                for (int y = img->min_y; y < img->min_y + img->height; y++) {
                    setPixel(img, x, y, gridColorR, gridColorG, gridColorB);
                    setPixel(img, -x, y, gridColorR, gridColorG, gridColorB);
                }
            }
        }
        
        // Grid horizontal (líneas paralelas al eje X)
        for (int y = img->min_y; y < img->min_y + img->height; y++) {
            if (y % 10 == 0 && y != 0) { // Cada 10 unidades
                for (int x = img->min_x; x < img->min_x + img->width; x++) {
                    setPixel(img, x, y, gridColorR, gridColorG, gridColorB);
                    setPixel(img, x, -y, gridColorR, gridColorG, gridColorB);
                }
            }
        }
    }
    
    // Dibujar marcas de graduación en los ejes
    // for (int i = -1000; i < 1000; i += 10) { // Cada 10 unidades
    //     if (i != 0) {
    //         // Marcas en el eje X
    //         setPixel(img, i, 2, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, i, -2, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, i, 1, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, i, -1, axisColorR, axisColorG, axisColorB);
            
    //         // Marcas en el eje Y
    //         setPixel(img, 2, i, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, -2, i, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, 1, i, axisColorR, axisColorG, axisColorB);
    //         setPixel(img, -1, i, axisColorR, axisColorG, axisColorB);
    //     }
    // }
    
    // Dibujar flechas en los extremos de los ejes
    // drawArrowX(img, img->min_x + img->width - 5, 0, axisColorR, axisColorG, axisColorB);
    // drawArrowY(img, 0, img->min_y + img->height - 5, axisColorR, axisColorG, axisColorB);
}

// Función auxiliar para dibujar flecha en el eje X
void drawArrowX(PPMImage *img, int x, int y, int r, int g, int b) {
    setPixel(img, x, y, r, g, b);
    setPixel(img, x-1, y-1, r, g, b);
    setPixel(img, x-1, y+1, r, g, b);
    setPixel(img, x-2, y-2, r, g, b);
    setPixel(img, x-2, y+2, r, g, b);
}

// Función auxiliar para dibujar flecha en el eje Y
void drawArrowY(PPMImage *img, int x, int y, int r, int g, int b) {
    setPixel(img, x, y, r, g, b);
    setPixel(img, x-1, y-1, r, g, b);
    setPixel(img, x+1, y-1, r, g, b);
    setPixel(img, x-2, y-2, r, g, b);
    setPixel(img, x+2, y-2, r, g, b);
}