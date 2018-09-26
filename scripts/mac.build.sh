curl -O https://archive.mozilla.org/pub/opus/opus-1.2.1.tar.gz
tar -zxf opus-1.1.2.tar.gz
./configure
make -j32
make install

mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3
