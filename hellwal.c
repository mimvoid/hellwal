#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* TODO: add templating */

/* Inspiration - https://github.com/dylanaraps/pywal */
/* Example of using RGB COLORS in terminal - https://chrisyeh96.github.io/2020/03/28/terminal-colors.html */

void hellwal_usage(char* message)
{
    if (message != NULL)
        printf("%s", message);
    printf("Usage:");
    printf("    ./hellwal ./File_path\n");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        hellwal_usage("You should provide an argument!\n");
        return 1;
    }
    char *input_file = argv[1];

    int width, height;
    int numberOfChannels;
    uint8_t *imageData = stbi_load(input_file, &width, &height, &numberOfChannels, 0);
    if (imageData == 0)
    {
        hellwal_usage("File does not exists!\n");
        return 1;
    }

    FILE *file = fopen("colors.pallette", "w");

    printf("\nColorscheme:\n");
    for (size_t i=2; i<18; i++)
    {
        int pos = (width * height * 3) / i;
        unsigned r = imageData[pos], g = imageData[pos+1], b = imageData[pos+2];

        /* Display block of color pallette */
        printf("\x1b[48;2;%d;%d;%dm \033[0m", r, g, b);
        printf("\x1b[48;2;%d;%d;%dm \033[0m", r, g, b);
        if (i == 9) printf("\n");

        /* Write to file */
        fprintf(file, "color%d=%02x%02x%02x\n", (int)i-2, r, g, b);
    }
    printf("\n");
    fclose(file);

    stbi_image_free(imageData);
    return 0;
}
