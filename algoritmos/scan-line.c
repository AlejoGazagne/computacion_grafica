#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_EDGES 1000
#define MAX_VERTICES 100

typedef struct {
    int ymax;     // Coordenada y máxima
    float x;      // Intersección x actual
    float dx;     // 1/m = incremento de x por scanline
} Edge;

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point vertices[MAX_VERTICES];
    int n; // cantidad de vértices
} Polygon;

Edge *GET[MAX_EDGES];   // Global Edge Table
int GET_size[MAX_EDGES];
Edge AET[MAX_EDGES];    // Active Edge Table
int AET_size = 0;

// Construcción de la tabla global de aristas
void buildSortedEdgeTable(Polygon *poly, int *ymin_out, int *ymax_out) {
    int i;
    *ymin_out = 99999;
    *ymax_out = -99999;

    for (i = 0; i < MAX_EDGES; i++) {
        GET[i] = NULL;
        GET_size[i] = 0;
    }

    for (i = 0; i < poly->n; i++) {
        Point p1 = poly->vertices[i];
        Point p2 = poly->vertices[(i + 1) % poly->n]; // siguiente vértice

        if (p1.y == p2.y) continue; // ignorar horizontales

        int yMinLocal = (p1.y < p2.y) ? p1.y : p2.y;
        int yMaxLocal = (p1.y > p2.y) ? p1.y : p2.y;
        float x = (p1.y < p2.y) ? p1.x : p2.x; // x inicial
        float dx = (float)(p2.x - p1.x) / (float)(p2.y - p1.y);

        Edge e = { yMaxLocal, x, dx };

        if (GET[yMinLocal] == NULL) {
            GET[yMinLocal] = (Edge*)malloc(MAX_EDGES * sizeof(Edge));
        }
        GET[yMinLocal][GET_size[yMinLocal]++] = e;

        if (yMinLocal < *ymin_out) *ymin_out = yMinLocal;
        if (yMaxLocal > *ymax_out) *ymax_out = yMaxLocal;
    }
}

// Agregar aristas nuevas de GET a AET
void addNewEdges(int y) {
    if (GET[y] == NULL) return;
    for (int i = 0; i < GET_size[y]; i++) {
        AET[AET_size++] = GET[y][i];
    }
}

// Eliminar aristas que terminan en y
void removeCompletedEdges(int y) {
    int i = 0;
    while (i < AET_size) {
        if (AET[i].ymax == y) {
            for (int j = i; j < AET_size - 1; j++) {
                AET[j] = AET[j + 1];
            }
            AET_size--;
        } else {
            i++;
        }
    }
}

// Actualizar intersecciones x
void updateActiveEdges() {
    for (int i = 0; i < AET_size; i++) {
        AET[i].x += AET[i].dx;
    }
}

// Comparar por x para ordenar
int cmpEdge(const void *a, const void *b) {
    Edge *ea = (Edge*)a;
    Edge *eb = (Edge*)b;
    if (ea->x < eb->x) return -1;
    else if (ea->x > eb->x) return 1;
    return 0;
}

// Dibujar spans
void fillSpans(int y) {
    qsort(AET, AET_size, sizeof(Edge), cmpEdge);
    for (int i = 0; i < AET_size; i += 2) {
        int x1 = (int)(AET[i].x + 0.5);
        int x2 = (int)(AET[i + 1].x + 0.5);
        glBegin(GL_POINTS);
        for (int x = x1; x <= x2; x++) {
            glVertex2i(x, y);
        }
        glEnd();
    }
}

// Algoritmo principal
void scanLineFill(Polygon *poly) {
    int ymin, ymax;
    buildSortedEdgeTable(poly, &ymin, &ymax);

    AET_size = 0;

    for (int y = ymin; y <= ymax; y++) {
        addNewEdges(y);
        if (AET_size > 0) fillSpans(y);
        removeCompletedEdges(y);
        updateActiveEdges();
    }
}

// Dibujar ejemplo
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    Polygon poly;
    poly.n = 4;
    poly.vertices[0].x = 100; poly.vertices[0].y = 100;
    poly.vertices[1].x = 200; poly.vertices[1].y = 300;
    poly.vertices[2].x = 300; poly.vertices[2].y = 200;
    poly.vertices[3].x = 250; poly.vertices[3].y = 100;

    glColor3f(1.0, 0.0, 0.0); // rojo
    scanLineFill(&poly);

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Scanline Fill en C");
    glClearColor(1.0, 1.0, 1.0, 1.0); // fondo blanco
    glColor3f(0.0, 0.0, 0.0);
    gluOrtho2D(0, 500, 0, 500);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
