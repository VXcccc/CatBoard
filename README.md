<h1 align="center">
  <br>
  <a>
  <img src="Resource/icon.ico" alt="icon" width="200">
</a>
  <br>
  <b>Catboard</b>
  <br>
</h1>

<h4 align="center"><b>CatBoard is the next-gen input overlay written in C++ via ImGui, that rendered directly on your screen</b>
</h4>

<div align="center">

[![Release](https://img.shields.io/github/v/release/VXcccc/CatBoard?style=flat-square&color=5BCEFA)](https://github.com/VXcccc/CatBoard/releases)
[![Downloads](https://img.shields.io/github/downloads/VXcccc/CatBoard/total?label=downloads&style=flat-square&color=F5A9B8)](https://github.com/VXcccc/CatBoard/releases)
[![License](https://img.shields.io/github/license/VXcccc/CatBoard?style=flat-square&color=FFFFFF)](https://github.com/VXcccc/CatBoard/blob/main/LICENSE)

</div>

<p align="center">
  <img src="https://femboy.beauty/lRnUYD", width="740">
</p>

The main idea behind catboard is to share your inputs without OBS, so you can screenshare them on discord or see what are you pressing by yourself


> [!NOTE]
> This is my first C++ project, so bugs, problems with optimization and bad code expected. Im gonna try my best at maintaining and improving this project. Also fill free to contribute I appreciate any contribution, check [CONTRIBUTING.md](https://github.com/VXcccc/CatBoard/blob/main/CONTRIBUTING.md)  

## **My major inspirations**
Some features and styles are heavily inspired by the projects below **(check them out!)**:
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
- **Image packs** if you want to configure them check how preinstalled image packs [config.json](/Assets/Images/DefaultImagePack/config.json) work.
- **Cross-platform** in development

<details>
<summary><b>Layout editor shortcuts</b></summary>

| Shortcut | Action |
|----------|--------|
| `Ctrl+R` | Record key |
| `Ctrl+C` / `Ctrl+V` | Copy / Paste |
| `Ctrl+Z` | Undo |
| `Ctrl+Shift+Z` | Redo |
| `Delete` | Delete selected |
| `Mouse3` | Pan canvas |

</details>


## **Installation**

> [!WARNING]
> Some evil strict anticheats may flag this application because they dont like overlays. Use it at your own risk!

There are multiple ways to install CatBoard

1. **Using installer**
    - Download and run the latest [EXE installer](https://github.com/VXcccc/CatBoard/releases/latest).

3. **Portable**
   - Download and extract the [portable ZIP file](https://github.com/VXcccc/CatBoard/releases/latest)


## **Fonts**
If a font specified in the config is missing, an error message will appear. Click the error message to copy it, then find and install the font manually.

Fonts used in default configs:

- [JetBrainsMonoNerd](https://github.com/ryanoasis/nerd-fonts/releases/download/v3.5.1/JetBrainsMono.zip)
- [Lilita-one](https://fonts.google.com/specimen/Lilita+One)
- [The wild breath of zelda](https://www.dafont.com/the-wild-breath-of-zelda.font)


## **Building from scratch**
### Requirements:
`Windows 10/11`,`CMake`,`GCC`,`Ninja`,`vcpkg`.  
Also dont forget to add vcpkg to $env `setx VCPKG_ROOT "Your vcpkg path"`, or just simply change CMakePresets vcpkg path to the absolute path.

1. **Clone the repo**
```
git clone --recursive https://github.com/VXcccc/CatBoard.git
```

2. **Build via CMake**
<details>
<summary><b>debug build</b></summary>

```bash
cmake --preset mingw-debug
cmake --build --preset mingw-debug
```
</details>

<details>
<summary><b>release build</b></summary>

```bash
cmake --preset mingw-release
cmake --build --preset mingw-release
```
</details>

<details>
<summary><b>pack (optional)</b></summary>

Installer:

```bash
cmake --preset mingw-release
cmake --build --preset mingw-release
cpack --preset installer
```

Portable:

```bash
cmake --preset mingw-release-portable
cmake --build --preset mingw-release-portable
cpack --preset portable-zip
```

> Installer and portable are separate builds (`CATBOARD_PORTABLE` is set at compile time),
> so each needs its own build before packing.

</details>
