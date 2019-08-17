```
find ~/vj/assets/ -name "*.mkv" | perl -nle '$line++; print "vid$line:\n    type: video\n    path: $_\n"'
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

Recording
```
ffmpeg -thread_queue_size 1028 -f x11grab -video_size 1920x1080 -framerate 30 -i :0.0 -thread_queue_size 1028 -f pulse -i default -c:v mjpeg -qscale:v 1 -vendor ap10 -pix_fmt yuvj422p -r 30 -c:a aac take-3.mov
```
