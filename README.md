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

## How to use?

Just run ``hellwal -i [image]`` with your image/wallpaper file, and you will get your colorscheme from templates in ~/.cache/hellwal/ or other specified directory.
You can also run with ``-i <folder> -r`` - it will randomly pick image from directory.

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

You can set specific color by writing number a from 0 to 15, and output format by writing '.' after (eg. `.hex` in `colors0.hex` or `colors0.rgb`)

### Available color template formats:
- [x] ``.hex`` | ``ffffff``
- [x] ``.rgb`` | ``r, g, b``

## Themes
You can set your own theme, re-run it anytime and apply to your config or other programs!

For example - gruvbox theme:

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

I recommend to put all themes to ~/.config/hellwal/themes folder, because from there you can just provide theme name, and it will pick it up automatically, without specifying path. Of course if you want, you can also set different theme-folder path. For example:

``
hellwal -t ./themes/gruvbox.hellwal --theme-folder ~/dotfiles/configs/hellwal/
``

### Modes

You can select ``--dark`` or ``--light`` mode on every color palette, no matter if it's generated from image or from theme file. Remember that --dark is regular sorted array, and --light is just reversed so using this mode will not save you from flashbacks.

But --light is really cool, I recommend to check it out :)

### Scripts

With ``--script`` or ``-s`` flag you can run script(or any shell command) after hellwal. Note that it will only run if hellwal will not encounter any errors

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
