# hellwal - fast, extensible colorscheme generator

- [ ] TODO: config ( is it really needed? )
- [ ] TODO: tweaking options for generated colors
- [ ] TODO: support for other OS's like Mac or Win 
- [ ] TODO: support for already built themes (like gruvbox etc.)
----------------------------------------------------------------
- [x] TODO: do more pleasant color schemes
- [x] TODO: print proper program usage
- [x] TODO: gen. colors
- [x] TODO: templating
- [x] TODO: parsing

# How to use?

Just run ``hellwal -i [image]`` with your image/wallpaper file, and you will get your colorscheme from templates in ~/.cache/hellwal/ or other specified directory.

Look up for templating examples in ./templates folder, they look more-less like this:
```
# Main
background='%%color|0|.hex%%'
foreground='%%color|15|.hex%%'

# Path
wallpaper_path='%%wallpaper_path|w|%%'

# Colors
color0='%%color|0|.hex%%'
color1='%%color|1|.hex%%'

# ... and so on and so fourth.

color15='%%color|15|.hex%%'
```

You can set specific color by writing number a from 0 to 15 between `|` character, and specify output format by writing it after `.` (eg. `.hex` in `colors|0|.hex')`

## Available color template formats:
- [x] ``.hex`` | ``ffffff``
- [x] ``.rgb`` | ``r, g, b``

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
