/*
 * PUC-Rio
 * Análise Numérica 2015.2
 * Professor: Waldemar Celes
 * Gabriel de Quadros Ligneul 1212560
 * Matheus Telles
 * Trabalho Final: Visualização Volumétrica
 */

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NX 256
#define NY 256
#define NZ 99

#define IMG_NX 128
#define IMG_NY 99

#define THRESHOLD 0.3
#define DENSITY 0.05

#define idx(i, j, k) ((k) * NY * NX + (j) * NX + (i))

typedef struct RayData {
    unsigned char *volume;
    int i;
    int k;
    double h;
} RayData;

/* IO */
static unsigned char *readctscan (const char *path);
static int writepgm (const char *path, unsigned char *buffer, int nx, int ny);
static void test_image (unsigned char *volume, unsigned char *bestimage,
                        double h);

/* Calculation */
static unsigned char *visualize (unsigned char *volume, double h);
static double getvalue (unsigned char *volume, int i, int k, double s);
static double ray (unsigned char *volume, int i, int k, double h);
static double volumetric_function (double s, void *data);
static double transfer_function (double s, void *data);
static double composite_simpson (double start, double end, double h,
                                 double (*f)(double x, void *data), void *data);
static double simpson (double a, double b, double (*f)(double x, void *data),
                       void *data);
static double mean_squared_error (unsigned char *a, unsigned char *b);

int main ()
{
    unsigned char *volume = readctscan("head-8bit.raw");
    if (!volume) {
        fputs("Unable to open the .raw file\n", stderr);
        return 1;
    }

    unsigned char *image = visualize(volume, 4.5);
    if (writepgm("out.pgm", image, IMG_NX, IMG_NY)) {
        fputs("Unable to write the output image\n", stderr);
        return 1;
    }
    free(image);

    printf("Mean squared error:\n");
    printf("h\terror\n");
    unsigned char *bestimage = visualize(volume, 1);
    for (int i = 2; i < 20; i += 2)
        test_image(volume, bestimage, i);
    free(bestimage);

    free(volume);
    return 0;
}

static unsigned char *readctscan (const char *path)
{
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

static int writepgm (const char *path, unsigned char *buffer, int nx, int ny)
{
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

static void test_image (unsigned char *volume, unsigned char *bestimage,
                        double h)
{
    clock_t start = clock();
    unsigned char *image = visualize(volume, h);
    clock_t end = clock();
    double time = (end - start) / (double)CLOCKS_PER_SEC;
    printf("%g\t%g\t%g\n", h, mean_squared_error(bestimage, image), time);
    free(image);
}

static unsigned char *visualize (unsigned char *volume, double h)
{
    unsigned char *image = (unsigned char *)malloc(IMG_NX * IMG_NY);

    for (int i = 0; i < IMG_NX; ++i) {
        for (int j = 0; j < IMG_NY; ++j) {
            double ray1 = ray(volume, 2 * i, j, h);
            double ray2 = ray(volume, 2 * i + 1, j, h);
            double value = (ray1 + ray2) / 2;
            int index = i + IMG_NX * j;
            image[index] = (unsigned char)(255 * value);
        }
    }

    return image;
}

static double getvalue (unsigned char *volume, int i, int k, double s)
{
    int s1 = floor(s);
    int s2 = ceil(s);

    if (s2 >= NY)
        return volume[idx(i, NY - 1, k)] / 255.0;

    double v1 = volume[idx(i, s1, k)] / 255.0;
    double v2 = volume[idx(i, s2, k)] / 255.0;
    double intpart;
    double alpha = modf(s, &intpart);
    return v1 * (1 - alpha) + v2 * alpha;
}

static double ray (unsigned char *volume, int i, int k, double h)
{
    RayData data = {volume, i, k, h};
    double value = composite_simpson(0, NY - 1, h, volumetric_function, &data);
    if (value > 1)
        return 1;
    else
        return value;
}

static double volumetric_function (double s, void *data)
{
    RayData *raydata = (RayData *)data;
    return transfer_function(s, data)
           * exp(-composite_simpson(0, s, raydata->h, transfer_function, data));
}

static double transfer_function (double s, void *data)
{
    RayData *ray = (RayData *)data;
    double x = getvalue(ray->volume, ray->i, ray->k, s);
    if (x < THRESHOLD)
        return 0;
    else
        return DENSITY * (x - THRESHOLD);
}

static double composite_simpson (double start, double end, double h,
                                 double (*f)(double x, void *data), void *data)
{
    double a = start;
    double b = h;
    double sum = 0;

    while (b < end) {
        sum += simpson(a, b, f, data);
        a = b;
        b = b + h;
    }
    if (a < end)
        sum += simpson(a, end, f, data);
    return sum;
}

static double simpson (double a, double b, double (*f)(double x, void *data),
                       void *data)
{
    double h = b - a;
    return (h / 6) * (f(a, data) + 4 * f((a + b) / 2, data) + f(b, data));
}

static double mean_squared_error (unsigned char *a, unsigned char *b)
{
    double err = 0;
    for (int i = 0; i < IMG_NX ; ++i) {
        double lineerr = 0;
        for (int j = 0; j < IMG_NY; ++j) {
            double e = a[i + IMG_NX * j] - b[i + IMG_NX * j];
            lineerr += e * e;
        }
        err += lineerr;
    }
    return err / (IMG_NX * IMG_NY);
}

