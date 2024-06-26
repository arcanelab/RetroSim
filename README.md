_(Please don't submit this repo to Hacker News yet, as it's heavily work-in-progress and not ready for the public spotlight just yet. Thank you!)_

# About

RetroSim is a modern fantasy retro-computer. 

What's a fantasy computer?

A fantasy computer (or fantasy console) is a software platform that emulates the limitations and characteristics of older computer hardware and video game consoles. Unlike genuine emulators, which aim to faithfully reproduce the functionality of historical hardware, fantasy consoles are not based on real-world devices. Instead, they are crafted to evoke the spirit and constraints of retro gaming and computing eras.

How does RetroSim compare to popular fantasy platforms like PICO-8 and TIC-80?

While we all know that constraints have a positive effect on creativity, there are occasions when the limitations of just 16 colors or a 128x128 pixel canvas can feel slightly too restrictive. In addition to having more colors (256) and higher resolution (464x256), RetroSim introduces some exciting new features. It supports high-level scripting languages like Lua and Gravity and also offers the ability to use assembly language for CPUs such as the 6502, 65816, and A65000.

For detailed specifications, check out the [Wiki](https://github.com/arcanelab/RetroSim/wiki).

# Building

RetroSim is written in C++ using SDL. [Xmake](https://xmake.io/) is used to manage the dependencies and to build the project.

## 3rd-party libraries
* [SDL](http://www.libsdl.org/)
* [Gravity](https://marcobambini.github.io/gravity/)
* [libretro](https://www.libretro.com/)
* [libtelnet](http://github.com/seanmiddleditch/libtelnet)
* [sdl-gpu](https://github.com/grimfang4/sdl-gpu)
* [CSID-mod](https://github.com/possan/csid-mod)
* [raylib](https://www.raylib.com/)
* [ImGui](https://github.com/ocornut/imgui)
* [rlImGui](https://github.com/raylib-extras/rlImGui)

# License

RetroSim is not free software at the moment. I'm planning to release it under a permissive licence at some later point, once the project reaches a mature point. The source code can be read and studied, but you are not allowed to copy, distribute or release it in source or other forms. Thank you for respecting these terms until I decide to release it with a permissive license.

# Credits

* Written by Zoltán Majoros ([GitHub](https://github.com/arcanelab)) ([Twitter](https://twitter.com/arcanelab))
* See /src/extern for contributors of 3rd party dependencies.
