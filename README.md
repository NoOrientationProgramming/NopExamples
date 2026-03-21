
<h2 id="frap-examples-start" style="display:none;"></h2>

![GitHub](https://img.shields.io/github/license/fractal-programming/FrapExamples?style=plastic)

![Windows](https://img.shields.io/github/actions/workflow/status/fractal-programming/FrapExamples/windows.yml?style=plastic&logo=github&label=Windows)
![Linux](https://img.shields.io/github/actions/workflow/status/fractal-programming/FrapExamples/linux.yml?style=plastic&logo=linux&logoColor=white&label=Linux)
![macOS](https://img.shields.io/github/actions/workflow/status/fractal-programming/FrapExamples/macos.yml?style=plastic&logo=apple&label=macOS)
![FreeBSD](https://img.shields.io/github/actions/workflow/status/fractal-programming/FrapExamples/freebsd.yml?style=plastic&logo=freebsd&label=FreeBSD)
![ARM, RISC-V & MinGW](https://img.shields.io/github/actions/workflow/status/fractal-programming/FrapExamples/cross.yml?style=plastic&logo=gnu&label=ARM%2C%20RISC-V%20%26%20MinGW)

[![Discord](https://img.shields.io/discord/960639692213190719?style=plastic&color=purple&logo=discord)](https://discord.gg/FBVKJTaY)
[![Twitch Status](https://img.shields.io/twitch/status/Naegolus?label=twitch.tv%2FNaegolus&logo=Twitch&logoColor=%2300ff00&style=plastic&color=purple)](https://twitch.tv/Naegolus)

Real-life examples based on the [SystemCore](https://github.com/fractal-programming/SystemCore#system-core-start).

Built with **Fractal System Architecture (FSA)**

Learn more: https://github.com/fractal-programming

Status
- In progress

## Run the Examples

### Supported Build Tools

- [Meson](https://mesonbuild.com/)
- [CMake](https://cmake.org/)
- [Make](https://www.gnu.org/software/make/)

### Clone the example repository

`git clone https://github.com/fractal-programming/FrapExamples.git --recursive`

### Build and Run an Examples

#### Go into the example directory and create a build folder
```
cd t01_tcp-echo-server
mkdir -p build
cd build
```

#### Build the application with your preferred build tool

**Meson**
```
meson setup . ..
meson compile
```

**CMake**
```
cmake ..
make -j
```

**Make**
```
make -f ../Makefile -j
```

#### Then execute the application by entering
`./app`

## Read the documentation

Every example will have its own README file for further information explaining what is going on.

## Add the SystemCore to your project

If you want to use the core in your own project then simply add the repository as a submodule like this

`git submodule add https://github.com/fractal-programming/SystemCore.git`

