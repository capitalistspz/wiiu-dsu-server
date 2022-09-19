mkdir build
cd build || exit
"$DEVKITPRO"/portlibs/wiiu/bin/powerpc-eabi-cmake ../
make