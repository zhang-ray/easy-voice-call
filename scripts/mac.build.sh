
# opus-1.1.2, same as Ubuntu 16.04
pwd
which curl
curl --version
curl -O https://github.com/xiph/opus/releases/download/v1.1.2/opus-1.1.2.tar.gz
tar -zxf opus-1.1.2.tar.gz
cd opus-1.1.2
./configure
make -j32
make install
cd ..


# portaudio
brew install portaudio


mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3
