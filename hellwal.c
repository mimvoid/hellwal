/* Inspiration - https://github.com/dylanaraps/pywal */
/* Example of using RGB COLORS in terminal - https://chrisyeh96.github.io/2020/03/28/terminal-colors.html */

/* [ ]TODO: config                      */
/* [ ]TODO: print proper program usage  */
/* [x]TODO: gen. colors                 */
/* [x]TODO: templating                  */
/* [x]TODO: parsing                     */

#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HELL_PARSER_IMPLEMENTATION
#include "hell_parser.h"

#define HELL_COL(p) p->input + p->pos + 1

char HELLWAL_DELIM = '%';
char HELLWAL_DELIM_COUNT = 2;


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
void hellwal_usage();
void err(const char *format, ...);

void set_term_colors(PALETTE pal);

PALETTE gen_palette(IMG *img);
void pallette_print(PALETTE pal);
void pallette_write(char *filename, PALETTE pal);
char *pallette_color(PALETTE pal, unsigned c, char *fmt);

IMG *img_load(char *filename);
void img_free(IMG *img);

char *load_text_file(char *filename);
char *process_template(char *template_name, PALETTE pal);


/*** 
 * FUNCTIONS DECLARATIONS
 ***/

 /* prints usage to stdout */
void hellwal_usage()
{
    printf("\nUsage:");
    printf("\t./hellwal [image_path] [template]\n");
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

/* Write color pallette to buffer */
char *pallette_color(PALETTE pal, unsigned c, char *fmt)
{
    if (c > 15)
        return NULL;

    char *buffer = (char*)malloc(64);
    sprintf(buffer, fmt, pal.colors[c].R, pal.colors[c].G, pal.colors[c].B);

    return buffer;
}

/* Write pallette to file */
void pallette_write(char *filename, PALETTE pal)
{
    FILE *file = fopen(filename, "w");

    if (file == NULL) err("Failed to open file");

    for (unsigned i=0; i<16; i++)
    {
        if (i == 7) printf("\n");
        fprintf(file, "color%u=%s\n", i, pallette_color(pal, i, "%02x%02x%02x"));
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

/* Loads text file, returns buffer if succeded, if not returns NULL */
char *load_text_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) err("Failed to open file: %s", filename);

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc(size + 1); // +1 for null terminator
    if (buffer == NULL)
    {
        printf("Failed to allocate memory for file buffer");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);

    return buffer;
}

/* 
 * loads template_name file content
 * and replaces content between delim with colors,
 * returns buffer of entire file
 */
char *process_template(char *template_name, PALETTE pal)
{
    char *template_buffer = NULL;
    size_t template_size = 0;
    int last_pos = 0;
    int buffrd_pos = 0;
    
    hell_parser_t *p = hell_parser_create(load_text_file(template_name));
    template_buffer = calloc(1, 1);
    
    while (!hell_parser_eof(p))
    {
        char ch;
        if (hell_parser_next(p, &ch) == HELL_PARSER_OK)
        {
            if (ch == HELLWAL_DELIM)
            {
                p->pos -= 1;  
                char *delim_buf = NULL;

                last_pos = p->pos + 1;

                if (hell_parser_delim(p, HELLWAL_DELIM, HELLWAL_DELIM_COUNT, &delim_buf) == HELL_PARSER_OK)
                {
                    assert(p->pos - last_pos > 0);
                    
                    int size_before_delim = last_pos - buffrd_pos - 1;
                    if (size_before_delim > 0)
                    {
                        template_size += size_before_delim + 1;
                        template_buffer = realloc(template_buffer, template_size);
                        strncat(template_buffer, p->input + buffrd_pos, size_before_delim);
                    }

                    hell_parser_t *pn = hell_parser_create(delim_buf);
                    char *number = NULL;

                    if (hell_parser_delim(pn, '|', 1, &number) == HELL_PARSER_OK)
                    {
                        assert(number != NULL);

                        if (pn->pos + 1 < pn->length)
                        {
                            if (!strcmp(HELL_COL(pn), "hex"))
                            {
                                const char *color = pallette_color(pal, atoi(number), "%02x%02x%02x");
                                size_t color_len = strlen(color);

                                template_size += color_len + 1;
                                template_buffer = realloc(template_buffer, template_size);
                                strcat(template_buffer, color);
                            }
                            if (!strcmp(HELL_COL(pn), "rgb"))
                            {
                                const char *color = pallette_color(pal, atoi(number), "rgb(%d, %d, %d)");
                                size_t color_len = strlen(color);

                                template_size += color_len + 1;
                                template_buffer = realloc(template_buffer, template_size);
                                strcat(template_buffer, color);
                            }
                        }
                    }
                    /* Update last read buffer position */
                    buffrd_pos = p->pos;

                    free(delim_buf);
                    free(number);
                    hell_parser_destroy(pn);
                }
            }
        }
    }

    /* 
     * If content of template does not end on delim,
     * add rest of the content
     */
    if ((size_t)buffrd_pos < p->length)
    {
        size_t size = p->length - buffrd_pos;
        template_size += size + 1;
        template_buffer = realloc(template_buffer, template_size);
        strncat(template_buffer, p->input + buffrd_pos, size);
    }

    hell_parser_destroy(p);

    printf("%s:\n%s\n", template_name, template_buffer);
    return template_buffer;
}

/***
 * MAIN
 ***/
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        hellwal_usage();
        err("You should provide an arguments! \n");
        return 1;
    }

    IMG *img = img_load(argv[1]);
    PALETTE pal = gen_palette(img);
    img_free(img);

    pallette_print(pal);
    set_term_colors(pal);

    // TODO: read templates from config templates folder and cmdline args
    process_template(argv[2], pal);
}
