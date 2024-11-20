/* Inspiration - https://github.com/dylanaraps/pywal */
/* Example of using RGB COLORS in terminal - https://chrisyeh96.github.io/2020/03/28/terminal-colors.html */

/* [x]TODO: gen. colors    */
/* [ ]TODO: templating     */
/* [ ]TODO: parsing        */
/* [ ]TODO: config         */
/* [ ]TODO: program usage  */

#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/***
 * STRUCTURES
 ***/
typedef struct
{
    /* With 3 channels it goes from 0 -> size:
     * pixels[0] = Red;
     * pixels[1] = Green;
     * pixels[2] = Blue;
     * pixels[3] = Red;
     * .. And so on
     */
    uint8_t *pixels; 
    size_t size; /* Size is width * height * channels(3) */
} IMG;

typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB;

typedef struct
{
    RGB colors[16];
} PALETTE;

/*** 
 * FUNCTIONS DECLARATIONS
 ***/

 /* prints usage to stdout */
void hellwal_usage()
{
    printf("\nUsage:");
    printf("\t./hellwal ./File_path\n");
}

/* prints error as formatted output and exits with EXIT_FAILURE */
void err(const char *format, ...)
{
    fprintf(stderr, "[ERROR]: ");

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/* Generates PALETTE structure from given image */
PALETTE gen_palette(IMG *img)
{
    PALETTE pal;

    for (unsigned i=1; i<17; i++)
    {
        unsigned j = i;

        uint8_t r = img->pixels[img->size / j]; j++;
        uint8_t g = img->pixels[img->size / j]; j++;
        uint8_t b = img->pixels[img->size / j];

        pal.colors[i-1].R = r;
        pal.colors[i-1].G = g;
        pal.colors[i-1].B = b;
    }
    return pal;
}

/* Prints pallete to stdout */
void pallette_print(PALETTE pal)
{
    for (unsigned i=0; i<16; i++)
    {
        /* Write block of color pallette to stdout */
        printf("\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        printf("\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        if (i == 7) printf("\n");
    }
    printf("\n");
}

/* Write pallette to a file */
void pallette_write(char *filename, PALETTE pal)
{
    FILE *file = fopen(filename, "w");
    for (unsigned i=0; i<16; i++)
    {
        if (i == 7) printf("\n");
        fprintf(file, "color%u=\"%02x%02x%02x\"\n", i, pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
    }
    fclose(file);
}

/* Load image file using stb, return IMG structure */
IMG *img_load(char *filename)
{
    int width, height;
    int numberOfChannels;
    uint8_t *imageData = stbi_load(filename, &width, &height, &numberOfChannels, 0);

    if (imageData == 0) err("Error while loading the file:!\n");

    IMG *img = malloc(sizeof(IMG));

    img->size = width * height * numberOfChannels;
    img->pixels = imageData;

    return img;
}

/* free all allocated stuff in IMG */
void img_free(IMG *img)
{
    stbi_image_free(img->pixels);
    free(img);
}

/* overides default terminal color variables from PALETTE */
void set_term_colors(PALETTE pal)
{
    for (unsigned i=0; i<16; i++)
    {
        printf("\033]4;%d;rgb:%02x/%02x/%02x\033\\", i,
                pal.colors[i].R,
                pal.colors[i].G,
                pal.colors[i].B);
    }
}

/***
 * MAIN
 ***/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        err("You should provide an argument!\n");
        return 1;
    }

    IMG *img = img_load(argv[1]);
    PALETTE pal = gen_palette(img);
    img_free(img);

    pallette_print(pal);
    pallette_write("colors", pal);
    set_term_colors(pal);

    return 0;
}
