# ZQuest Classic

ZQuest Classic is an engine for making games similar to the original NES Zelda.

Website: [zquestclassic.com](https://zquestclassic.com)

[Latest Downloads](https://zquestclassic.com/releases/)

[Our Discord](https://discord.gg/ddk2dk4guq)

We support Windows (Win7+), OSX (12+), and Linux. There is also an experimental [web version](https://zquestclassic.com/play/) that can also be played on mobile devices.

## Development

See [`docs/building.md`](./docs/building.md).

ZQuest Classic builds Allegro 5 from source. The code base still uses Allegro 4 APIs, but uses [Allegro-Legacy](https://github.com/NewCreature/Allegro-Legacy) to translate into Allegro 5.

The compiled binaries require a number of resource files to be available at runtime. By default, building the project will copy those files to the correct location. They will only be copied if they do not already exist–so you can modify these files (like the various `.cfg` files) in your build directory to your liking. To completely refresh the resources file, build the `copy_resources` target (ex: `cmake --build build -t copy_resources`)

Logs are written to `allegro.log`.

We have dozens of replay tests (`tests/replays/*.zplay`), which are text files that have recorded inputs of actual gameplay. To avoid regressions, we play this input back to the game engine, and in CI make sure that every single frame is drawn as expected. This command will run a single replay:

```sh
python3 tests/run_replay_tests.py --filter tests/replays/classic_1st.zplay
```

See [`docs/replays.md`](./docs/replays.md) for more.

## Contributing to ZQuest Classic

We encourage third-party submission of patches and new features! If you're interesting in contributing to ZQuest Classic's development, please read the CONTRIBUTE file.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

**This resurrected and reconstructed repo was constructed by THE TIMELORD under the agreement that this message and the below verses of the Torah, regarding the theft of the original AGN ZC repo are never to be removed or obfuscated from this readme.md!**

**Proverbs 21:2** Every way of a man is right in his own eyes; but the LORD weigheth the hearts.

**Proverbs 21:6** The getting of treasures by a lying tongue is a vapour driven to and fro; they seek death.

**Proverbs 21:10** The soul of the wicked desireth evil; his neighbour findeth no favour in his eyes.

**Proverbs 21:10** When the scorner is punished, the thoughtless is made wise; and when the wise is instructed, he receiveth knowledge.

**Proverbs 21:15** To do justly is joy to the righteous, but ruin to the workers of iniquity.

**Proverbs 21:21** He that followeth after righteousness and mercy findeth life, prosperity, and honour.
**Proverbs 21:24** A proud and haughty man, scorner is his name, even he that dealeth in overbearing pride.
**Proverbs 21:26** There is that coveteth greedily all the day long; but the righteous giveth and spareth not.
**Proverbs 21:28** A false witness shall perish; but the man that obeyeth shall speak unchallenged.
