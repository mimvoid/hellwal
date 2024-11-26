/*  hellwal - v0.0.3 - MIT LICENSE
 *
 *  [ ] TODO: config ( is it really needed? )                               
 *  [ ] TODO: support for other OS's like Mac or Win                        
 *  ------------------------------------------------------------------------
 *  [x] TODO: support for already built themes (like gruvbox etc.)          
 *  [x] TODO: tweaking options for generated colors (func + dark-light mode 
 *  [x] TODO: do more pleasant color schemes                                
 *  [x] TODO: print proper program usage                                    
 *  [x] TODO: gen. colors                                                   
 *  [x] TODO: templating                                                    
 *  [x] TODO: parsing                                                       
 *
 *  changelog v0.0.3:
 *   - removed TEMPLATE_ARG, OUTPUT_NAME_ARG - I've come to conclusion that it's useless.
 *   - changed how 'variables' are parsed. Now index of colors ex. "color|0|.hex" is depracated. Use "color0.hex" instead.
 *   - added support for static themes, for example you can add gruvbox, catppuccin etc.
 *   - added THEME_ARG, THEME_FOLDER_ARG
 *   - added keyword "wallpaper" for wallpaper path.
 *   - added example themes in ./themes folder
 *
 *  changelog v0.0.2:
 *   - actually working and better generated colorschemes
 *   - silent output: -q or --quiet
 *   - variable |w|, that recognized in templates returns IMAGE_ARG path
 *   - support for both light/dark mode
 *   - added more example templates
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

/* it tells bounds of valid rgb values */
#define MIN_BRIGHTNESS 50
#define MAX_BRIGHTNESS 200

/* just palette size */
#define PALETTE_SIZE 16

/* set default value for global char* variables */
#define SET_DEF(x, s) \
    if (x == NULL) \
        x = home_full_path(s); \
    else \
        x = home_full_path(x);


/***
 * STRUCTURES
 ***/

/* IMG
 * 
 * image structure that contains all image data,
 * we need to create color palette
 */
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

/* RGB,
 * what can I say huh */
typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB;

/* PALETTE
 *
 * stores all RGB colors */
typedef struct
{
    RGB colors[PALETTE_SIZE];
} PALETTE;

/* TEMPLATE
 *
 * helps read and write and store processed buffer*/
typedef struct
{
    char *name;
    char *path;
    char *content;
} TEMPLATE;


/***
 * GLOBAL VARIABLES
 ***/

/* these are being set in set_args() */
char HELLWAL_DELIM = '%';
char HELLWAL_DELIM_COUNT = 2;

/* image path, which will be used to create PALETTE */
char *IMAGE_ARG = NULL;

/* quiet arg, if NULL you will get verbose output,
 * otherwise its going to print everthing normally 
 */
char *QUIET_ARG = NULL;

/* darkmode or lightmode, darkmode is default */
char *DARK_ARG  = NULL;
char *LIGHT_ARG = NULL;

/* folder that contains templates */
char *TEMPLATE_FOLDER_ARG = NULL;

/* 
 * output folder for generated templates,
 * default one is in ~/.cache/hellwal/
 */
char *OUTPUT_ARG = NULL;

/* name of theme in THEME_FOLDER_ARG,
 * or (in case was not found) path to file */
char *THEME_ARG = NULL;

/* folder that contains static themes */
char *THEME_FOLDER_ARG  = NULL;

/* one palette as global variable, so log_c() can access colors */
size_t pal_log_iter = 1;
PALETTE pal_log = {
    .colors[1].R = 235,
    .colors[1].G = 255,
    .colors[1].B = 235
 };
PALETTE pal_log_err = {
    .colors[1].R = 235,
    .colors[1].G = 20,
    .colors[1].B = 5
 };
PALETTE pal_log_warn = {
    .colors[1].R = 235,
    .colors[1].G = 255,
    .colors[1].B = 10
 };


/*** 
 * FUNCTIONS DECLARATIONS
 ***/

/* args */
int set_args(int argc, char *argv[]);

/* utils */
void check_output_dir(char *path);
void hellwal_usage(const char *name);
char* home_full_path(const char* path);

/* logging */
void eu(const char *format, ...);
void err(const char *format, ...);
void warn(const char *format, ...);
void log_c(const char *format, ...);

