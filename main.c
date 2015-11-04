/*
 * PUC-Rio
 * Análise Numérica 2015.2
 * Professor: Waldemar Celes
 * Gabriel de Quadros Ligneul 1212560
 * Matheus Telles
 * Trabalho Final: Visualização Volumétrica
 */

#include <stdio.h>
#include <stdlib.h>

#define NX 256
#define NY 256
#define NZ 99

#define IMG_NX 128
#define IMG_NY 99

static unsigned char *readctscan (const char *path);
static unsigned char *visualize (unsigned char *volume);
static int writepgm (const char *path, unsigned char *buffer, int nx, int ny);

int main (int argc, char *argv[]) {
    if (argc < 2) {
        fputs("The .raw file must be sent as argument", stderr);
        return 1;
    }

    unsigned char *volume = readctscan(argv[1]);
    if (!volume) {
        fputs("Unable to open the .raw file", stderr);
        return 1;
    }

    unsigned char *image = visualize(volume);
    free(volume);
    if (writepgm("out.pgm", image, IMG_NX, IMG_NY)) {
        fputs("Unable to write the output image", stderr);
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
            image[i + IMG_NX * j] = volume[i + 30 * NX + j * NY * NX];
        }
    }

    return image;
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

