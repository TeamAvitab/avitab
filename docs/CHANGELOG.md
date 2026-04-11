# Avitab Changelog

## Avitab v1.0.0 (April 2026)

### Summary

This is the first update to Avitab overseen by [TeamAvitab](https://github.com/TeamAvitab) after [Folke Will](https://github.com/fpw) stepped back from full-time maintenance. Many thanks to Folke for his original authorship and ongoing development of the project, and for his support in transferring the project to TeamAvitab.

This release has been given the label v1.0.0 to mark the handover, and to indicate that no major new features are considered necessary for basic usability. (In practice, Avitab has been in this state for many previous releases, but the major version never got the bump it perhaps should have had.)

Much of the work done since TeamAvitab adopted the project has been under-the-hood, updating the project structure and build scripts, and incorporating up-to-date versions of many of the third-party libraries.

### Bug fixes

Avitab's support for PDF and other documents is implemented by the MuPDF library, and this has now been updated to a much more recent version which resolves simulation crashes that had been [reported](https://github.com/fpw/avitab/issues/232) in Avitab v0.7.1. Thanks to [tzimpel](https://github.com/tzimpel) for his work in identifying the cause and prototyping a solution.

### Enhancements

[sfrsfrsfr](https://github.com/sfrsfrsfr) submitted a number of enhancements to the previous project archive which have been integrated into the new Avitab project:

- Use TCAS datarefs for other aircraft. This resolves an unrecorded issue where other aircraft were not displayed when connected to VATSIM (and probably other multi-player systems). [See the PR](https://github.com/fpw/avitab/pull/218)

- A `Nearest` button is added to the Airports app. [See the PR](https://github.com/fpw/avitab/pull/220)

- Additional runway surface types are now supported when loading the NAV data. [See the PR](https://github.com/fpw/avitab/pull/222)

- The header clock can be clicked to cycle through four modes: real world clock, sim zulu time, sim local time, stopwatch. [See the PR](https://github.com/fpw/avitab/pull/223).
    - Real world clock is the local time on your PC. It is annotated `{hh:mm}`
    - Simulation zulu time is annotated with a `z` suffix.
    - Simulation local time has no further annotation.
    - The stopwatch is annotated with a `+` prefix. It is reset by long-clicking for at least 2 seconds. A short-click will cycle to the next clock mode and the stopwatch will continue counting in the background.

- Improvements to the airport search: FAA and local names are now also searched, and display: elevation, distance and direction shown. [See the PR](https://github.com/fpw/avitab/pull/225)

- Closed airports are no longer loaded into the NAV database, and obsolete prefixes are now removed. [See the PR](https://github.com/fpw/avitab/pull/226)

- Extended (UTF-8) text overlayed on the map is now rendered correctly. [See the PR](https://github.com/fpw/avitab/pull/227)

- The selected online map source is stored in the preferences and will be used on the next launch. [See the PR](https://github.com/fpw/avitab/pull/229)

[dave6502](https://github.com/dave6502) contributed a couple of enhancements:

- Extend fix identification to support qualification with airport ID in addition to region ID. [See the changes](https://github.com/TeamAvitab/avitab/commit/9771c0d)

- Add bindable X-Plane commands to switch tabs in Charts app. [See the changes](https://github.com/TeamAvitab/avitab/commit/1b9f277)

The Avitab installable package no longer includes files that contain user preferences. This is intended to make updating an existing installation easier (simply copy the new files onto the existing installation), and less annoying (when preferences are unintentionally reset). Default versions of these files are created only if they do not exist.

Many thanks to all our contributors.

### Disclaimer

Avitab is provided for free in the hope that it will be found useful for flight simulation enthusiasts and gamers. We intend that the plugin will be reliable, however this release of Avitab has had limited testing, constrained by the systems and time available to the authoring team.

If you find a bug, please report it, and if you would like a new feature, please ask. We will make all reasonable efforts to respond, but be aware that we do other things for fun as well as this.

If you find our response times to be unacceptably slow in the 24/7 attention-deficit world of 2026, then, if you have C++ skills in your locker, fork the [project](https://github.com/TeamAvitab/avitab) and submit your ideas as a PR. Contributions are welcome.

All that having been said - enjoy!

*TeamAvitab*, April 2026.
