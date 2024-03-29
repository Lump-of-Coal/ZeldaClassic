 # Zelda Classic

Zelda Classic is an engine for making games similar to the original Famicom/NES '_Legend of Zelda_', ('_Zelda no Densetsu_', 「ゼルダの伝説」) title from 1986, but can be used to create anything from that, to a wholly custom game engine, much in the same way that commercial software such as **PICO-8** or **Gamemaker** allows you to construct a game of your _own devices_, but far more easily: The packages include the custom, C, C, Pascal, and Ruby-inspired, powerful scripting language: **ZScript**, that allows you to directly modify the game engine to the limits of your imagination, creativity, and programming expertise. 

For most basic game creation, all tasks are handled by the _JWin _user interface in graphical, instantaneous, and easily editable means. All items, power-ups, enemies, and other facets of a game are something that_ you can designate and customise using the base editor alone!_ Should you want more, the ZScript interface language, with the included tools and compiler, allows you to modify some of the deepest innerworkings of the engine, to tailor it to your specific needs.

Originally created in 1999 by Jeremy (a.k.a. Phantom Menace) Craner as an experiment in C-, Zelda Classic evolved into a much beloved game engine that allows its users to create custom NES/Famicom to SNES/SFC style action RPG games, on par with many released commercially. Now it is open source, updated for modern hardware, and you too can now benefit from it, creating anything from a hobbyist bit of fun, to a **fully commercial** title!

To learn more about Zelda Classic, or to _download_ _**over 23 years of existing 'Quests'**_ (our term for custom games), please vitit our website at:: [zeldaclassic.com](https://zeldaclassic.com)

To get the **latest release versions and builds** of our software, check out the Releases section on GitHub, or visit us at:
[Latest Downloads](https://www.zeldaclassic.com/downloads/)

If you are on _Discord_ and wish to chat with other users, developers, and staff, you can join us here:
[Official ZC Discord](https://discord.gg/VQwSs3DAjM)

We support Windows (Win7+), OSX (12+), and Linux. <!--There is also an experimental [web version](https://zquestclassic.com/play/) that can also be played on mobile devices.--!>

## Development

For information on contributing to this project, please read: [`docs/building.md`](./docs/building.md).

Zelda Classic builds _Allegro 5_ from source. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to translate into Allegro 5.

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`)

Logs are written to `allegro.log`.

We have dozens of replay tests (`tests/replays/*.zplay`), which are text files that have recorded inputs of actual gameplay. To avoid regressions, we play this input back to the game engine, and in CI make sure that every single frame is drawn as expected. This command will run a single replay:

```sh
python3 tests/run_replay_tests.py --filter tests/replays/classic_1st.zplay
```

See [`docs/replays.md`](./docs/replays.md) for more.

## Contributing to Zelda Classic

We encourage third-party submission of patches and new features! If you're interesting in contributing to Zelda Classic's development, please read the CONTRIBUTE file.

## License and Disclaimer

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License v3, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

## Special Thanks
**This resurrected and reconstructed repo was constructed by _THE TIMELORD_ under the agreement that this message, and the below verses of the Tanakh _Ketuvim_, regarding the theft of the original AGN ZC repo are never to be removed or obfuscated from this readme.md!**

**Proverbs 21:2** Every way of a man is right in his own eyes; but the LORD weigheth the hearts.

**Proverbs 21:6** The getting of treasures by a lying tongue is a vapour driven to and fro; they seek death.

**Proverbs 21:10** The soul of the wicked desireth evil; his neighbour findeth no favour in his eyes.

**Proverbs 21:10** When the scorner is punished, the thoughtless is made wise; and when the wise is instructed, he receiveth knowledge.

**Proverbs 21:15** To do justly is joy to the righteous, but ruin to the workers of iniquity.

**Proverbs 21:21** He that followeth after righteousness and mercy findeth life, prosperity, and honour.

**Proverbs 21:24** A proud and haughty man, scorner is his name, even he that dealeth in overbearing pride.

**Proverbs 21:26** There is that coveteth greedily all the day long; but the righteous giveth and spareth not.

**Proverbs 21:28** A false witness shall perish; but the man that obeyeth shall speak unchallenged.
