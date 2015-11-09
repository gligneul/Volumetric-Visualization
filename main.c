/*
 * PUC-Rio
 * Análise Numérica 2015.2
 * Professor: Waldemar Celes
 * Gabriel de Quadros Ligneul 1212560
 * Matheus Telles
 * Trabalho Final: Visualização Volumétrica
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NX 256
#define NY 256
#define NZ 99

#define IMG_NX 128
#define IMG_NY 99

#define idx(i, j, k) ((k) * NY * NX + (j) * NX + (i))

static unsigned char *readctscan (const char *path);
static unsigned char *visualize (unsigned char *volume);
static double getvalue (unsigned char *volume, int i, int j, int k, double s);
static int writepgm (const char *path, unsigned char *buffer, int nx, int ny);

int main () {
    unsigned char *volume = readctscan("head-8bit.raw");
    if (!volume) {
        fputs("Unable to open the .raw file\n", stderr);
        return 1;
    }

    unsigned char *image = visualize(volume);
    free(volume);
    if (writepgm("out.pgm", image, IMG_NX, IMG_NY)) {
        fputs("Unable to write the output image\n", stderr);
        return 1;
    }

    free(image);
    return 0;
}

static unsigned char *readctscan (const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return NULL;

    int size = NX * NY * NZ;
    unsigned char *buffer = (unsigned char *)malloc(size);
    if (!buffer)
        goto error;

    for (int i = 0; i < size; ++i)
        if (fscanf(file, "%c", &buffer[i]) != 1)
            goto error;

    fclose(file);
    return buffer;

  error:
    free(buffer);
    fclose(file);
    return NULL;
}

static unsigned char *visualize (unsigned char *volume) {
    unsigned char *image = (unsigned char *)malloc(IMG_NX * IMG_NY);

    for (int i = 0; i < IMG_NX; ++i) {
        for (int j = 0; j < IMG_NY; ++j) {
            double ray1 = getvalue(volume, 2 * i, 0, j, 100);
            double ray2 = getvalue(volume, 2 * i + 1, 0, j, 100);
            image[i + IMG_NX * j] = (unsigned char)(255 * (ray1 + ray2) / 2);
        }
    }

    return image;
}

static double getvalue (unsigned char *volume, int i, int j, int k, double s) {
    int s1 = floor(s);
    int s2 = ceil(s);

    if (s1 > NY || s2 > NY)
        return volume[idx(i, NY - 1, k)] / 255.0;

    double v1 = volume[idx(i, j + s1, k)] / 255.0;
    double v2 = volume[idx(i, j + s2, k)] / 255.0;
    double intpart;
    double alpha = modf(s, &intpart);
    return v1 * (1 - alpha) + v2 * alpha;
}

static int writepgm (const char *path, unsigned char *buffer, int nx, int ny) {
    FILE *file = fopen(path, "w");
    if (!file)
        return 1;

    fprintf(file, "P2\n%d %d\n255\n", nx, ny);
    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i)
            fprintf(file, "%d ", buffer[i + nx * j]);
        fprintf(file, "\n");
    }
    fclose(file);

    return 0;
}

