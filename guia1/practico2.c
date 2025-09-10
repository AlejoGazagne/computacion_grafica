#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../ppm/ppm.h"

typedef struct {
    int x0, y0, x1, y1;
    const char *name;
} TestCase;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point *points;
    int count;
} PixelList;

// Algoritmo DDA (devuelve lista de píxeles)
PixelList getLineDDAPixels(int x0, int y0, int x1, int y1) {
    PixelList list;
    list.points = NULL;
    list.count = 0;
    
    int dx = x1 - x0;
    int dy = y1 - y0;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    
    if (steps == 0) {
        list.points = malloc(sizeof(Point));
        list.points[0] = (Point){x0, y0};
        list.count = 1;
        return list;
    }
    
    list.points = malloc((steps + 1) * sizeof(Point));
    
    float xIncrement = dx / (float)steps;
    float yIncrement = dy / (float)steps;
    
    float x = x0;
    float y = y0;
    
    for (int i = 0; i <= steps; i++) {
        list.points[i] = (Point){round(x), round(y)};
        list.count++;
        x += xIncrement;
        y += yIncrement;
    }
    
    return list;
}

// Algoritmo de Bresenham (devuelve lista de píxeles)
PixelList getLineBresenhamPixels(int x0, int y0, int x1, int y1) {
    PixelList list;
    list.points = NULL;
    list.count = 0;
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    // Calcular número máximo de puntos
    int max_points = (dx > dy ? dx : dy) + 1;
    list.points = malloc(max_points * sizeof(Point));
    
    int index = 0;
    while (1) {
        list.points[index++] = (Point){x0, y0};
        list.count++;
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    
    return list;
}

// Función para desplazar una línea a una nueva posición
void desplazarLinea(TestCase *testCase, int desplazamientoX, int desplazamientoY) {
    testCase->x0 += desplazamientoX;
    testCase->y0 += desplazamientoY;
    testCase->x1 += desplazamientoX;
    testCase->y1 += desplazamientoY;
}

// Función para encontrar los límites de todas las líneas
void findOverallBounds(TestCase *testCases, int numTests, int *min_x, int *max_x, int *min_y, int *max_y) {
    *min_x = *max_x = *min_y = *max_y = 0;
    
    for (int i = 0; i < numTests; i++) {
        PixelList ddaPixels = getLineDDAPixels(
            testCases[i].x0, testCases[i].y0, 
            testCases[i].x1, testCases[i].y1
        );
        
        PixelList bresenhamPixels = getLineBresenhamPixels(
            testCases[i].x0, testCases[i].y0, 
            testCases[i].x1, testCases[i].y1
        );
        
        // Verificar puntos DDA
        for (int j = 0; j < ddaPixels.count; j++) {
            if (ddaPixels.points[j].x < *min_x) *min_x = ddaPixels.points[j].x;
            if (ddaPixels.points[j].x > *max_x) *max_x = ddaPixels.points[j].x;
            if (ddaPixels.points[j].y < *min_y) *min_y = ddaPixels.points[j].y;
            if (ddaPixels.points[j].y > *max_y) *max_y = ddaPixels.points[j].y;
        }
        
        // Verificar puntos Bresenham
        for (int j = 0; j < bresenhamPixels.count; j++) {
            if (bresenhamPixels.points[j].x < *min_x) *min_x = bresenhamPixels.points[j].x;
            if (bresenhamPixels.points[j].x > *max_x) *max_x = bresenhamPixels.points[j].x;
            if (bresenhamPixels.points[j].y < *min_y) *min_y = bresenhamPixels.points[j].y;
            if (bresenhamPixels.points[j].y > *max_y) *max_y = bresenhamPixels.points[j].y;
        }
        
        free(ddaPixels.points);
        free(bresenhamPixels.points);
    }
}

// Función para dibujar todas las líneas en una sola imagen
void createAllInOneImage(TestCase *testCases, int numTests, const char *filename) {
    int min_x, max_x, min_y, max_y;
    findOverallBounds(testCases, numTests, &min_x, &max_x, &min_y, &max_y);
    
    // Agregar margen
    int margin = 20;
    int width = (max_x - min_x) + 2 * margin + 1;
    int height = (max_y - min_y) + 2 * margin + 1;
    
    // Asegurar tamaño mínimo
    if (width < 300) width = 300;
    if (height < 300) height = 300;
    
    // Crear imagen usando tu clase PPM
    PPMImage *img = createPPMImage(width, height);
    img->min_x = min_x - margin;
    img->min_y = min_y - margin;
    
    // Inicializar fondo blanco
    initializeImage(img, 255, 255, 255);
    
    // Dibujar ejes de coordenadas
    // Eje X (y = 0)
    for (int x = img->min_x; x < img->min_x + img->width; x++) {
        setPixel(img, x, 0, 200, 200, 200);
    }
    // Eje Y (x = 0)
    for (int y = img->min_y; y < img->min_y + img->height; y++) {
        setPixel(img, 0, y, 200, 200, 200);
    }
    
    // Colores para diferentes líneas
    int colors[][3] = {
        {255, 0, 0},      // Rojo - DDA
        {0, 0, 255},      // Azul - Bresenham
        {0, 128, 0},      // Verde oscuro - DDA
        {255, 0, 255},    // Magenta - Bresenham
        {0, 128, 128},    // Verde azulado - DDA
        {255, 128, 0},    // Naranja - Bresenham
        {128, 0, 128},    // Púrpura - DDA
        {0, 0, 128},      // Azul oscuro - Bresenham
        {128, 128, 0},    // Oliva - DDA
        {128, 0, 0}       // Marrón - Bresenham
    };
    
    // Dibujar todas las líneas
    for (int i = 0; i < numTests; i++) {
        PixelList ddaPixels = getLineDDAPixels(
            testCases[i].x0, testCases[i].y0, 
            testCases[i].x1, testCases[i].y1
        );
        
        PixelList bresenhamPixels = getLineBresenhamPixels(
            testCases[i].x0, testCases[i].y0, 
            testCases[i].x1, testCases[i].y1
        );
        
        // Dibujar línea DDA (más gruesa para mejor visibilidad)
        for (int j = 0; j < ddaPixels.count; j++) {
            setPixel(img, ddaPixels.points[j].x, ddaPixels.points[j].y, 
                    colors[i*2 % 10][0], colors[i*2 % 10][1], colors[i*2 % 10][2]);
            // Hacer la línea un poco más gruesa
            setPixel(img, ddaPixels.points[j].x + 1, ddaPixels.points[j].y, 
                    colors[i*2 % 10][0], colors[i*2 % 10][1], colors[i*2 % 10][2]);
        }
        
        // Dibujar línea Bresenham
        for (int j = 0; j < bresenhamPixels.count; j++) {
            setPixel(img, bresenhamPixels.points[j].x, bresenhamPixels.points[j].y, 
                    colors[(i*2+1) % 10][0], colors[(i*2+1) % 10][1], colors[(i*2+1) % 10][2]);
        }
        
        free(ddaPixels.points);
        free(bresenhamPixels.points);
    }
    
    // Guardar imagen
    savePPM(img, filename);
    printf("Imagen completa guardada como: %s\n", filename);
    printf("Contiene %d pares de líneas comparadas\n", numTests);
    printf("Dimensiones: %d x %d píxeles\n", width, height);
    
    freePPMImage(img);
}

// Función para comparar dos listas de píxeles
void comparePixelLists(PixelList ddaList, PixelList bresenhamList, const char *lineName) {
    printf("\n=== %s ===\n", lineName);
    
    // Mostrar puntos por consola
    printf("Puntos DDA (%d): ", ddaList.count);
    for (int i = 0; i < ddaList.count; i++) {
        printf("(%d,%d) ", ddaList.points[i].x, ddaList.points[i].y);
    }
    printf("\n");
    
    printf("Puntos Bresenham (%d): ", bresenhamList.count);
    for (int i = 0; i < bresenhamList.count; i++) {
        printf("(%d,%d) ", bresenhamList.points[i].x, bresenhamList.points[i].y);
    }
    printf("\n");
    
    // Verificar si son idénticos
    int identical = 1;
    if (ddaList.count != bresenhamList.count) {
        identical = 0;
        printf("❌ DIFERENTE número de puntos: DDA=%d, Bresenham=%d\n", 
               ddaList.count, bresenhamList.count);
    } else {
        for (int i = 0; i < ddaList.count; i++) {
            if (ddaList.points[i].x != bresenhamList.points[i].x || 
                ddaList.points[i].y != bresenhamList.points[i].y) {
                identical = 0;
                printf("❌ DIFERENCIA en punto %d: DDA(%d,%d) vs Bresenham(%d,%d)\n",
                       i, ddaList.points[i].x, ddaList.points[i].y,
                       bresenhamList.points[i].x, bresenhamList.points[i].y);
                break;
            }
        }
    }
    
    if (identical) {
        printf("✅ IDÉNTICOS - Ambos algoritmos generan los mismos píxeles\n");
    }
    
    printf("---\n");
}

// Función para liberar memoria de las listas
void freePixelList(PixelList list) {
    if (list.points != NULL) {
        free(list.points);
    }
}

// Función principal de prueba
void testAlgorithms() {
    printf("COMPARACIÓN DE ALGORITMOS DDA vs BRESENHAM\n");
    printf("==========================================\n");
    
    // Casos de prueba originales (sin desplazar)
    TestCase testCasesOriginal[] = {
        {0, 0, 8, 20, "Línea (0,0) a (8,20) - Pendiente > 1"},
        {0, 0, 20, 8, "Línea (0,0) a (20,8) - Pendiente < 1"},
        {0, 0, 15, 15, "Línea (0,0) a (15,15) - Pendiente = 1"},
        {0, 0, 0, 25, "Línea vertical (0,0) a (0,25)"},
        {0, 0, 25, 0, "Línea horizontal (0,0) a (25,0)"},
        {0, 0, 20, -15, "Línea (0,0) a (20,-15) - Pendiente negativa"},
        {0, 0, 12, 18, "Línea (0,0) a (12,18) - Pendiente pronunciada"},
        {0, 0, 18, 6, "Línea (0,0) a (18,6) - Pendiente suave"}
    };
    
    int numTests = sizeof(testCasesOriginal) / sizeof(testCasesOriginal[0]);
    TestCase testCases[numTests];
    
    // Copiar y desplazar las líneas para que no se superpongan
    int desplazamientoX = 0;
    int desplazamientoY = 0;
    
    for (int i = 0; i < numTests; i++) {
        testCases[i] = testCasesOriginal[i];
        
        // Desplazar cada línea a una posición diferente
        desplazamientoX = (i % 4) * 40;  // 4 columnas
        desplazamientoY = (i / 4) * 40;  // 2 filas (para 8 líneas)
        
        desplazarLinea(&testCases[i], desplazamientoX - 60, desplazamientoY - 40);
    }
    
    // Primero crear la imagen con todas las líneas
    createAllInOneImage(testCases, numTests, "todas_las_comparaciones.ppm");
    
    // Luego hacer la comparación detallada por consola (usando las originales)
    for (int i = 0; i < numTests; i++) {
        PixelList ddaPixels = getLineDDAPixels(
            testCasesOriginal[i].x0, testCasesOriginal[i].y0, 
            testCasesOriginal[i].x1, testCasesOriginal[i].y1
        );
        
        PixelList bresenhamPixels = getLineBresenhamPixels(
            testCasesOriginal[i].x0, testCasesOriginal[i].y0, 
            testCasesOriginal[i].x1, testCasesOriginal[i].y1
        );
        
        comparePixelLists(ddaPixels, bresenhamPixels, testCasesOriginal[i].name);
        
        freePixelList(ddaPixels);
        freePixelList(bresenhamPixels);
    }
    
    printf("\n🎯 RESUMEN: Todas las líneas se han dibujado en 'todas_las_comparaciones.ppm'\n");
    printf("   - Las líneas han sido desplazadas para evitar superposiciones\n");
    printf("   - Líneas DDA: colores sólidos y más gruesas\n");
    printf("   - Líneas Bresenham: colores alternos y delgadas\n");
    printf("   - Cada par de algoritmos para una línea tiene colores similares\n");
}

int main() {
    testAlgorithms();
    return 0;
}