#ifndef PPM_H
#define PPM_H

#define MAX_COLOR 255

typedef struct {
    int r, g, b;
} Pixel;

typedef struct {
    Pixel **pixels;
    int width;
    int height;
    int min_x;
    int min_y;
} PPMImage;

// Función para convertir coordenadas cartesianas a coordenadas de imagen
int cartesianToImageY(PPMImage *img, int cartesianY);
int cartesianToImageX(PPMImage *img, int cartesianX);

// Crear y destruir imagen
PPMImage* createPPMImage(int width, int height);
void freePPMImage(PPMImage *img);

// Manipulación de píxeles
void setPixel(PPMImage *img, int cartesianX, int cartesianY, int r, int g, int b);
void initializeImage(PPMImage *img, int r, int g, int b);

// Guardar archivo
void savePPM(PPMImage *img, const char *filename);

// Utilidades
void calculateImageDimensions(int x0, int y0, int x1, int y1, int *w, int *h, int min_width, int min_height);
void adjustImageCoordinates(PPMImage *img, int x0, int y0, int x1, int y1);

void drawCartesianAxes(PPMImage *img, int axisColorR, int axisColorG, int axisColorB, int gridColorR, int gridColorG, int gridColorB, int drawGrid);
void drawArrowX(PPMImage *img, int x, int y, int r, int g, int b);
void drawArrowY(PPMImage *img, int x, int y, int r, int g, int b);

#endif