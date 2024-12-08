# Hellwal - Fast, Extensible Color Palette Generator

<p align="center">
    <p>Showcase with <a href="https://github.com/danihek/Themecord">Themecord</a> and <a href="https://github.com/LGFae/swww">swww</a></p>
    <img src="https://github.com/user-attachments/assets/ee6a9753-1996-4b80-b8e6-8e4e9c96d57c" alt="Preview" width="1200">
</p>

## Installation

To install from **AUR**, run:

```sh
yay -S hellwal
```

## Building

Clone git repo, run make command and you are ready to go! - you just need C compiler and gnumake!

```sh
git clone https://github.com/danihek/hellwal && cd hellwal  && make
```

## How to use?

Run this with your wallpaper image:

```sh
hellwal -i [image]
```

You can also randomly pick image from given directory like this:

```sh
hellwal -i <folder> --random
```

Generated, templates are saved in ``~/.cache/hellwal/`` directory.

## Templates
**[INFO]** - if you got hellwal from **AUR**, examples of default templates are stored in `/usr/share/docs/`

Look up for templating examples in [templates folder](./templates), they look more-less like this:

```sh
# Main
background='%%background%%'
foreground='%%foreground%%'

# Path
wallpaper_path='%%wallpaper%%'

# Colors
color0='%%color0.hex%%'
color1='%%color1.hex%%'

# ... and so on and so fourth.

color15='%%color15.hex%%'


# RGB
backgroundRGB='%%background.rgb%%'
foregroundRGB='%%foreground.rgb%%'

color15butRGB='%%color15.rgb%%'

```

You can get any generated color between 0-15 values.
By writing '.' after keyword you can specify type: hex or rgb. If you havent specify this output of template will be in hex.


### Available color template formats:

| Type | Input      | Output  |
|------|------------|---------|
| hex  | color0.hex | 000000  |
| rgb  | color0.rgb | 0, 0, 0 |

## Themes

You can set your own theme, re-run it anytime and apply to your config or other programs!
It can be previously generated palette from image, gruvbox, tokyonight or anything you want!
For example gruvbox theme:

```sh
%% color0  = #282828 %%
%% color1  = #cc241d %%
%% color2  = #98971a %%
%% color3  = #d79921 %%
%% color4  = #458588 %%
%% color5  = #b16286 %%
%% color6  = #689d6a %%
%% color7  = #a89984 %%
%% color8  = #928374 %%
%% color9  = #fb4934 %%
%% color10 = #b8bb26 %%
%% color11 = #fabd2f %%
%% color12 = #83a598 %%
%% color13 = #d3869b %%
%% color14 = #8ec07c %%
%% color15 = #ebdbb2 %%
```

Save text above as file or take from this repo [gruvbox](./themes/gruvbox.hellwal) and just run hellwal:

```sh
hellwal --theme ./themes/gruvbox.hellwal
```

I recommend to put all themes to ``~/.config/hellwal/themes folder``, because from there you can just provide theme name, and it will pick it up automatically, without specifying path. Of course if you want, you can also set different theme-folder path. For example:

```sh
hellwal -t gruvbox.hellwal --theme-folder ~/dotfiles/configs/hellwal/themes
```

### Modes

You can select ``-d`` and ``--dark`` or ``-l`` and ``--light`` mode on every given image, theme etc, no matter if it's generated from image or from theme file.lashbacks.

### Scripts

With ``--script`` or ``-s`` you can run script(or any shell command) after hellwal.
**Note**: it will only run if hellwal will not encounter any errors.

### On a side note:

If you want your new terminals to open with previusly generated or specified color palette, add this templates to your ``~/.config/hellwal/templates/`` folder:
    - variables.sh
    - terminal.sh

then in ``.bash.rc`` add following lines:

```sh
source ~/.cache/hellwal/variables.sh
sh ~/.cache/hellwal/terminal.sh
```

### TODO

- [ ] TODO: gtk css?
- [ ] TODO: config ( is it really needed? )                               
- [ ] TODO: support for other OS's like Mac or Win                        
- [ ] TODO: handle exception or warn: unclosed delim
--------------------------------------------------------------------------
- [x] TODO: tweaking options for generated colors (func + dark-light mode 
- [x] TODO: bright & dark offset value as cmd line argument               
- [x] TODO: support for already built themes (like gruvbox etc.)          
- [x] TODO: do more pleasant color schemes                                
- [x] TODO: better light theme                                            
- [x] TODO: print proper program usage                                    
- [x] TODO: -r for random                                                 
- [x] TODO: -s for scripts                                                
- [x] TODO: gen. colors                                                   
- [x] TODO: templating                                                    
- [x] TODO: parsing                                                       

# Showcase
![showcase2](https://github.com/user-attachments/assets/ddf2a55e-0fbb-4661-827a-6b124f1dacdb)

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
