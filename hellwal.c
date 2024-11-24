/*  hellwal - v0.0.1 - MIT LICENSE
 *
 *  [ ] todo: config                            
 *  [ ] todo: do more pleasant color schemes    
 *  [ ] todo: tweaking options for color palette
 *  --------------------------------------------
 *  [x] todo: print proper program usage        
 *  [x] todo: gen. colors                       
 *  [x] todo: templating                        
 *  [x] todo: parsing                           
 */

#include <glob.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HELL_PARSER_IMPLEMENTATION
#include "hell_parser.h"

/***
 * MACROS
 ***/
#define HELL_COL(p) p->input + p->pos + 1


/***
 * GLOBAL VARIABLES
 ***/

/* these are being set in set_args() */
char HELLWAL_DELIM = '%';
char HELLWAL_DELIM_COUNT = 2;

/* image path, which will be used to create PALETTE */
char *IMAGE_ARG =  NULL;

/* folder, contains templates where colors will be placed */
char *TEMPLATE_FOLDER_ARG =  NULL;

/* 
 * output folder for generated templates,
 * default one is in ~/.cache/hellwal/
 */
char *OUTPUT_ARG =  NULL;

/*
 * you can specify single template input file to which you want to generate;
 * This option does not collide with TEMPLATE_FOLDER_ARG or default path: ~/.config/hellwal/templates
 */
char *TEMPLATE_ARG =  NULL;

/* 
 * specify output of single template you provided,
 * it works only with option above: TEMPLATE_ARG,
 * if not set, default path will be used.
 */
char *OUTPUT_NAME_ARG = NULL;


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
     *
     * Also, we are using uint8_t instead of RGB structure
     * here, only because I dont know how to do it.
     */
    uint8_t *pixels; 
    size_t size; /* Size is width * height * channels(3) */

    unsigned width;
    unsigned height;
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

typedef struct
{
    char *name;
    char *path;
    char *content;
} TEMPLATE;


/*** 
 * FUNCTIONS DECLARATIONS
 ***/
int set_args(int argc, char *argv[]);
char* home_full_path(const char* path);

void check_output_dir(char *path);
void set_term_colors(PALETTE pal);
void err(const char *format, ...);
void warn(const char *format, ...);
void hellwal_usage(const char *name);


IMG *img_load(char *filename);
void img_free(IMG *img);

PALETTE gen_palette(IMG *img);
void palette_print(PALETTE pal);
char *palette_color(PALETTE pal, unsigned c, char *fmt);

void process_templating(PALETTE pal);
char *load_template_file(char *filename);
void template_write(TEMPLATE *t, char *dir);
void process_template(TEMPLATE *t, PALETTE pal);
TEMPLATE **get_templates(const char *dir_path, size_t *_size);


/*** 
 * FUNCTIONS DECLARATIONS
 ***/

 /* prints usage to stdout */
void hellwal_usage(const char *name)
{
    printf("Usage:\n\t%s [OPTIONS]\n", name);
    printf("Options:\n");
    printf("  --image,           -i <image>     Set the image file.\n\n");

    printf("  --template-folder, -f <folder>    Set the template folder.\n");
    printf("  --output,          -o <output>    Set the output folder for generated templates\n\n");

    printf("  --template,        -t <template>  Set the template file.\n");
    printf("  --output-name,     -n <output>    Set the output name for single one, specified generated template\n\n");

    printf("  --help,            -h             Display this help and exit.\n");

    printf("\n\nDetailed: \n");
    printf("  --image: image path, which will be used to create color palette\n\n");

    printf("  --template-folder: folder which contains templates to process\n \
            to generate colors ; default one is ~/.config/hellwal/templates \n\n");

    printf("  --output: output folder where generated templates\n \
              will be saved, default one is set to ~/.cache/hellwal/\n\n");

    printf("  --template: you can specify single template input file which you\n \
              want to generate ; This option does not collide with --template-folder\n\n");

    printf("  --output-name: specify output of single template you provided\n \
            it works only with option above: --template,\n \
            if not set, default path will be used.\n\n");
}

