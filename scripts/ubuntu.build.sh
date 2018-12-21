sudo apt-get update || exit 1 
sudo apt --quiet -y install libopus-dev portaudio19-dev libboost-all-dev qtbase5-dev || exit 1 

mkdir ../easy-voice-call-build
cd ../easy-voice-call-build
cmake ../easy-voice-call || exit 1 
make -j3 || exit 1


###################### prepare AppImage ###################### 
wget "https://github.com/AppImage/AppImageKit/releases/download/10/AppRun-x86_64" || exit 1 
wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" || exit 1 
wget "https://github.com/AppImage/AppImageKit/releases/download/10/appimagetool-x86_64.AppImage" || exit 1 
chmod a+x AppRun-x86_64
chmod a+x linuxdeploy-x86_64.AppImage
chmod a+x appimagetool-x86_64.AppImage



###################### make server AppImage ###################### 
cd ./server/
mkdir AppDir
mkdir AppDir/usr/
mkdir AppDir/usr/bin/
cp ../AppRun-x86_64 AppDir/AppRun
cp ./server AppDir/usr/bin/EasyVoiceCall.Server.Linux
echo -e "[Desktop Entry]\nName=EasyVoiceCall.Server.Linux\nExec=EasyVoiceCall.Server.Linux\nIcon=EasyVoiceCall.Server.Linux\nType=Application\nCategories=AudioVideo;Audio;" > AppDir/EasyVoiceCall.Server.Linux.desktop
convert -size 256x256 xc:transparent AppDir/EasyVoiceCall.Server.Linux.png
../linuxdeploy-x86_64.AppImage --appdir=AppDir || exit 1 
../appimagetool-x86_64.AppImage AppDir || exit 1 
cp EasyVoiceCall.Server.Linux-x86_64.AppImage ../../




############  sanity check  ############
chmod +x EasyVoiceCall.Server.Linux-x86_64.AppImage
(./EasyVoiceCall.Server.Linux-x86_64.AppImage 1222 echo &) || exit 1
