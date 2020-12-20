
Shrink a video to a certain width

```
function shrink_vid() {
    ffmpeg -i "$1" -c:v mjpeg -vf "scale='min(960,iw)':-1" -qscale:v 1 -vendor ap10 -pix_fmt yuvj422p "$1-SHRINK.mov"
}
```


```
find assets/ -type f |  perl -nle '$line++; print q(Video("$_", {"mirror"}),)'
```

```
ffmpeg -i gif_me.mkv -vf fps=10 frames/ffout%03d.png
convert -loop 0 frames/ffout*.png out.gif
```

Converting to limit keyframes for chopping (usally second argument ends with the extenton .mov)
```
function make-choppable() {
    ffmpeg -i "$1" -c:v mjpeg -qscale:v 1 -vendor ap10 -pix_fmt yuvj422p "$2"
}

function org-chopable() {
    name="${1%.*}"
    ext="${1##*.}"
    mkdir -p "$name"
    ffmpeg -i "$name.$ext" -c:v mjpeg -qscale:v 1 -vendor ap10 -pix_fmt yuvj422p "$name/full.mov"
}
```

Lengths: 
```
function vid-lengths() {
    for f in $@
    do
      ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 "$f" | xargs echo -n
      echo " $f"
    done
}
```

Removing black bars
https://superuser.com/questions/810471/remove-mp4-video-top-and-bottom-black-bars-using-ffmpeg

Recording (this is REALLY wonky formatting, but it works good with youtube)
```
ffmpeg -thread_queue_size 1028 -f x11grab -video_size 1920x1080 -framerate 30 -i :0.0 -thread_queue_size 1028 -f pulse -i default -c:v mjpeg -qscale:v 1 -vendor ap10 -pix_fmt yuvj422p -r 30 -c:a aac take-3.mov
```


Instagram video encoding settings
```
    -c:v libx264 \
    -preset slow \
    -crf 18 \
    -profile:v baseline \
    -level 3.0 \
    -movflags +faststart \
    -pix_fmt yuv420p
```

e.g.
```
ffmpeg -i foo.mkv -c:v libx264 -preset slow -crf 18 -profile:v baseline -level 3.0 -movflags +faststart -pix_fmt yuv420p out.mp4
```

Fonfiguring ffmpeg
```
./configure --enable-nvenc --enable-nvdec --enable-libsnappy --enable-ffplay --enable-gpl --enable-libass    --enable-libfdk-aac --enable-libmp3lame --enable-libopus --enable-libtheora   --enable-libvorbis --enable-libvpx --enable-libx264 --enable-nonfree
```


```
convert -size 1920x -background black -fill white  -gravity center label:"Atmosphere is important" assets/text-test.png
```

Hap encoding several videos
```
find -name "*.mkv" -exec ffmpeg -i {} -c:v hap {}.hap.mov
```

Rendering controller template ERB

```
ruby -rerb -e 'puts ERB.new(File.read("workspace/controllers/fighter_twister.yml.erb")).result'
```


Capturing portions of the screen as video playback device. Replace the -video\_size and -i with output of slop, also scale=

```
slop -f '-video_size %wx%h -i +%x,%y'
ffmpeg -f x11grab -r 25 -video_size 664x398 -i +1692,1183 -vcodec rawvideo -pix_fmt yuv420p -threads 0 -f v4l2 -vf 'scale=664x398' /dev/video4
```