/* set given arguments */
int set_args(int argc, char *argv[])
{
    if (argc < 2)
        return 1;

    int j; /* 'int i' from for loop counter to track incomplete option error */

    for (int i = 1; i < argc; i++)
    {
        j = i;

        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            hellwal_usage(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else if ((strcmp(argv[i], "--template-folder") == 0 || strcmp(argv[i], "-f") == 0))
        {
            if (i + 1 < argc)
                TEMPLATE_FOLDER_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--template") == 0 || strcmp(argv[i], "-t") == 0))
        {
            if (i + 1 < argc)
                TEMPLATE_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--image") == 0 || strcmp(argv[i], "-i") == 0))
        {
            if (i + 1 < argc)
                IMAGE_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--output-folder") == 0 || strcmp(argv[i], "-o") == 0))
        {
            if (i + 1 < argc)
                OUTPUT_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--output-name") == 0 || strcmp(argv[i], "-n") == 0))
        {
            if (i + 1 < argc)
                OUTPUT_NAME_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else
        {
            err("Unknown option: %s\n", argv[i]);
        }
    }

    if (argc == -1) {
        err("Incomplete option: %s\n", argv[j]); }


    if (IMAGE_ARG == NULL) {
        err("You have to provide image path"); }

    if (TEMPLATE_FOLDER_ARG == NULL) {
        TEMPLATE_FOLDER_ARG = home_full_path("~/.config/hellwal/templates"); }
    else
        TEMPLATE_FOLDER_ARG = home_full_path(TEMPLATE_FOLDER_ARG);

    if (OUTPUT_ARG == NULL) {
        OUTPUT_ARG = home_full_path("~/.cache/hellwal/"); }
    else
        OUTPUT_ARG = home_full_path(OUTPUT_ARG);

    if (OUTPUT_NAME_ARG != NULL && TEMPLATE_ARG == NULL) warn("--output-name is not used", OUTPUT_NAME_ARG);
    if (OUTPUT_NAME_ARG == NULL) {
        OUTPUT_NAME_ARG = OUTPUT_ARG; }
    else
        OUTPUT_NAME_ARG = home_full_path(OUTPUT_NAME_ARG);


    return 0;
}

/* prints to stderr formatted output and exits with EXIT_FAILURE */
void err(const char *format, ...)
{
    fprintf(stderr, "\n[ERROR]: ");

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    hellwal_usage("hellwal");

    exit(EXIT_FAILURE);
}

/* prints to stderr formatted output, but not exits */
void warn(const char *format, ...)
{
    fprintf(stderr, "\n[WARNING]: ");

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}

/* get full path from '~/' or other relative paths */
char* home_full_path(const char* path)
{
    if (path[0] == '~')
    {
        const char* home = getenv("HOME");
        if (home)
        {
            char* full_path = malloc(strlen(home) + strlen(path));
            if (full_path)
            {
                sprintf(full_path, "%s%s", home, path + 1);
                return full_path;
            }
        }
    }
    return strdup(path);
}

/* 
 * checks if output directory exists,
 * if not creates it
 */
void check_output_dir(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
        mkdir(path, 0700);
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

/* Writes palete to stdout */
void palette_print(PALETTE pal)
{
    for (unsigned i=0; i<16; i++)
    {
        /* Write color from palete as colored block */
        fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        if (i == 7) printf("\n");
    }
    printf("\n");
}

/* 
 * Write color palette to buffer you HAVE TO specify char *fmt
 * for 3 uint8_t values, otherwise it will crash.
 */
char *palette_color(PALETTE pal, unsigned c, char *fmt)
{
    if (c > 15)
        return NULL;

    char *buffer = (char*)malloc(64);
    sprintf(buffer, fmt, pal.colors[c].R, pal.colors[c].G, pal.colors[c].B);

    return buffer;
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

/* 
 * overides default terminal color variables from PALETTE
 * This works thanks to this (and chatgpt):
 * - https://github.com/dylanaraps/paleta
 * - https://github.com/alacritty/alacritty/issues/656
 * - http://pod.tst.eu/http://cvs.schmorp.de/rxvt-unicode/doc/rxvt.7.pod#XTerm_Operating_System_Commands
 *
 * Manipulate special colors.
 *   10 = foreground,
 *   11 = background,
 *   12 = cursor foregound,
 *   13 = mouse foreground,
 *   708 = terminal border background
 *
 *   \033]{index};{color}\007
 */
void set_term_colors(PALETTE pal) {
    char buffer[1024];
    size_t offset = 0;

    const char *fmt_s = "\033]%d;#%s\033\\";
    const char *fmt_p = "\033]4;%d;#%s\033\\";

    /* Create the sequences */
    for (unsigned i = 0; i < 16; i++) {
        const char *color = palette_color(pal, i, "%02x%02x%02x");
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_p, i, color);
    }
    const char *fg_color = palette_color(pal, 10, "%02x%02x%02x");
    const char *bg_color = palette_color(pal, 11, "%02x%02x%02x");
    const char *cursor_color = palette_color(pal, 12, "%02x%02x%02x");
    const char *border_color = palette_color(pal, 708, "%02x%02x%02x");
    fg_color = fg_color ? fg_color : "FFFFFF";             /* Default to white */
    bg_color = bg_color ? bg_color : "000000";             /* Default to black */
    cursor_color = cursor_color ? cursor_color : "FFFFFF"; /* Default to white */
    border_color = border_color ? border_color : "000000"; /* Default to black */

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_s, 10, fg_color);
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_s, 11, bg_color);
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_s, 12, cursor_color);
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_s, 708, border_color);

    /* Broadcast the sequences to all terminal devices */
    glob_t globbuf;
    if (glob("/dev/pts/*", GLOB_NOSORT, NULL, &globbuf) == 0) {
        for (size_t i = 0; i < globbuf.gl_pathc; i++) {
            FILE *f = fopen(globbuf.gl_pathv[i], "w");
            if (f) {
                fwrite(buffer, 1, offset, f);
                fclose(f);
            }
        }
        globfree(&globbuf);
    }

    /* Also write to the current terminal */
    fwrite(buffer, 1, offset, stdout);
}

/* Loads template text file, returns buffer if succeded, otherwise NULL */
char *load_template_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
    warn("Failed to open file: %s", filename); return NULL; }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);

    char *buffer = (char*)malloc(size + 1); // +1 for null terminator
    if (buffer == NULL)
    {
        warn("Failed to allocate memory for file buffer");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);

    TEMPLATE *template = malloc(sizeof(TEMPLATE));
    template->path = filename;
    template->content = buffer;

    return buffer;
}

