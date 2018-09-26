sudo apt --quiet -y install libopus-dev portaudio19-dev

mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3