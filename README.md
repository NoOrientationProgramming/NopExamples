
![GitHub](https://img.shields.io/github/license/NoOrientationProgramming/ProcessingExamples?style=plastic)
<!-- ![Lines of code](https://img.shields.io/tokei/lines/github/NoOrientationProgramming/ProcessingExamples?style=plastic) -->

[![Discord](https://img.shields.io/discord/960639692213190719?style=plastic&color=purple&logo=discord)](https://discord.gg/FBVKJTaY)
[![Twitch Status](https://img.shields.io/twitch/status/Naegolus?label=twitch.tv%2FNaegolus&logo=Twitch&logoColor=%2300ff00&style=plastic&color=purple)](https://twitch.tv/Naegolus)

Real-life examples based on the [SystemCore](https://github.com/NoOrientationProgramming/SystemCore#processing-start).

Status
- In progress

## Run the Examples

### Install the required Build Tools

On Debian, Ubuntu and derivatives execute the following with root permissions

`apt install build-essential`

If you want to use [meson and ninja](https://mesonbuild.com/) (recommended) add

`apt install meson ninja-build`

### Clone the example repository

`git clone https://github.com/NoOrientationProgramming/ProcessingExamples.git`

Initialize and Update the Submodules

Enter the new directory with `cd ProcessingExamples/` and execute

`git submodule update --init --recursive`

### Build and Run an Examples

Type
`./meson.sh t01_tcp-echo-server`
or just
`./meson.sh t01`

Then execute the application by entering

`./t01_tcp-echo-server/build-meson-ubuntu/app`

## Read the documentation

Every example will have its own README file for further information explaining what is going on.

## Add the Processing() class to your project

If you want to use the core in your own project then simply add the repository as a submodule like this

`