/* 
 * reads content of all given and found templates paths,
 * and writes to specified or default output folder.
 */
void process_templating(PALETTE pal)
{
    TEMPLATE **templates;
    size_t templates_count;

    /* Process templates loaded from folder */
    templates = get_templates(TEMPLATE_FOLDER_ARG, &templates_count);
    if (templates == NULL) return;

    for (size_t i = 0; i < templates_count; i++)
        process_template(templates[i], pal);

    if (OUTPUT_NAME_ARG != NULL) {
        check_output_dir(OUTPUT_NAME_ARG);
        template_write(templates[templates_count-1], OUTPUT_NAME_ARG);
        templates_count--;
    }

    check_output_dir(OUTPUT_ARG);
    for (size_t i = 0; i < templates_count; i++)
        template_write(templates[i], OUTPUT_ARG);
}

/* load t->path file to buffer and replaces content between delim with colors from PALETTE colors */
void process_template(TEMPLATE *t, PALETTE pal)
{
    char *template_buffer = calloc(1, 1);
    size_t template_size = 0;
    int last_pos = 0;
    int buffrd_pos = 0;
    
    hell_parser_t *p = hell_parser_create(load_template_file(t->path));
    if (p == NULL)
    {
        t->content = NULL;
        return;
    }

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
                                const char *color = palette_color(pal, atoi(number), "%02x%02x%02x");
                                size_t color_len = strlen(color);

                                template_size += color_len + 1;
                                template_buffer = realloc(template_buffer, template_size);
                                if (template_buffer == NULL) return;

                                strcat(template_buffer, color);
                            }
                            if (!strcmp(HELL_COL(pn), "rgb"))
                            {
                                const char *color = palette_color(pal, atoi(number), "rgb(%d, %d, %d)");
                                size_t color_len = strlen(color);

                                template_size += color_len + 1;
                                template_buffer = realloc(template_buffer, template_size);
                                if (template_buffer == NULL) return;

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
        if (template_buffer == NULL) 
            return;

        strncat(template_buffer, p->input + buffrd_pos, size);
    }

    hell_parser_destroy(p);

    t->content = template_buffer;
}

