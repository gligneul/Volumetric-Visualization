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

#define H 4.5
#define THRESHOLD 0.6

#define idx(i, j, k) ((k) * NY * NX + (j) * NX + (i))

typedef struct RayData {
    unsigned char *volume;
    int i;
    int k;
} RayData;

/* IO */
static unsigned char *readctscan (const char *path);
static unsigned char *visualize (unsigned char *volume);
static int writepgm (const char *path, unsigned char *buffer, int nx, int ny);

/* Calculation */
static double getvalue (unsigned char *volume, int i, int k, double s);
static double ray (unsigned char *volume, int i, int k);
static double volumetric_function (double s, void *data);
static double transfer_function (double s, void *data);
static double composite_simpson (double a, double b,
        double (*f)(double x, void *data), void *data);
static double simpson (double a, double b, double (*f)(double x, void *data),
                       void *data);

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
            double ray1 = ray(volume, 2 * i, j);
            double ray2 = ray(volume, 2 * i + 1, j);
            double value = (ray1 + ray2) / 2;
            int index = i + IMG_NX * j;
            printf("Ray: (%d, %d)\n", i, j);
            image[index] = (unsigned char)(255 * value);
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

static double getvalue (unsigned char *volume, int i, int k, double s) {
    int s1 = floor(s);
    int s2 = ceil(s);

    if (s1 > NY || s2 > NY)
        return volume[idx(i, NY - 1, k)] / 255.0;

    double v1 = volume[idx(i, s1, k)] / 255.0;
    double v2 = volume[idx(i, s2, k)] / 255.0;
    double intpart;
    double alpha = modf(s, &intpart);
    return v1 * (1 - alpha) + v2 * alpha;
}

static double ray (unsigned char *volume, int i, int k) {
    RayData data = {volume, i, k};
    double value = composite_simpson(0, NY - 1, volumetric_function, &data);
    if (value > 1)
        return 1;
    else
        return value;
}

static double volumetric_function (double s, void *data) {
    return transfer_function(s, data)
           * exp(-composite_simpson(0, s, transfer_function, data));
}

static double transfer_function (double s, void *data) {
    RayData *ray = (RayData *)data;
    double d = getvalue(ray->volume, ray->i, ray->k, s);
    double threshold = THRESHOLD;
    if (d < threshold)
        return 0;
    else
        return 0.05 * (d - threshold);
}

static double composite_simpson (double start, double end,
        double (*f)(double x, void *data), void *data) {
    double a = start;
    double b = H;
    double sum = 0;

    while (1) {
        if (b > end)
            return sum + simpson(a, end, f, data);
        sum += simpson(a, b, f, data);
        a = b;
        b = b + H;
    }
    return 0;
}

static double simpson (double a, double b, double (*f)(double x, void *data),
                       void *data) {
    double h = b - a;
    return (h / 6) * (f(a, data) + 4 * f((a + b) / 2, data) + f(b, data));
}

