
brew install portaudio
brew install qt
brew install wget

# opus-1.1.2, same as Ubuntu 16.04
wget https://github.com/zhang-ray/opus/releases/download/macOS-v0.0.5/opus.macOS.tar.gz
tar -zxf opus.macOS.tar.gz
cd opus
make install
cd ..



mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3





####### make artifact
# TODO: try to auto find the macdeployqt path?
/usr/local/Cellar/qt/5.11.2/bin/macdeployqt client_qt5
cd client_qt5
mkdir Contents/MacOS
mv client_qt5 Contents/MacOS/EasyVoiceCall
rm -rf CMakeFiles
rm -rf client_qt5_autogen
rm -f  Makefile
rm -f  cmake_install.cmake
cd ../../easy-voice-call/scripts/appdmg/
mv ../../../easy-voice-call-build/client_qt5 EasyVoiceCall.app
cp Info.plist  EasyVoiceCall.app/Contents/
cp PkgInfo     EasyVoiceCall.app/Contents/
npm install -g appdmg
appdmg spec.json EasyVoiceCall.dmg
mv EasyVoiceCall.dmg ../../../