
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
cmake ../easy-voice-call || exit 1 
make -j3 || exit 1 





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

cp ../../misc/Icon.png Icon.png
mkdir MyIcon.iconset
sips -z 16 16     Icon.png --out MyIcon.iconset/icon_16x16.png
sips -z 32 32     Icon.png --out MyIcon.iconset/icon_16x16@2x.png
sips -z 32 32     Icon.png --out MyIcon.iconset/icon_32x32.png
sips -z 64 64     Icon.png --out MyIcon.iconset/icon_32x32@2x.png
sips -z 128 128   Icon.png --out MyIcon.iconset/icon_128x128.png
sips -z 256 256   Icon.png --out MyIcon.iconset/icon_128x128@2x.png
sips -z 256 256   Icon.png --out MyIcon.iconset/icon_256x256.png
sips -z 512 512   Icon.png --out MyIcon.iconset/icon_256x256@2x.png
sips -z 512 512   Icon.png --out MyIcon.iconset/icon_512x512.png
cp Icon.png MyIcon.iconset/icon_512x512@2x.png
iconutil -c icns MyIcon.iconset


mv ../../../easy-voice-call-build/client_qt5 EasyVoiceCall.app
cp Info.plist  EasyVoiceCall.app/Contents/
cp PkgInfo     EasyVoiceCall.app/Contents/
cp MyIcon.icns EasyVoiceCall.app/Contents/Resources/
npm install -g appdmg
appdmg spec.json EasyVoiceCall.dmg
mv EasyVoiceCall.dmg ../../../EasyVoiceCall.Client.macOS.dmg