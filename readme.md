# ST. BRIELLE: THE INTERVAL

A first-person horror / immersive-sim prototype built on the Skin Deep / idTech 4 source foundation.

You are a municipal continuity inspector. The city says its facilities are functioning. The physical city disagrees with its own records. Your investigation decides which account survives.

## Start here

- [Game identity and campaign](stbrielle/design/GAME_BIBLE.md)
- [Current implementation, gaps, and next milestones](stbrielle/design/DEVELOPMENT_STATUS.md)
- [Playable slice and Windows playtest](stbrielle/design/VERTICAL_SLICE.md)
- [Game data and controls](stbrielle/README.md)

The current source contains the Municipal Annex, its first hostile encounter, an exterior investigation, and persistent choices preparing the St. Mercuria assignment. It is a development greybox, not a finished chapter or release build.

## Build and launch

1. Open `solution/monstergame.sln` in Visual Studio 2022 with the C++ desktop workload and v143 toolset.
2. Build the **stbrielle** project for **x64**, using the existing project configuration. The upstream layout assumes the source folder is named `neo`.
3. Run `run_stbrielle_annex.bat` from the repository root. It prepares the generated assets and runtime dependencies, compiles both maps, and launches the Annex.

The launcher expects `stbrielle.exe` in the repository root. No prebuilt executable is included. Changes to C++ require rebuilding it before launching.

Portable authored-map checks:

```text
python -m unittest discover -s tests -v
```

These tests exercise the street's Record-target graph and validate its structure; they do not compile the engine/maps or verify rendering, collision, audio, or native save/load.

## Upstream and license

This project is based on [Skin Deep](https://blendogames.com/skindeep), using [idTech 4](https://github.com/id-Software/DOOM-3) and [dhewm3](https://dhewm3.org). Skin Deep's programmers are Sanjay Madhav, Eric Itomura, Suzanne Will, and Brendon Chung. See its [full credits](https://blendogames.com/skindeep/credits.htm).

The source code is licensed under the [GNU General Public License](LICENSE). Skin Deep's commercial assets (art, models, textures, audio, levels, etc.) are not covered by that source release. Distributable St. Brielle content must be original or separately licensed. Preserve the existing upstream notices and third-party license files.
