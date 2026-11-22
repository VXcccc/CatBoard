<h1 align="center">
  <br>
  <a>
  <img src="Resource/icon.ico" alt="icon" width="200">
</a>
  <br>
  <b>Catboard</b>
  <br>
</h1>

<h4 align="center"><b>CatBoard is the next-gen input overlay written in C++ via ImGui, that rendered directly on your screen.</b>
</h4>

The main idea behind it is to share your inputs without OBS, so you can screenshare them on discord or see what you are pressing by yourself.


> [!NOTE]
> This is my first C++ project, so bugs and bad code expected. Im gonna try my best at maintaining this project. Also fill free to contribute I appreciate any contribution, check [CONTRIBUTING.md](https://github.com/VXcccc/CatBoard/blob/main/CONTRIBUTING.md)  

## **My major inspirations**
Some features are heavily inspired by the projects below **(check them out!)**:
- [girlglock input-overlay](https://github.com/girlglock/input-overlay/)
- [ThoNohT NohBoard](https://github.com/ThoNohT/NohBoard)
- [conventoangelo OverKeys](https://github.com/conventoangelo/OverKeys)


## **Features**
- **Customizable everything:** you can customize almost everything in this overlay (layout, hotkeys, style, font, image packs, sound packs etc.)
- **Analog keyboard support** via the [Wooting Analog SDK](https://github.com/WootingKb/wooting-analog-sdk) Right now only wooting keyboards are supported, but I may add support for other keyboards in the future
- **Mouse tracking** using the Windows Raw Input API
- **Asmr sounds** working via [Mechvbies](https://github.com/hainguyents13/mechvibes) sound packs, you can [create your own](https://editor.mechvibes.com/) or [browse existing](https://mechvibes.com/sound-packs)
- **OBS mode** is similliar to [NohBoard](https://github.com/ThoNohT/NohBoard) obs mode (simply adds a background to the application). In future, if possible, im going to try other methods of bringing my overlay into the OBS
- **Auto hide** automatically hide your overlay when you inactive
- **Cross-platform** in development


## **Installation**
CatBoard can be installed through several methods

## **Fonts**
If a font specified in the config is missing, an error message will appear. Click the error message to copy it, then find and install the font manually.

Fonts used in default configs:

- [JetBrainsMonoNerd](https://github.com/ryanoasis/nerd-fonts/releases/download/v3.5.1/JetBrainsMono.zip)
- [Lilita-one](https://fonts.google.com/specimen/Lilita+One)
- [The wild breath of zelda](https://www.dafont.com/the-wild-breath-of-zelda.font)


## **Build guide**
### Requirements:
`Windows 10/11`,`CMake`,`GCC`,`Ninja`,`vcpkg`.  
Also dont forget to add vcpkg to $env `setx VCPKG_ROOT "Your vcpkg path"`, or just simply change CMakePresets vcpkg path to the absolute path.

1. **Clone the repo**
```
git clone --recursive https://github.com/<user>/CatBoard.git
cd CatBoard
```

2. **Build via CMake**
- Debug:
```
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```
- Release:
```
cmake --preset mingw-release
cmake --build --preset mingw-release
```
- Pack(optional):
```
cpack --preset portable-zip
cpack --preset installer
```
