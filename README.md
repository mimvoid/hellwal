# hellwal - fast, extensible colorscheme generator

[ ] todo: config                            
[ ] todo: do more pleasant color schemes    
[ ] todo: tweaking options for color palette
--------------------------------------------
[x] todo: print proper program usage        
[x] todo: gen. colors                       
[x] todo: templating                        
[x] todo: parsing                           

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
- ``hex`` | ``ebcb96``
- ``rgb`` | ``rgb(r, g, b)``


``hellwal --help``

Usage:
	./hellwal [OPTIONS]
Options:
  --image,           -i <image>     Set the image file.

  --template-folder, -f <folder>    Set the template folder.
  --output,          -o <output>    Set the output folder for generated templates

  --template,        -t <template>  Set the template file.
  --output-name,     -n <output>    Set the output name for single one, specified generated template

  --help,            -h             Display this help and exit.


Detailed: 
  --image: image path, which will be used to create color palette

  --template-folder: folder which contains templates to process
             to generate colors ; default one is ~/.config/hellwal/templates 

  --output: output folder where generated templates
               will be saved, default one is set to ~/.cache/hellwal/

  --template: you can specify single template input file which you
               want to generate ; This option does not collide with --template-folder

  --output-name: specify output of single template you provided
             it works only with option above: --template,
             if not set, default path will be used.

# Special thanks:
- [dylanaraps](https://github.com/dylanaraps) - for [https://github.com/dylanaraps/pywal](pywal) and other amazing stuff he created.
