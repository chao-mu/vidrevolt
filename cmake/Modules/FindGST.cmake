find_package(PkgConfig) 

pkg_check_modules(GST REQUIRED gstreamer-1.0>=1.4 
    gstreamer-sdp-1.0>=1.4 
    gstreamer-video-1.0>=1.4 
    gstreamer-app-1.0>=1.4) 