/* load templates from dir */
TEMPLATE **get_templates(const char *dir_path, size_t *_size)
{
    if (dir_path == NULL) return NULL;

    TEMPLATE **t_arr = NULL;
    size_t size = 0;

    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        warn("Cannot access templates directory: %s", dir_path);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            size_t path_len = strlen(dir_path) + strlen(entry->d_name) + 2; // +2 for '/' and '\0'
            char *full_path = malloc(path_len);
            if (full_path == NULL)
            {
                perror("Failed to allocate memory for file path");
                closedir(dir);
                return NULL;
            }
            snprintf(full_path, path_len, "%s/%s", dir_path, entry->d_name);

            TEMPLATE **new_t_arr = realloc(t_arr, (size + 1) * sizeof(TEMPLATE*));
            if (new_t_arr == NULL)
            {
                perror("Failed to allocate memory for templates array");
                free(full_path);
                closedir(dir);
                free(t_arr);
                return NULL;
            }
            t_arr = new_t_arr;

            t_arr[size] = calloc(sizeof(TEMPLATE), 1);
            t_arr[size]->path = full_path;
            t_arr[size]->name = strdup(entry->d_name);

            size++;
        }
    }

    closedir(dir);

    /* single template provided from arguments */
    if (TEMPLATE_ARG != NULL)
    {
        TEMPLATE **new_t_arr = realloc(t_arr, (size + 1) * sizeof(TEMPLATE *));
        if (new_t_arr == NULL) {
            perror("Failed to allocate memory for templates array");
            free(t_arr);
            *_size = -1;
            TEMPLATE_ARG = NULL;
            return NULL;
        }

        t_arr = new_t_arr;
        t_arr[size] = calloc(1, sizeof(TEMPLATE));
        if (t_arr[size] == NULL) {
            perror("Failed to allocate memory for new template");
            *_size = -1;
            return NULL;
        }

        char *temp = strdup(TEMPLATE_ARG);
        char *name = strrchr(temp, '/');
        if (name)
            name++; // Move past the '/'
        else
            name = temp; // Use the full string if no '/' is found

        t_arr[size]->name = name;
        t_arr[size]->path = TEMPLATE_ARG;

        size++;
    }

    if (_size)
        *_size = size;

    return t_arr;
}

/* write generated template to dir */
void template_write(TEMPLATE *t, char *dir)
{
    if (t == NULL || dir == NULL) return;
    if (t->content == NULL) return;

    char* path = malloc(strlen(dir) + strlen(t->name));
    if (path)
        sprintf(path, "%s%s", dir, t->name);
    else
        return;

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        perror(NULL);
        warn("Cannot write to file: %s\n", t->path);
        return;
    }

    fprintf(f, "%s", t->content);

    fclose(f);
}

/***
 * MAIN
 ***/
int main(int argc, char **argv)
{
    if (set_args(argc,argv) != 0)
    {
        hellwal_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    IMG *img = img_load(IMAGE_ARG);
    PALETTE pal = gen_palette(img);
    img_free(img);

    palette_print(pal);
    set_term_colors(pal);

    process_templating(pal);
}
