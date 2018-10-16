sudo apt-get update
sudo apt --quiet -y install libopus-dev portaudio19-dev libboost-all-dev qtbase5-dev

mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call
make -j3


###################### prepare AppImage ###################### 
wget "https://github.com/AppImage/AppImageKit/releases/download/10/AppRun-x86_64" #  -O AppDir/AppRun 
wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
wget "https://github.com/AppImage/AppImageKit/releases/download/10/appimagetool-x86_64.AppImage"
chmod a+x AppRun-x86_64
chmod a+x linuxdeploy-x86_64.AppImage
chmod a+x appimagetool-x86_64.AppImage


###################### make GUI client AppImage ###################### 
cd ./client_qt5/
mkdir AppDir
mkdir AppDir/usr/
mkdir AppDir/usr/bin/
cp ../AppRun-x86_64 AppDir/AppRun
cp ./client_qt5 AppDir/usr/bin/EasyVoiceCall.qt5
echo -e "[Desktop Entry]\nName=EasyVoiceCall.qt5\nExec=EasyVoiceCall.qt5\nIcon=EasyVoiceCall.qt5\nType=Application" > AppDir/EasyVoiceCall.qt5.desktop
convert -size 256x256 xc:transparent AppDir/EasyVoiceCall.qt5.png
../linuxdeploy-x86_64.AppImage --appdir=AppDir
# try to fix:
# ``` This application failed to start because it could not find or load the Qt platform plugin "xcb".
mkdir AppDir/usr/lib/x86_64-linux-gnu
mkdir AppDir/usr/lib/x86_64-linux-gnu/qt5
mkdir AppDir/usr/lib/x86_64-linux-gnu/qt5/plugins
cp -r /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms AppDir/usr/lib/x86_64-linux-gnu/qt5/plugins
../appimagetool-x86_64.AppImage AppDir
mv EasyVoiceCall.qt5-x86_64.AppImage ../../
cd ..


###################### make server AppImage ###################### 
cd ./server/
mkdir AppDir
mkdir AppDir/usr/
mkdir AppDir/usr/bin/
cp ../AppRun-x86_64 AppDir/AppRun
cp ./server AppDir/usr/bin/EasyVoiceCall.Server
echo -e "[Desktop Entry]\nName=EasyVoiceCall.Server\nExec=EasyVoiceCall.Server\nIcon=EasyVoiceCall.Server\nType=Application" > AppDir/EasyVoiceCall.Server.desktop
convert -size 256x256 xc:transparent AppDir/EasyVoiceCall.Server.png
../linuxdeploy-x86_64.AppImage --appdir=AppDir
../appimagetool-x86_64.AppImage AppDir
mv EasyVoiceCall.Server-x86_64.AppImage ../../