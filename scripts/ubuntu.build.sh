sudo apt --quiet -y install libopus-dev portaudio19-dev

mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3


###################### make AppImage ###################### 
pwd
mkdir AppDir
mkdir AppDir/usr/
mkdir AppDir/usr/bin/
wget "https://github.com/AppImage/AppImageKit/releases/download/10/AppRun-x86_64" -O AppDir/AppRun 
chmod a+x AppDir/AppRun
cp ./client_qt5/client_qt5 AppDir/usr/bin/EasyVoiceCall.qt5
echo -e "[Desktop Entry]\nName=EasyVoiceCall.qt5\nExec=EasyVoiceCall.qt5\nIcon=EasyVoiceCall.qt5\nType=Application" > AppDir/EasyVoiceCall.qt5.desktop
convert -size 256x256 xc:transparent AppDir/EasyVoiceCall.qt5.png
wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod a+x linuxdeploy-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --appdir=AppDir
wget "https://github.com/AppImage/AppImageKit/releases/download/10/appimagetool-x86_64.AppImage"
chmod a+x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage AppDir
mv EasyVoiceCall.qt5-x86_64.AppImage ../