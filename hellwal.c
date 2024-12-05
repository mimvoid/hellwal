/*  hellwal - v1.0.0 - MIT LICENSE
 *
 *  [ ] TODO: gtk css?
 *  [ ] TODO: better light theme
 *  [ ] TODO: config ( is it really needed? )                               
 *  [ ] TODO: support for other OS's like Mac or Win                        
 *  [ ] TODO: bright & dark offset value as cmd line argument               
 *  [ ] TODO: cmd line arg to manipulate variety of different colors        
 *  [ ] TODO: handle exception: unclosed delim
 *  ------------------------------------------------------------------------
 *  [x] TODO: tweaking options for generated colors (func + dark-light mode 
 *  [x] TODO: support for already built themes (like gruvbox etc.)          
 *  [x] TODO: do more pleasant color schemes                                
 *  [x] TODO: print proper program usage                                    
 *  [x] TODO: -r for random                                                 
 *  [x] TODO: -s for scripts                                                
 *  [x] TODO: gen. colors                                                   
 *  [x] TODO: templating                                                    
 *  [x] TODO: parsing                                                       
 *
 *
 * changelog v1.0.1:
 *  - arguably better light mode
 *
 * changelog v1.0.0:
 *  - first decent verision to release.
 *  - combined couple of ways into one for generating colorpalette
 *  - added bins to obtains more precise colors
 *  - fixed: half of palette is no more white randomly
 *  - get rid of unecessary functions
 *
 * ----------------------------------------------------------------------------------------------------
 *
 *  changelog v0.0.7:
 *  - improved colors, gen_palette() function is re-designed (again)
 *  - changed wallpaper gen_palette algorithm to median cut
 *  - fixed some templates
 *
 *  changelog v0.0.6:
 *  - improved colors, gen_palette() function is re-designed
 *  - added in ./assests example script how you can use hellwal
 *  - removed colors.sh
 *
 *  changelog v0.0.5:
 *  - fixed .hex, .rgb while using keyword like (background, foreground etc.)
 *  - themes adjustments
 *  - added rgb template
 *  - changed directories errors to warn, to allow to use same variable as path
 *
 *  changelog v0.0.4:
 *   - combined logging palletes into one
 *   - support for --random (pics random image or template from specified directory)
 *   - support for executing scripts after running hellwal
 *   - added terminal template, adjusted colors.sh
 *   - better --help
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


#include <time.h>
#include <glob.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HELL_PARSER_IMPLEMENTATION
#include "hell_parser.h"


/***
 * MACROS
 ***/

/* just palette size */
#define PALETTE_SIZE 16
#define BINS 8

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
    uint8_t R; // Red   [0, 255]
    uint8_t G; // Green [0, 255]
    uint8_t B; // Blue  [0, 255]
} RGB;

typedef struct {
    float H; // Hue        [0, 360]
    float S; // Saturation [0, 1]
    float L; // Luminance  [0, 1]
} HSL;

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

/* pick random THING (image or theme) of provided folder path:
 *   - provide path to IMG using *IMAGE_ARG
 *   - or *THEME_FOLDER_ARG
 */
char *RANDOM_ARG = NULL;

/* run script after successfull hellwal job */
char *SCRIPT_ARG = NULL;

/* one palette as global variable, so log_c() can access colors
 * 1 = log
 * 2 = err
 * 3 = warn
 * */
size_t pal_log_iter = 1;
PALETTE pal_log = {
    /* log */
    .colors[1].R = 235,
    .colors[1].G = 255,
    .colors[1].B = 235,

    /* warn */
    .colors[2].R = 235,
    .colors[2].G = 255,
    .colors[2].B = 10,

    /* err */
    .colors[3].R = 235,
    .colors[3].G = 20,
    .colors[3].B = 5
 };


/*** 
 * FUNCTIONS DECLARATIONS
 ***/

/* args */
int set_args(int argc, char *argv[]);

/* utils */
RGB clamp_rgb(RGB color);
uint8_t clamp_uint8(int value);

char *rand_file(char *path);
char* home_full_path(const char* path);

void check_output_dir(char *path);
void run_script(const char *script);
void hellwal_usage(const char *name);

/* logging */
void eu(const char *format, ...);
void err(const char *format, ...);
void warn(const char *format, ...);
void log_c(const char *format, ...);

