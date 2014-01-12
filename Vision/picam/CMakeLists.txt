cmake_minimum_required(VERSION 2.8)

SET(COMPILE_DEFINITIONS -Werror)
add_definitions(-std=c++11 -O2)

include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host)
include_directories(/opt/vc/include/interface/vmcs_host/linux)

include_directories(/home/pi/userland)
include_directories(/opt/vc/src/hello_pi/libs/vgfont)

link_directories(/opt/vc/lib)
link_directories(/opt/vc/src/hello_pi/libs/vgfont)

add_executable(vision PiCam.cpp GetImages.cpp Receive.cpp Send.cpp VisionServer.cpp)

find_package( OpenCV REQUIRED )

target_link_libraries(vision mmal_core mmal_util mmal_vc_client vcos bcm_host ${OpenCV_LIBS} openmaxil EGL)
