#!/usr/bin/env sh

# Set up wallpaper folder
wallpaper_path=$HOME/pics/wallpapers

# Run hellwal to generate templates and get color palette
./hellwal --image $wallpaper_path --random

# Optionally you can load theme
#./hellwal -r -k ./themes/

# Obtain variables (color values, wallpaper path)
source ~/.cache/hellwal/variables.sh

# Run wallpaper daemon
swww img $wallpaper \
  --transition-type="grow" \
  --transition-duration 2 \
  --transition-fps 165 \
  --resize="crop" \
  --invert-y

# Update discord colors
themecord -p ~/.cache/hellwal/discord-colors.css

# Relaunch waybar to apply colors
    #wbar-reload

# Update firefox css
    #pywalfox update