/* IMG */
IMG *img_load(char *filename);
void img_free(IMG *img);

/* RGB */
float calculate_color_distance(RGB a, RGB b);

int is_color_palette_var(char *name);
int is_valid_luminance(float luminance);
int is_hex_to_rgb(const char *hex, RGB *p);
int is_color_too_similar(RGB *palette, int num_colors, RGB new_color);

RGB darken_color(RGB color, float factor);
RGB lighten_color(RGB color, float factor);
RGB saturate_color(RGB color, float factor);

/* palettes */
PALETTE gen_palette(IMG *img);
float calculate_luminance(RGB c);
int compare_luminance(const void *a, const void *b);
char *palette_color(PALETTE pal, unsigned c, char *fmt);
void palette_print(PALETTE pal);
void set_term_colors(PALETTE pal);
void reverse_palette(PALETTE *palette);
void sort_palette_by_luminance(PALETTE *palette);

/* templates */
char *load_file(char *filename);
void process_templating(PALETTE pal);
size_t template_write(TEMPLATE *t, char *dir);
void process_template(TEMPLATE *t, PALETTE pal);
TEMPLATE **get_template_structure_dir(const char *dir_path, size_t *_size);

/* themes */
char *load_theme(char *themename);
PALETTE process_themeing(char *theme);
int process_theme(char *t, PALETTE *pal);

/*** 
 * FUNCTIONS DECLARATIONS
 ***/

 /* prints usage to stdout */
void hellwal_usage(const char *name)
{
    printf("Usage:\n\t%s [OPTIONS]\n", name);
    printf("Options:\n");
    printf("  --image,           -i <image>     Set image file.\n\n");
    printf("  --dark,            -d             Set dark mode (ON by default)\n");
    printf("  --light,           -l             Set light mode\n\n");

    printf("  --quiet,           -q             Set silent output\n\n");

    printf("  --template-folder, -f <folder>    Set template folder.\n");
    printf("  --output,          -o <folder>    Set output folder for generated templates\n\n");

    printf("  --theme,           -t <file>      Set theme from THEME FOLDER or (in case was not found) path to theme file\n");
    printf("  --theme-folder,    -k <folder>    Set folder that contains themes\n\n");

    printf("  --help,            -h             Display this help and exit.\n");

    printf("\n\nDetailed: \n");
    printf("  --image: image path, which will be used to create color palette\n\n");
    printf("  --dark: colors go brrrr (#000000)\n\n");
    printf("  --light: colors go ffffff (#ffffff)\n\n");
    printf("  --quiet: \n\n"); /* XD */

    printf("  --template-folder: folder which contains templates to process to generate colors ; default one is ~/.config/hellwal/templates \n\n");
    printf("  --output: output folder where generated templates will be saved, default one is set to ~/.cache/hellwal/\n\n");

    printf("  --theme: name of theme in theme folder OR (in case was not found) path to theme file\n\n");
    printf("  --theme-folder: folder that contains themes, default one is set to ~/.config/hellwal/themes/\n");
}

