apt update && apt -y install wget git g++ cmake qt5-default libboost-all-dev  libopus-dev portaudio19-dev
pwd
git clone https://github.com/zhang-ray/audio-processing-module
mkdir ../build-evc-linux
cd ../build-evc-linux
cmake ../easy-voice-call
make -j8
cd ../easy-voice-call
mv ../build-evc-linux .

