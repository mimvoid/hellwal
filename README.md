# hellwal - fast, extensible colorscheme generator

- [ ] TODO: config ( is it really needed? )                               
- [ ] TODO: support for other OS's like Mac or Win                        
--------------------------------------------------------------------------
- [x] TODO: tweaking options for generated colors (func + dark-light mode 
- [x] TODO: support for already built themes (like gruvbox etc.)          
- [x] TODO: do more pleasant color schemes                                
- [x] TODO: print proper program usage                                    
- [x] TODO: -r for random                                                 
- [x] TODO: -s for scripts                                                
- [x] TODO: gen. colors                                                   
- [x] TODO: templating                                                    
- [x] TODO: parsing                                                       

## Installation

Go to [releases](https://github.com/danihek/hellwal/releases) page and download an [hellwal](https://github.com/danihek/hellwal/releases/download/1.0.0/hellwal) executable

## Building

Clone git repo, run make command and you are ready to go!

``git clone https://github.com/danihek/hellwal && make``

## How to use?

Just run ``hellwal -i [image]`` with your image/wallpaper file, and you will get your terminals colored, (if any)templates will be saved in ~/.cache/hellwal/ or other specified directory.
You can also run with ``-i <folder> -r`` - it will randomly pick image from directory.

## Templates
Look up for templating examples in ./templates folder, they look more-less like this:
```
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
```

You can set specific color by writing number from 0 to 15, and output format by writing '.' after (eg. `.hex` in `colors0.hex` or `.rgb` in `colors0.rgb` - in case you haven't provided arg after '.', default one is set to hex.)

### Available color template formats:

| Type | Input      | Output  |
|------|------------|---------|
| hex  | color0.hex | ffffff  |
| rgb  | color0.rgb | 0, 0, 0 |

## Themes
You can set your own theme, re-run it anytime and apply to your config or other programs!

For example - set gruvbox theme in ~/.config/hellwal/themes/[name].hellwal:

```
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

Save text above as file or take from this repo [./themes/gruvbox.hellwal](gruvbox) and just run hellwal:

``
hellwal --theme ./themes/gruvbox.hellwal
``

I recommend to put all themes to ``~/.config/hellwal/themes folder``, because from there you can just provide theme name, and it will pick it up automatically, without specifying path. Of course if you want, you can also set different theme-folder path. For example:

``
hellwal -t ./themes/gruvbox.hellwal --theme-folder ~/dotfiles/configs/hellwal/
``

### Modes

You can select ``--dark`` or ``--light`` mode on every color palette, no matter if it's generated from image or from theme file. Remember that --dark is regular sorted array, and --light is just reversed so using this mode will not save you from flashbacks.

But --light is really cool, I recommend to check it out :)

### Scripts

With ``--script`` or ``-s`` flag you can run script(or any shell command) after hellwal. Note that it will only run if hellwal will not encounter any errors

### On a side note:

- if you want your new terminals to open with previusly specified theme, add ./templates/variables.sh ./templates/terminal.sh to your hellwal templates, then in .bash.rc add following lines:
```
source ~/.cache/hellwal/variables.sh
sh ~/.cache/hellwal/terminal.sh
```

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