/* set given arguments */
int set_args(int argc, char *argv[])
{
    int j = 0; /* 'int i' from for loop counter to track incomplete option error */

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
        else if ((strcmp(argv[i], "--image") == 0 || strcmp(argv[i], "-i") == 0))
        {
            if (i + 1 < argc)
                IMAGE_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0))
        {
            /* anything other than NULL, makes quiet/silent output */
            QUIET_ARG = "";
        }
        else if ((strcmp(argv[i], "--dark") == 0 || strcmp(argv[i], "-d") == 0))
        {
            /* anything other than NULL, makes dark mode */
            DARK_ARG = "";
        }
        else if ((strcmp(argv[i], "--light") == 0 || strcmp(argv[i], "-l") == 0))
        {
            /* anything other than NULL, makes light mode */
            LIGHT_ARG = "";
        }
        else if ((strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0))
        {
            if (i + 1 < argc)
                OUTPUT_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--theme") == 0 || strcmp(argv[i], "-t") == 0))
        {
            if (i + 1 < argc)
                THEME_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--theme-folder") == 0 || strcmp(argv[i], "-k") == 0))
        {
            if (i + 1 < argc)
                THEME_FOLDER_ARG = argv[++i];
            else
            {
                argc = -1;
            }
        }
        else
        {
            eu("Unknown option: %s", argv[i]);
        }
    }

    if (argc == -1)
        eu("Incomplete option: %s", argv[j]);

    /* handle needed arguments */
    if (IMAGE_ARG == NULL && THEME_ARG == NULL && THEME_FOLDER_ARG == NULL)
        err("You have to provide --image, or --theme");

    if (IMAGE_ARG == NULL && THEME_ARG == NULL && THEME_FOLDER_ARG != NULL)
        err("You have to provide theme name or path");

    if (THEME_ARG  == NULL && THEME_FOLDER_ARG != NULL)
        warn("specified theme folder is not used: \"%s\"", THEME_FOLDER_ARG);

    if (THEME_ARG  != NULL && IMAGE_ARG != NULL)
        err("you cannot use both --image and --theme:");

    /* If not specified, set default ones */
    SET_DEF(OUTPUT_ARG, "~/.cache/hellwal/");
    SET_DEF(THEME_FOLDER_ARG , "~/.config/hellwal/themes");
    SET_DEF(TEMPLATE_FOLDER_ARG, "~/.config/hellwal/templates");

    return 0;
}

/* 
 * prints to stderr formatted output and exits with EXIT_FAILURE,
 * also prints hellwal_usage()
 */
void eu(const char *format, ...)
{
    fprintf(stderr, "[ERROR]: ");

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\n");

    hellwal_usage("hellwal");

    exit(EXIT_FAILURE);
}

/* prints to stderr formatted output and exits with EXIT_FAILURE */
void err(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    fprintf(stderr, "\033[38;2;%d;%d;%dm[ERROR]: ",
            pal_log_err.colors[1].R,
            pal_log_err.colors[1].G,
            pal_log_err.colors[1].B);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\033[0m");

    va_end(ap);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}

/* prints to stderr formatted output, but not exits */
void warn(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "\033[38;2;%d;%d;%dm[WARN]: ",
            pal_log_warn.colors[1].R,
            pal_log_warn.colors[1].G,
            pal_log_warn.colors[1].B);

    vfprintf(stderr, format, ap);
    fprintf(stderr, "\033[0m");
    va_end(ap);
    fprintf(stderr, "\n");
}

