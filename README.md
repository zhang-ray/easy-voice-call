![server_and_clients](./documents/user/screenshot.png)


<p align="center">
  <a href="LICENSE"><img alt="License" src="https://img.shields.io/github/license/zhang-ray/easy-voice-call.svg"></a>
  <a ><img alt="HitCount" src="https://hitcounter.pythonanywhere.com/count/tag.svg?url=https%3A%2F%2Fgithub.com%2Fzhang-ray%2Feasy-voice-call"></a>
  <a href="https://github.com/zhang-ray/easy-voice-call/pulls"><img alt="PRs Welcome" src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square"></a>
  <a href="https://github.com/zhang-ray/easy-voice-call/commits/master"><img alt="Commits" src="https://img.shields.io/github/commits-since/zhang-ray/easy-voice-call/latest.svg?style=flat-square"></a>
</p>

<h1 align="center">
  EVC(easy-voice-call) is a self-hosting voice chating App. 
</h1>


## User Guide
EVC is very **easy to use**
- Deploy the server
  - Download latest *EasyVoiceCall.Server.Linux-x86_64.AppImage* from https://github.com/zhang-ray/easy-voice-call/releases
  - `chmod +x EasyVoiceCall.Server.Linux-x86_64.AppImage && ./EasyVoiceCall.Server.Linux-x86_64.AppImage 1222 # 1222 is the UDP port`
- Use the client
  - Download latest *EasyVoiceCall.Client.macOS.dmg* or *EasyVoiceCall.Client.Windows.Installer* from https://github.com/zhang-ray/easy-voice-call/releases
  - Make sure your recording/playback device are ready, and then open the client!
  - Just connect to the coresponding server!
  - Waiting for the other if you're the fisrt one online and vice versa.



## License
[LICENSE](./LICENSE). By the way, [IANAL](https://en.wikipedia.org/wiki/IANAL).



## Technical details
**Technical features**
- [x] private and secure - the server is self-hosting;
- [x] cross platform (Windows and macOS) and easy-to-use GUI client;
- [x] Voice Activity Detection; 
- [x] Acoustic Echo Cancellation; 
- [x] Noise Suppression;
- [x] Comfort Noise Generator;
- [x] concise source code - less than 10k lines (including clients, server, unit_tests and smoke_tests / sanity_check code);