/* IMG */
IMG *img_load(char *filename);
void img_free(IMG *img);

/* RGB */
void print_rgb(RGB col);
HSL rgb_to_hsl(RGB color);
void median_cut(RGB *colors, size_t *starts, size_t *ends, size_t *num_boxes, size_t target_boxes);

float calculate_luminance(RGB c);
float color_distance(RGB color1, RGB color2);
float calculate_color_distance(RGB a, RGB b);

int is_more_colorful(RGB a, RGB b);
int is_color_palette_var(char *name);
int compare_luminance(const void *a, const void *b);
int get_channel(RGB *colors, size_t start, size_t end, int channel);
int is_color_too_similar(RGB *palette, int num_colors, RGB new_color);

RGB darken_color(RGB color, float factor);
RGB lighten_color(RGB color, float factor);
RGB saturate_color(RGB color, float factor);
RGB adjust_luminance(RGB color, float factor);
RGB bin_to_color(int r_bin, int g_bin, int b_bin);
RGB average_color(IMG *img, size_t start, size_t end);
RGB blend_colors(const RGB color1, const RGB color2, float blend_factor);
RGB blend_with_brightness(RGB bright_color, RGB mix_color, float mix_ratio);

/* term */
void set_term_colors(PALETTE pal);

/* palettes */
PALETTE gen_palette(IMG *img);
void print_palette(PALETTE pal);
void reverse_palette(PALETTE *palette);
void sort_palette_by_luminance(PALETTE *palette);
char *palette_color(PALETTE pal, unsigned c, char *fmt);

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
    printf("  --image,           -i <image>     Set image file.\n");
    printf("  --dark,            -d             Set dark mode (ON by default).\n");
    printf("  --light,           -l             Set light mode.\n");
    printf("  --quiet,           -q             Suppress output.\n");
    printf("  --script,          -s             Execute script after running hellwal.\n");
    printf("  --random,          -r             Pick random image or theme from directory.\n");
    printf("  --template-folder, -f <folder>    Set folder containing templates.\n");
    printf("  --output,          -o <folder>    Set output folder for generated templates.\n");
    printf("  --theme,           -t <file>      Set theme from theme folder or path to theme file.\n");
    printf("  --theme-folder,    -k <folder>    Set folder containing themes.\n");
    printf("  --help,            -h             Display this help and exit.\n");

    printf("\nDetailed Description:\n");
    printf("  --image: Path to the image used to create a color palette.\n");
    printf("  --dark: Use dark mode colors (e.g., #000000).\n");
    printf("  --light: Use light mode colors (e.g., #FFFFFF).\n");
    printf("  --quiet: Suppress output messages.\n");
    printf("  --script: Specify a script or shell command to execute after running hellwal.\n");
    printf("  --random: Pick a random image or theme from a directory. Specify the directory using --image or --theme-folder.\n");
    printf("  --template-folder: Folder containing templates to process for generating colors. Default: ~/.config/hellwal/templates.\n");
    printf("  --output: Output folder for generated templates. Default: ~/.cache/hellwal/.\n");
    printf("  --theme: Specify a theme name from the theme folder or provide a path to a theme file.\n");
    printf("  --theme-folder: Folder containing themes. Default: ~/.config/hellwal/themes/.\n");

    printf("\nNote for package manager installations:\n");
    printf("Default templates and themes are pre-installed in /etc/hellwal/templates and /etc/hellwal/themes.\n");
    printf("You can copy these to your configuration directory.\n");
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
            else {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--image") == 0 || strcmp(argv[i], "-i") == 0))
        {
            if (i + 1 < argc)
                IMAGE_ARG = argv[++i];
            else {
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
        else if ((strcmp(argv[i], "--random") == 0 || strcmp(argv[i], "-r") == 0))
        {
            /* anything other than NULL, triggers */
            RANDOM_ARG = "";
        }
        else if ((strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0))
        {
            if (i + 1 < argc)
                OUTPUT_ARG = argv[++i];
            else {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--theme") == 0 || strcmp(argv[i], "-t") == 0))
        {
            if (i + 1 < argc)
                THEME_ARG = argv[++i];
            else {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--theme-folder") == 0 || strcmp(argv[i], "-k") == 0))
        {
            if (i + 1 < argc)
                THEME_FOLDER_ARG = argv[++i];
            else {
                argc = -1;
            }
        }
        else if ((strcmp(argv[i], "--script") == 0 || strcmp(argv[i], "-s") == 0))
        {
            if (i + 1 < argc)
                SCRIPT_ARG = argv[++i];
            else
                argc = -1;
        }
        else {
            eu("Unknown option: %s", argv[i]);
        }
    }

    if (argc == -1)
        err("Incomplete option: %s", argv[j]);

    /* handle needed arguments and warns - idk how to do this the other way */
    if (RANDOM_ARG != NULL && (THEME_FOLDER_ARG == NULL && IMAGE_ARG == NULL))
        err("you have to specify --image to provide image folder or --theme-folder to use RANDOM");

    if (IMAGE_ARG == NULL && THEME_ARG == NULL && ((THEME_FOLDER_ARG == NULL || TEMPLATE_FOLDER_ARG == NULL) && RANDOM_ARG == NULL))
        err("You have to provide one of these:\n\t--image, \n\t--theme, \n\t--random");

    if (THEME_ARG != NULL && IMAGE_ARG != NULL)
        err("you cannot use both --image and --theme");

    if (RANDOM_ARG != NULL && THEME_ARG != NULL)
        warn("specified theme is not used: \"%s\"", THEME_ARG);

    if (THEME_ARG == NULL && THEME_FOLDER_ARG != NULL && RANDOM_ARG == NULL)
        warn("specified theme folder is not used: \"%s\", you have to provide --theme", THEME_FOLDER_ARG);

    /* if RANDOM, get file */
    if (RANDOM_ARG != NULL)
    {
        if (IMAGE_ARG != NULL)
            IMAGE_ARG = rand_file(IMAGE_ARG);
        else
            THEME_ARG = rand_file(THEME_FOLDER_ARG);
    }

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
    fprintf(stderr, "\033[38;2;%d;%d;%dm[ERROR]: ",
            pal_log.colors[3].R,
            pal_log.colors[3].G,
            pal_log.colors[3].B);

    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    fprintf(stderr, "\033[0m\n");

    hellwal_usage("hellwal");

    exit(EXIT_FAILURE);
}

/* prints to stderr formatted output and exits with EXIT_FAILURE */
void err(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    fprintf(stderr, "\033[38;2;%d;%d;%dm[ERROR]: ",
            pal_log.colors[3].R,
            pal_log.colors[3].G,
            pal_log.colors[3].B);
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
            pal_log.colors[2].R,
            pal_log.colors[2].G,
            pal_log.colors[2].B);

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

/* run script from given path */
void run_script(const char *script)
{
    if (script == NULL)
        return;

    log_c("Starting script: %s", script);
    size_t exit_code = system(script);
    if (exit_code == 0)
        log_c("Script \"%s\" exited with code: %d", script, exit_code);
    else
        err("Script \"%s\" exited with code: %d", script, exit_code);
}

/* avoid exceeding max value */
uint8_t clamp_uint8(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

/* avoid exceeding max value for each */
RGB clamp_rgb(RGB color)
{
    return (RGB)
    {
        .R = clamp_uint8(color.R),
        .G = clamp_uint8(color.G),
        .B = clamp_uint8(color.B)
    };
}

/* get random file from given path */
char *rand_file(char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
        err("Cannot access directory %s: ", path);

    struct dirent *entry;
    char **files = NULL;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG)
        {
            files = realloc(files, sizeof(char *) * (count + 1));
            if (files == NULL)
            {
                perror("Memory allocation failed");
                closedir(dir);
                return NULL;
            }
            files[count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    if (count == 0) {
        free(files);
        err("No files found in directory: %s\n", path);
    }

    srand(time(NULL));
    size_t r_idx = rand() % count;
    char *choosen = calloc(1, strlen(path) + strlen(files[r_idx] + 2));
    sprintf(choosen, "%s/%s", path, strdup(files[r_idx]));

    for (size_t i = 0; i < count; i++)
        free(files[i]);
    free(files);

    return choosen;
}

/* removes whitespaces from buffer */
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
float calculate_luminance(RGB c)
{
    return (0.2126 * c.R + 0.7152 * c.G + 0.0722 * c.B); 
}

/* sort palette by luminance to spread out colors */
void sort_palette_by_luminance(PALETTE *palette)
{
    qsort(palette->colors, 8, sizeof(RGB), compare_luminance);
}

/* compare two RGB colors */
int compare_luminance(const void *a, const void *b)
{
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

/* saturate a color */
RGB saturate_color(RGB color, float factor)
{
    float max_val = fmaxf(color.R, fmaxf(color.G, color.B));
    color.R = (uint8_t)(color.R + (max_val - color.R) * factor);
    color.G = (uint8_t)(color.G + (max_val - color.G) * factor);
    color.B = (uint8_t)(color.B + (max_val - color.B) * factor);
    return color;
}

/* Convert RGB to hsl */
HSL rgb_to_hsl(RGB color)
{
    float r = color.R / 255.0f;
    float g = color.G / 255.0f;
    float b = color.B / 255.0f;
    
    float max_val = fmaxf(r, fmaxf(g, b));
    float min_val = fminf(r, fminf(g, b));
    float delta = max_val - min_val;

    HSL hsl;
    hsl.L = max_val;

    if (max_val != 0)
    {
        hsl.S = delta / max_val; // Saturation
    }
    else
    {
        hsl.S = 0;
        hsl.H = -1;
        return hsl;
    }

    if (delta == 0)
    {
        hsl.H = 0;
    }
    else
    {
        if (r == max_val)
        {
            hsl.H = (g - b) / delta;
        }
        else if (g == max_val)
        {
            hsl.H = 2 + (b - r) / delta;
        }
        else
        {
            hsl.H = 4 + (r - g) / delta;
        }

        hsl.H *= 60;

        if (hsl.H < 0)
        {
            hsl.H += 360;
        }
    }

    return hsl;
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

/* Get channel */
int get_channel(RGB *colors, size_t start, size_t end, int channel)
{
    uint8_t min = 255, max = 0;
    for (size_t i = start; i < end; i++)
    {
        uint8_t value = ((uint8_t *)&colors[i])[channel];
        if (value < min) min = value;
        if (value > max) max = value;
    }
    return max - min;
}

/* check Euclidean distance between two colors to ensure diversity */
float calculate_color_distance(RGB a, RGB b)
{
    return sqrtf(powf(a.R - b.R, 2) + powf(a.G - b.G, 2) + powf(a.B - b.B, 2));
}

/* ensure that new color is not too similar to existing colors in the palette */
int is_color_too_similar(RGB *palette, int num_colors, RGB new_color)
{
    for (int i = 0; i < num_colors; i++)
    {
        if (calculate_color_distance(palette[i], new_color) < 35)
            return 1;  // Color is too similar
    }
    return 0;
}

/* blend two colors together based on a blend factor */
RGB blend_colors(RGB c1, RGB c2, float weight) {
    weight = (weight < 0.0f) ? 0.0f : (weight > 1.0f) ? 1.0f : weight; // Clamp weight
    return clamp_rgb((RGB){
        .R = (int)(c1.R * (1 - weight) + c2.R * weight),
        .G = (int)(c1.G * (1 - weight) + c2.G * weight),
        .B = (int)(c1.B * (1 - weight) + c2.B * weight)
    });
}

/* 
 * blend two colors together based on a blend factor,
 * bright mode - leave little color accent on white surface
 */
RGB blend_with_brightness(RGB bright_color, RGB mix_color, float mix_ratio)
{
    if (mix_ratio < 0.0f) mix_ratio = 0.0f;
    if (mix_ratio > 1.0f) mix_ratio = 1.0f;

    RGB blended;
    blended.R = bright_color.R + mix_ratio * (mix_color.R - bright_color.R);
    blended.G = bright_color.G + mix_ratio * (mix_color.G - bright_color.G);
    blended.B = bright_color.B + mix_ratio * (mix_color.B - bright_color.B);

    uint8_t max_channel = blended.R > blended.G ? (blended.R > blended.B ? blended.R : blended.R) : (blended.G > blended.B ? blended.G : blended.B);
    if (max_channel < 255)
    {
        float adjustment = 255.0f / max_channel;
        blended.R = (uint8_t)(blended.R * adjustment);
        blended.G = (uint8_t)(blended.G * adjustment);
        blended.B = (uint8_t)(blended.B * adjustment);
    }

    return blended;
}

/* calculate the average color of a given pixel range in an image */
RGB average_color(IMG *img, size_t start, size_t end) 
{
    long sum_r = 0, sum_g = 0, sum_b = 0;
    size_t count = end - start;

    for (size_t i = start; i < end; i++)
    {
        sum_r += img->pixels[i * 3];
        sum_g += img->pixels[i * 3 + 1];
        sum_b += img->pixels[i * 3 + 2];
    }

    return clamp_rgb(
    (RGB)
    {
        .R = (int)(sum_r / count),
        .G = (int)(sum_g / count),
        .B = (int)(sum_b / count)
    });
}

/* convert bin indices to an rgb color */
RGB bin_to_color(int r_bin, int g_bin, int b_bin)
{
    return clamp_rgb(
    (RGB)
    {
        .R = r_bin * (256 / BINS),
        .G = g_bin * (256 / BINS),
        .B = b_bin * (256 / BINS)
    });
}

/* adjust luminance of a color */
RGB adjust_luminance(RGB color, float factor)
{
    return (RGB){
        .R = (uint8_t)fminf(color.R * factor, 255),
        .G = (uint8_t)fminf(color.G * factor, 255),
        .B = (uint8_t)fminf(color.B * factor, 255)
    };
}

/* lighten a color by a factor */
RGB lighten_color(RGB color, float factor)
{
    return adjust_luminance(color, 1.0f + factor);
}

/* darken a color by a factor */
RGB darken_color(RGB color, float factor)
{
    return adjust_luminance(color, 1.0f - factor);
}

/* part of median algo */
size_t partition_colors(RGB *colors, size_t start, size_t end, int channel, uint8_t pivot)
{
    size_t left = start, right = end - 1;
    while (left <= right)
    {
        while (((uint8_t *)&colors[left])[channel] <= pivot && left < end) left++;
        while (((uint8_t *)&colors[right])[channel] > pivot && right > start) right--;
        if (left < right)
        {
            RGB temp = colors[left];
            colors[left] = colors[right];
            colors[right] = temp;
        }
    }
    return left;
}

/* perform median cut to partition the color space */
void median_cut(RGB *colors, size_t *starts, size_t *ends, size_t *num_boxes, size_t target_boxes) 
{
    while (*num_boxes < target_boxes)
    {
        size_t largest_segment_index = 0;
        int largest_range = 0;

        for (size_t i = 0; i < *num_boxes; i++)
        {
            size_t start = starts[i];
            size_t end = ends[i];
            int range_r = get_channel(colors, start, end, 0);
            int range_g = get_channel(colors, start, end, 1);
            int range_b = get_channel(colors, start, end, 2);
            int max_range = fmax(range_r, fmax(range_g, range_b));
            if (max_range > largest_range) {
                largest_range = max_range;
                largest_segment_index = i;
            }
        }

        size_t start = starts[largest_segment_index];
        size_t end = ends[largest_segment_index];
        int channel = (largest_range == get_channel(colors, start, end, 0)) ? 0 :
                      (largest_range == get_channel(colors, start, end, 1)) ? 1 : 2;

        size_t mid = (end - start) / 2;
        uint8_t pivot = ((uint8_t *)&colors[start + mid])[channel];

        size_t median = partition_colors(colors, start, end, channel, pivot);

        // Update the segments
        starts[largest_segment_index] = start;
        ends[largest_segment_index] = median;
        starts[*num_boxes] = median;
        ends[*num_boxes] = end;
        (*num_boxes)++;
    }
}

void gen_palette_handle_light_mode(PALETTE *p)
{
    if (p == NULL)
        return;

    for (int i = PALETTE_SIZE / 2; i < PALETTE_SIZE; i++)
        p->colors[i] = lighten_color(p->colors[i - PALETTE_SIZE / 2], 0.2);

    RGB temp_color_0 =  blend_with_brightness(saturate_color(lighten_color(p->colors[15], 0.6), 0.6), p->colors[5], 0.2f);
    RGB temp_color_7 =  darken_color( p->colors[7], 0.5);
    RGB temp_color_15 = p->colors[0];

    p->colors[14] = darken_color(p->colors[14], 0.3f);

    p->colors[0] = temp_color_0;
    p->colors[7] = temp_color_7;
    p->colors[15] = temp_color_15;
}

void gen_palette_handle_dark_mode(PALETTE *p)
{
    if (p==NULL)
        return;

    for (int i = PALETTE_SIZE / 2; i < PALETTE_SIZE; i++)
        p->colors[i] = lighten_color(p->colors[i - PALETTE_SIZE / 2], 0.25f);

    p->colors[0] = darken_color(p->colors[0], 0.7f);    // BG
    p->colors[15] = lighten_color(p->colors[15], 0.5f); // FG
    p->colors[7] = lighten_color(p->colors[7], 0.4f);   // Term text
}

PALETTE gen_palette(IMG *img)
{
    PALETTE palette;
    size_t total_pixels = img->size / 3;
    RGB *all_colors = (RGB *)img->pixels;
    int num_colors = 0;

    size_t starts[PALETTE_SIZE / 2] = {0};
    size_t ends[PALETTE_SIZE / 2] = {total_pixels};
    size_t num_boxes = 1;

    median_cut(all_colors, starts, ends, &num_boxes, PALETTE_SIZE / 2);

    int histogram[BINS][BINS][BINS] = {{{0}}};
    for (size_t i = 0; i < total_pixels; i++)
    {
        int r_bin = img->pixels[i * 3] / (256 / BINS);
        int g_bin = img->pixels[i * 3 + 1] / (256 / BINS);
        int b_bin = img->pixels[i * 3 + 2] / (256 / BINS);

        histogram[r_bin][g_bin][b_bin] = (histogram[r_bin][g_bin][b_bin] < INT_MAX) ? histogram[r_bin][g_bin][b_bin] + 1 : INT_MAX;
    }

    typedef struct {
        int count;
        int r_bin, g_bin, b_bin;
    } BinCount;

    BinCount top_bins[(PALETTE_SIZE / 2)] = {0};

    for (int r = 0; r < BINS; r++)
    {
        for (int g = 0; g < BINS; g++)
        {
            for (int b = 0; b < BINS; b++)
            {
                int count = histogram[r][g][b];
                if (count > top_bins[(PALETTE_SIZE / 2) - 1].count)
                {
                    top_bins[(PALETTE_SIZE / 2) - 1] = (BinCount){count, r, g, b};
                    for (int k = (PALETTE_SIZE / 2) - 1; k > 0 && top_bins[k].count > top_bins[k - 1].count; k--)
                    {
                        BinCount temp = top_bins[k];
                        top_bins[k] = top_bins[k - 1];
                        top_bins[k - 1] = temp;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < PALETTE_SIZE / 2; i++)
    {
        RGB avg_color = average_color(img, starts[i], ends[i]);
        RGB bin_color = bin_to_color(top_bins[i].r_bin, top_bins[i].g_bin, top_bins[i].b_bin);
        RGB blended_colors = blend_colors(avg_color, bin_color, 0.5f);

        print_rgb(avg_color);
        printf(" - ");
        print_rgb(bin_color);
        printf(" = ");
        print_rgb(blended_colors);
        printf("\n");

        palette.colors[num_colors++] = blended_colors;
    }

    size_t sample_points[] = {
        0,
        (img->height - 1) * img->width,
        (img->height - 1) * img->width + (img->width - 1),
        (img->height / 2) * img->width + (img->width / 2),
        (img->height / 4) * img->width + (img->width / 4),
        (img->height / 4) * img->width + 3 * (img->width / 4),
        3 * (img->height / 4) * img->width + (img->width / 4),
        3 * (img->height / 4) * img->width + 3 * (img->width / 4)
    };

    for (size_t j = 0; j < sizeof(sample_points) / sizeof(sample_points[0]) && num_colors < PALETTE_SIZE; j++)
    {
        size_t i = sample_points[j];
        if (i + 3 >= img->size) continue;

        RGB new_color = {img->pixels[i], img->pixels[i + 1], img->pixels[i + 2]};

        if (!is_color_too_similar(palette.colors, num_colors, new_color))
        {
            palette.colors[num_colors++] = new_color;
        }
        else
        {
            palette.colors[num_colors++] = blend_colors(new_color, *palette.colors, 0.5);
        }
    }


    /* 
     * if user provided LIGHT_ARG, make some changes
     * to make it look better
     */
    sort_palette_by_luminance(&palette);

    /* Handle dark/light mode */
    if (LIGHT_ARG != NULL)
        gen_palette_handle_light_mode(&palette);
    else
        gen_palette_handle_dark_mode(&palette);

    return palette;
}

/* Writes color as block to stdout - it does not perform new line by itself */
void print_rgb(RGB col)
{
    if (QUIET_ARG != NULL)
        return;

    /* Write color from as colored block */
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", col.R, col.G, col.B);
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", col.R, col.G, col.B);
}

/* Writes palete to stdout */
void print_palette(PALETTE pal)
{
    if (QUIET_ARG != NULL)
        return;

    for (size_t i=0; i<PALETTE_SIZE; i++)
    {
        print_rgb(pal.colors[i]);
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
    int forcedNumberOfChannels = 3;

    uint8_t *imageData = stbi_load(filename, &width, &height, &numberOfChannels, forcedNumberOfChannels); /* fix BAD number of channels. It causes unmatched and inaccurate colors */

    if (imageData == 0) err("Error while loading the file: %s", filename);

    IMG *img = malloc(sizeof(IMG));

    img->size = width * height * forcedNumberOfChannels;
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
    //log_c("  - generating template buffer: %s", t->name);

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
                        size_t r_size = pd->length - pd->pos;

                        char *left  = calloc(1, l_size);
                        char *right = calloc(1, r_size);

                        strncpy(left, pd->input, l_size);
                        strncpy(right, pd->input + pd->pos, r_size);

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
                        else if (!strcmp(left, "foreground") || !strcmp(left, "cursor") || !strcmp(left, "border"))
                        {
                            if (!strcmp(right, "rgb"))
                                var_arg = palette_color(pal, 15, "%d, %d, %d");
                            else
                                var_arg = palette_color(pal, 15, "%02x%02x%02x");
                        }
                        else if (!strcmp(left, "background"))
                        {
                            if (!strcmp(right, "rgb"))
                                var_arg = palette_color(pal, 0, "%d, %d, %d");
                            else
                                var_arg = palette_color(pal, 0, "%02x%02x%02x");
                        }

                        free(left);
                        free(right);
                    }
                    else if (!strcmp(delim_buf, "wallpaper")) /* check if an argument stands for wallpaper path */
                    {
                        if (IMAGE_ARG != NULL)
                        {
                            len = strlen(IMAGE_ARG);
                            var_arg = IMAGE_ARG;
                        }
                        else if (THEME_ARG)
                            var_arg = THEME_ARG;
                        else
                            var_arg = "";
                    }
                    else if (!strcmp(delim_buf, "foreground") || !strcmp(delim_buf, "cursor") || !strcmp(delim_buf, "border"))
                        var_arg = palette_color(pal, 15, "%02x%02x%02x");
                    else if (!strcmp(delim_buf, "background"))
                        var_arg = palette_color(pal, 0, "%02x%02x%02x");
                    else
                    {
                        /* '.' was not found, try to find color, put hex by default on it */
                        idx = is_color_palette_var(delim_buf);

                        if (idx != -1)
                                var_arg = palette_color(pal, idx, "%02x%02x%02x");
                    }

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
    if (dir == NULL) {
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

            t_arr[size] = calloc(1 ,sizeof(TEMPLATE));
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
    //log_c("  - wrote template to: %s\n", path);

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
    char *t = NULL;
    struct dirent *entry;

    if (dir == NULL)
        warn("Cannot access directory: %s", THEME_FOLDER_ARG);
    else
    {
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
    }
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

int is_more_colorful(RGB a, RGB b)
{
    return rgb_to_hsl(a).S > rgb_to_hsl(b).S;
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
    {
        if (!process_theme(t, &pal))
            err("Not enough colors were specified in color palette: %s", theme);
    }
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
    else {
        IMG *img = img_load(IMAGE_ARG);
        pal = gen_palette(img);
        img_free(img);
    }

    print_palette(pal);
    set_term_colors(pal);
    process_templating(pal);

    /* Run script or command from --script argument */
    run_script(SCRIPT_ARG);

    return 0;
}