/* prints formatted output to stdout with colors */
void log_c(const char *format, ...)
{
    if (QUIET_ARG != NULL)
        return;

    va_list ap;
    va_start(ap, format);

    fprintf(stdout, "\033[38;2;%d;%d;%dm[INFO]: ",
            pal_log.colors[pal_log_iter].R,
            pal_log.colors[pal_log_iter].G,
            pal_log.colors[pal_log_iter].B);

    vfprintf(stdout, format, ap);
    fprintf(stdout, "\033[0m");

    va_end(ap);
    fprintf(stdout, "\n");

    if (pal_log_iter != 1)
        pal_log_iter+=4;
    if (pal_log_iter > PALETTE_SIZE)
        pal_log_iter = 2;
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

void remove_whitespaces(char *str)
{
    if (!str) return;

    char *read = str;
    char *write = str;

    while (*read) {
        if (*read != ' ' && *read != '\t' && *read != '\n' && *read != '\r') {
            *write++ = *read;
        }
        read++;
    }
    *write = '\0';
}

/* calculate how bright is color */
float calculate_luminance(RGB c) {
    return (0.2126 * c.R + 0.7152 * c.G + 0.0722 * c.B); 
}

/* check if luminance is within a valid range  */
int is_valid_luminance(float luminance) {
    return luminance >= MIN_BRIGHTNESS && luminance <= MAX_BRIGHTNESS;
}

/* check Euclidean distance between two colors to ensure diversity */
float calculate_color_distance(RGB a, RGB b) {
    return sqrtf(powf(a.R - b.R, 2) + powf(a.G - b.G, 2) + powf(a.B - b.B, 2));
}

/* ensure that new color is not too similar to existing colors in the palette */
int is_color_too_similar(RGB *palette, int num_colors, RGB new_color) {
    for (int i = 0; i < num_colors; i++) {
        if (calculate_color_distance(palette[i], new_color) < 30)
            return 1;  // Color is too similar
    }
    return 0;
}

/* sort palette by luminance to spread out colors */
void sort_palette_by_luminance(PALETTE *palette) {
    qsort(palette->colors, PALETTE_SIZE, sizeof(RGB), compare_luminance);
}

/* compare two RGB colors */
int compare_luminance(const void *a, const void *b) {
    RGB *color_a = (RGB *)a;
    RGB *color_b = (RGB *)b;

    float lum_a = calculate_luminance(*color_a);
    float lum_b = calculate_luminance(*color_b);

    if (lum_a < lum_b)
        return -1;
    else if (lum_a > lum_b)
        return 1;
    else
        return 0;
}

/* darken a color */
RGB darken_color(RGB color, float factor) {
    color.R = (uint8_t)(color.R * factor);
    color.G = (uint8_t)(color.G * factor);
    color.B = (uint8_t)(color.B * factor);
    return color;
}

/* lighten a color */
RGB lighten_color(RGB color, float factor) {
    color.R = (uint8_t)(255 - ((255 - color.R) * factor));
    color.G = (uint8_t)(255 - ((255 - color.G) * factor));
    color.B = (uint8_t)(255 - ((255 - color.B) * factor));
    return color;
}

/* saturate a color */
RGB saturate_color(RGB color, float factor) {
    float max_val = fmaxf(color.R, fmaxf(color.G, color.B));
    color.R = (uint8_t)(color.R + (max_val - color.R) * factor);
    color.G = (uint8_t)(color.G + (max_val - color.G) * factor);
    color.B = (uint8_t)(color.B + (max_val - color.B) * factor);
    return color;
}


/* function to reverse the palette, used when light mode is specified */
void reverse_palette(PALETTE *palette)
{
    for (int i = 0; i < PALETTE_SIZE / 2; i++)
    {
        RGB temp = palette->colors[i];

        palette->colors[i] = palette->colors[PALETTE_SIZE - 1 - i];
        palette->colors[PALETTE_SIZE - 1 - i] = temp;
    }
}

/* generate palette from given image */
PALETTE gen_palette(IMG *img)
{
    log_c("Generating color palette...");

    PALETTE p;
    int num_colors = 0;
    int step = 3;

    for (size_t i = 0; i < img->size; i += step) {
        RGB new_color = {img->pixels[i], img->pixels[i + 1], img->pixels[i + 2]};

        float luminance = calculate_luminance(new_color);
        if (!is_valid_luminance(luminance))
            continue;

        if (!is_color_too_similar(p.colors, num_colors, new_color) && num_colors < PALETTE_SIZE) {
            p.colors[num_colors++] = new_color; }

        if (num_colors >= PALETTE_SIZE) {
            break;
        }
    }

    /* if not enough colors found, keep adding random colors from the image */
    while (num_colors < PALETTE_SIZE) {
        RGB new_color = {img->pixels[(rand() % (img->size / 3)) * 3],
                         img->pixels[(rand() % (img->size / 3)) * 3 + 1],
                         img->pixels[(rand() % (img->size / 3)) * 3 + 2]};
        p.colors[num_colors++] = new_color;
    }

    /* Adjust saturation for all colors except:
     * [0]  - background
     * [7]  - text
     * [8]  - I don't know
     * [15] - foreground
     */
    for (int i = 0; i < PALETTE_SIZE; i++) {
        if (i != 0 && i != 7 && i != 8 && i != 15) {
            p.colors[i] = saturate_color(p.colors[i], 0.60); /* TODO: load from config, args?? idk */
        }
    }

    sort_palette_by_luminance(&p);

    p.colors[0] = darken_color(p.colors[0], 0.10);
    p.colors[8] = darken_color(p.colors[8], 0.25);
    p.colors[7] = lighten_color(p.colors[7], 0.45);
    p.colors[15] = lighten_color(p.colors[15], 0.35);

    /* set output color palette for logs */
    pal_log = p;
    pal_log_iter = 2;

    log_c("...Generated!");
    return p;
}

/* Writes palete to stdout */
void palette_print(PALETTE pal)
{
    if (QUIET_ARG != NULL)
        return;

    for (unsigned i=0; i<PALETTE_SIZE; i++)
    {
        /* Write color from palete as colored block */
        fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", pal.colors[i].R, pal.colors[i].G, pal.colors[i].B);
        if (i+1 == PALETTE_SIZE/2) printf("\n");
    }
    printf("\n");
}

/* 
 * Write color palette to buffer you HAVE TO specify char *fmt
 * for 3 uint8_t values, otherwise it will crash.
 */
char *palette_color(PALETTE pal, unsigned c, char *fmt)
{
    if (c > PALETTE_SIZE - 1)
        return NULL;

    char *buffer = (char*)malloc(64);
    sprintf(buffer, fmt, pal.colors[c].R, pal.colors[c].G, pal.colors[c].B);

    return buffer;
}

/* Load image file using stb, return IMG structure */
IMG *img_load(char *filename)
{
    if (IMAGE_ARG == NULL)
        err("No image provided");
    log_c("Loading image %s", IMAGE_ARG);

    int width, height;
    int numberOfChannels;
    uint8_t *imageData = stbi_load(filename, &width, &height, &numberOfChannels, 0);

    if (imageData == 0) err("Error while loading the file: %s", filename);

    IMG *img = malloc(sizeof(IMG));

    img->size = width * height * numberOfChannels;
    img->pixels = imageData;

    log_c("Loaded!");
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
void set_term_colors(PALETTE pal)
{
    log_c("Setting terminals...");

    char buffer[1024];
    size_t offset = 0;

    const char *fmt_s = "\033]%d;#%s\033\\";
    const char *fmt_p = "\033]4;%d;#%s\033\\";

    /* Create the sequences */
    for (unsigned i = 0; i < PALETTE_SIZE; i++) {
        const char *color = palette_color(pal, i, "%02x%02x%02x");
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, fmt_p, i, color);
    }
    const char *bg_color     = palette_color(pal, 0,  "%02x%02x%02x");
    const char *fg_color     = palette_color(pal, 15, "%02x%02x%02x");
    const char *cursor_color = palette_color(pal, 15, "%02x%02x%02x");
    const char *border_color = palette_color(pal, 15, "%02x%02x%02x");

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

    log_c("...All set!\n");
}

/* 
 * Loads content from file file to buffer,
 * returns buffer if succeded, otherwise NULL
 */
char *load_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        warn("Failed to open file: %s", filename);
        return NULL;
    }

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

    return buffer;
}

