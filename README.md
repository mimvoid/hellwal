# hellwal - fast, extensible colorscheme generator

- [ ] TODO: config                            
- [ ] TODO: do more pleasant color schemes    
- [ ] TODO: tweaking options for color palette
----------------------------------------------
- [x] TODO: print proper program usage        
- [x] TODO: gen. colors                       
- [x] TODO: templating                        
- [x] TODO: parsing                           

# How to use?

Just run ``hellwal -i [image]`` with your image/wallpaper file, and you will get your colorscheme from templates in ~/.cache/hellwal/ or other specified directory.

Look up for templating examples in ./templates folder, they look more-less like this:
```
# Special
background='%%color|0|.hex%%'
foreground='%%color|15|.hex%%'

# Colors
color0='%%color|0|.hex%%'
color1='%%color|1|.hex%%'

# ... and so on and so fourth.

color15='%%color|15|.hex%%'
```

You can set specific color by writing number a from 0 to 15 between `|` character, and specify output format by writing it after `.` (eg. `.hex` in `colors|0|.hex')`

## Available color formats:
- [x] ``hex`` | ``ebcb96``
- [x] ``rgb`` | ``rgb(r, g, b)``


## hellwal --help

Usage:
	./hellwal [OPTIONS]
Options:
- Set **image file**
	- --image \<image\>
	- -i \<image\>

- Set the **template folder**
	- --template-folder \<folder\>
	- -f \<folder\>

- Set the **output folder** for **generated templates**
	- --output \<output\>
	- -o \<output\>

- Set the **template file**
	- --template <template\>
	- -t \<template\>

- Set the **output name** for a **single**, specified generated **template**
	- --output-name \<output\>
	- -n \<output\>

- Display **help** and exit
	- --help
	- -h

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
