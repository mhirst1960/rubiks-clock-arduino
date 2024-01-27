#!/bin/bash

#12hourclock-800x480/rubiks-clock-1228.mpg
vid=rubiks-clock-1228
start_time=00:00:00  # set to 00:00:01 if you want to cut off first second
duration_default=5       
duration_midnight=30   # clock at midnight takes longer to solve      
height=128    # height of TFT in pixels. 
width=128     # width of TFT in pixels. 
fps=25        # frames per a second.


#crop and scale 800x400 size mpg down to 
filters="fps=$fps,crop=iw-320:ih:120:0,scale=$width:$height:flags=lanczos"

# indir is directory of mpg files to read
indir=12hourclock-800x480-mpg-tmp/

# outdir is the directory to put all the new gifs
outdir=12hourclock-128x128-gif/

mkdir $outdir

for filename in $indir/*.mpg; do
    infile=$filename
    outfile=$outdir/$(basename "$filename" .mpg).gif
    #echo ffmpeg something "$filename" "12hourclock-128x128/$(basename "$filename" .mpg).gif"

   duration=$duration_default

   [[ $(basename "$filename" .mpg) == "rubiks-clock-0000.mpg" ]] || duration=$duration_midnight

    ffmpeg -ss $start_time                             \
       -t  $duration                               \
       -i  $infile                                  \
       -vf "$filters,palettegen"                   \
       -y palette.png                             &&
    ffmpeg -ss $start_time                             \
       -t  $duration                               \
       -i  $infile                                 \
       -i  palette.png                                \
       -lavfi "$filters [x]; [x][1:v] paletteuse"  \
       -y  $outfile                               &&
    rm palette.png 

done