/* 
 * reads content of all given and found templates paths,
 * and writes to specified or default output folder.
 */
void process_templating(PALETTE pal)
{
    log_c("Processing templates: ");

    TEMPLATE **templates;
    size_t templates_count, t_success = 0;

    /* Process templates loaded from folder */
    templates = get_template_structure_dir(TEMPLATE_FOLDER_ARG, &templates_count);
    if (templates == NULL) return;

    check_output_dir(OUTPUT_ARG);
    for (size_t i = 0; i < templates_count; i++) {
        process_template(templates[i], pal);
        t_success += template_write(templates[i], OUTPUT_ARG);
    }

    log_c("Sucessfully processed %d templates!", t_success);
}

/* load t->path file to buffer and replaces content between delim with colors from PALETTE colors */
void process_template(TEMPLATE *t, PALETTE pal)
{
    if (t == NULL) {
        t->content = NULL;
        return;
    }
    log_c("  - generating template buffer: %s", t->name);

    char *template_buffer = calloc(1, 1);
    size_t template_size = 0;
    int last_pos = 0;
    int buffrd_pos = 0;
    
    hell_parser_t *p = hell_parser_create(load_file(t->path));
    if (p == NULL) {
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
                int idx = 0;
                size_t len = 0;
                char *var_arg = NULL;
                char *delim_buf = NULL;

                last_pos = p->pos + 1;

                int size_before_delim = last_pos - buffrd_pos - 1;
                if (size_before_delim > 0)
                {
                    template_size += size_before_delim + 1;
                    template_buffer = realloc(template_buffer, template_size);
                    strncat(template_buffer, p->input + buffrd_pos, size_before_delim);
                }

                if (hell_parser_delim_buffer_between(p, HELLWAL_DELIM, HELLWAL_DELIM_COUNT, &delim_buf) == HELL_PARSER_OK)
                {
                    hell_parser_t *pd = hell_parser_create(delim_buf);
                    if (pd == NULL)
                        err("Failed to allocate parser");

                    if (hell_parser_delim(pd, '.', 1) == HELL_PARSER_OK) {
                        size_t l_size = pd->pos - 1;
                        size_t r_size = pd->length - pd->pos + 1;

                        char *left  = calloc(1, l_size);
                        char *right = calloc(1, r_size);

                        strncpy(left, pd->input, l_size);
                        strncpy(right, pd->input + pd->pos + 1, r_size);

                        remove_whitespaces(left);
                        remove_whitespaces(right);

                        idx = is_color_palette_var(left);
                        if (idx != -1 && pd->pos + 1 < pd->length)
                        {
                            /* 
                             * check if after '.' is rgb, if yes get output as rgb,
                             * if not output will always be hex
                             */
                            if (!strcmp(right, "rgb"))
                                var_arg = palette_color(pal, idx, "%d, %d, %d");
                            else
                                var_arg = palette_color(pal, idx, "%02x%02x%02x");
                        }
                        free(left);
                        free(right);
                    }
                    /* check if an argument stands for wallpaper path */
                    else if (!strcmp(delim_buf, "wallpaper"))
                    {
                        if (IMAGE_ARG != NULL)
                        {
                            len = strlen(IMAGE_ARG);
                            var_arg = IMAGE_ARG;
                        }
                        else
                            var_arg = "";
                    }
                    //* TODO: keywords not working except "wallpaper" */
                    //* TODO: add rgb as an option for keywords */
                    else if (!strcmp(delim_buf, "foregound"))
                        var_arg = palette_color(pal, 15, "%02x%02x%02x");
                    else if (!strcmp(delim_buf, "background"))
                        var_arg = palette_color(pal, 0, "%02x%02x%02x");
                    else if (!strcmp(delim_buf, "cursor"))
                        var_arg = palette_color(pal, 15, "%02x%02x%02x");

                    if (var_arg != NULL) {
                        len = strlen(var_arg);
                        template_size += len + 1;
                        template_buffer = realloc(template_buffer, template_size);
                        if (template_buffer == NULL) return;
                        strcat(template_buffer, var_arg);
                    }
                     
                    /* Update last read buffer position */
                    buffrd_pos = p->pos;

                    free(delim_buf);
                    hell_parser_destroy(pd);
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

/* return array of TEMPLATE structure of files in directory */
TEMPLATE **get_template_structure_dir(const char *dir_path, size_t *_size)
{
    if (dir_path == NULL) return NULL;

    TEMPLATE **t_arr = NULL;
    size_t size = 0;

    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        warn("Cannot access directory: %s", dir_path);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
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

    if (_size)
        *_size = size;

    return t_arr;
}

/* 
 * write generated template to dir,
 * returns 1 on success
 */
size_t template_write(TEMPLATE *t, char *dir) {
    if (t == NULL || dir == NULL) return 0;
    if (t->content == NULL) return 0;

    char* path = malloc(strlen(dir) + strlen(t->name));
    if (path)
        sprintf(path, "%s%s", dir, t->name);
    else
        return 0;

    FILE *f = fopen(path, "w");
    if (f == NULL) {
        perror(NULL);
        warn("Cannot write to file: %s", t->path);
        return 0;
    }

    fprintf(f, "%s", t->content);
    log_c("  - wrote template to: %s\n", t->path);

    fclose(f);
    return 1;
}

/*
 * check in directory for theme with provided name,
 * if not exist, try to open it as a path */
char *load_theme(char *themename)
{
    log_c("Loading static theme: %s", THEME_ARG);
    DIR *dir = opendir(THEME_FOLDER_ARG);
    if (dir == NULL)
        err("Cannot access directory: %s", THEME_FOLDER_ARG);

    char *t = NULL;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, themename) == 0)
        {
            size_t path_len = strlen(THEME_FOLDER_ARG) + strlen(entry->d_name) + 2; // +2 for '/' and '\0'
            char *path = malloc(path_len);
            if (path == NULL) {
                perror("Failed to allocate memory for file path");
                closedir(dir);
                return NULL;
            }

            sprintf(path, "%s/%s", THEME_FOLDER_ARG, entry->d_name); 
            t = load_file(path);
            
            if (t != NULL)
                return t;
        }
    }

    //log_c("Theme not found in directory: \"%s\", trying as path");

    t = load_file(themename);
    if (t != NULL)
        return t;

    return NULL;
}

int hex_to_rgb(const char *hex, RGB *p)
{
    if (!hex || hex[0] != '#' || (strlen(hex) != 7 && strlen(hex) != 4)) {
        return 0;
    }

    if (strlen(hex) == 7) {
        // #RRGGBB format
        p->R = (int)strtol(hex + 1, NULL, 16) >> 16 & 0xFF;
        p->G = (int)strtol(hex + 3, NULL, 16) >> 8 & 0xFF;
        p->B = (int)strtol(hex + 5, NULL, 16) & 0xFF;
    } else if (strlen(hex) == 4) {
        // #RGB format (expand each digit to 2 digits)
        p->R = (int)strtol((char[]){hex[1], hex[1], '\0'}, NULL, 16);
        p->G = (int)strtol((char[]){hex[2], hex[2], '\0'}, NULL, 16);
        p->B = (int)strtol((char[]){hex[3], hex[3], '\0'}, NULL, 16);
    }

    return 1;
}

int is_color_palette_var(char *name)
{
    for (size_t i = 0; i < PALETTE_SIZE; i++) {
        char *col = calloc(1, 16);
        sprintf(col, "color%lu", i);

        if (strcmp(col, name) == 0)
            return i;
    }
    return -1;
}

int process_theme(char *t, PALETTE *pal)
{
    int processed_colors = 0;

    hell_parser_t *p = hell_parser_create(t);
    if (p == NULL) 
        err("Failed to create parser");

    while (!hell_parser_eof(p))
    {
        char ch;
        if (hell_parser_next(p, &ch) == HELL_PARSER_OK)
        {
            if (ch == HELLWAL_DELIM)
            {
                p->pos -= 1;  
                char *delim_buf = NULL;

                if (hell_parser_delim_buffer_between(p, HELLWAL_DELIM, HELLWAL_DELIM_COUNT, &delim_buf) == HELL_PARSER_OK)
                {
                    hell_parser_t *pd = hell_parser_create(delim_buf);
                    if (pd == NULL)
                        err("Failed to allocate parser");

                    if (hell_parser_delim(pd, '=', 1) == HELL_PARSER_OK) {
                        size_t var_size = pd->pos - 1;
                        size_t val_size = pd->length - pd->pos + 1;

                        char *variable = calloc(1, var_size);
                        char *value = calloc(1, val_size);

                        strncpy(variable, pd->input, var_size);
                        strncpy(value, pd->input + pd->pos + 1, val_size);

                        remove_whitespaces(variable);
                        remove_whitespaces(value);

                        RGB p;
                        if (hex_to_rgb(value, &p))
                        {
                            int idx = is_color_palette_var(variable);
                            if (idx != -1) {
                                pal->colors[idx] = p;
                                processed_colors++;
                            }
                        }
                        else
                        {
                            free(variable);
                            free(value);
                        }
                    }

                    free(delim_buf);
                }
            }
        }
    }
    hell_parser_destroy(p);

    if (processed_colors == PALETTE_SIZE)
        return 1;
    if (processed_colors > 0)
        free(pal);

    return 0;
}

PALETTE process_themeing(char *theme)
{
    char *t = load_theme(theme);
    PALETTE pal;

    if (t!=NULL)
        if (process_theme(t, &pal))
            return pal;
        else
            err("Not enough colors were specified in color palette: %s", theme);
    else
        err("Theme not found: %s", theme);

    pal_log = pal;
    pal_log_iter = 2;
    return pal;
}

/***
 * MAIN
 ***/
int main(int argc, char **argv)
{
    if (set_args(argc,argv) != 0)
        err("arguments error");

    PALETTE pal;

    if (THEME_ARG)
        pal = process_themeing(THEME_ARG); /* if true, program end's here */
    else
    {
        IMG *img = img_load(IMAGE_ARG);
        pal = gen_palette(img);
        img_free(img);
    }

    if (LIGHT_ARG != NULL && DARK_ARG == NULL)
        reverse_palette(&pal);

    palette_print(pal);
    set_term_colors(pal);
    process_templating(pal);

    return 0;
}
