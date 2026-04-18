# Avitab Release Notes

## Avitab v1.0.1 (April 2026)

This is a bugfix release to resolve [interoperability issues with some 3rd-party aircraft and other plugins](https://github.com/TeamAvitab/avitab/issues/47).

## Avitab v1.0.0 (April 2026)

### Summary

This is the first update to Avitab overseen by [TeamAvitab](https://github.com/TeamAvitab) after [Folke Will](https://github.com/fpw) stepped back from full-time maintenance. Many thanks to Folke for his original authorship and ongoing development of the project, and for his support in transferring the project to TeamAvitab.

This release has been given the label v1.0.0 to mark the handover, and to indicate that no major new features are considered necessary for basic usability. (In practice, Avitab has been in this state for many previous releases, but the major version never got the bump it perhaps should have had.)

Much of the work done since TeamAvitab adopted the project has been under-the-hood, updating the project structure and build scripts, and incorporating up-to-date versions of many of the third-party libraries. A number of contributions to Avitab since the 0.7.1 release have also been adopted.

### Bug fixes

Avitab's support for PDF and other documents is implemented by the MuPDF library, and this has now been updated to a newer version which resolves simulation crashes that had been [reported](https://github.com/fpw/avitab/issues/232) in Avitab v0.7.1. Thanks to [tzimpel](https://github.com/tzimpel) for his work in identifying the cause and prototyping a solution.

### Enhancements

[sfrsfrsfr](https://github.com/sfrsfrsfr) submitted a number of enhancements to the previous project archive which have been integrated into the new Avitab project:

- Use TCAS datarefs for other aircraft. This resolves an unrecorded issue where other aircraft were not displayed when connected to VATSIM (and probably other multi-player systems). [See the PR](https://github.com/fpw/avitab/pull/218)

- A `Nearest` button is added to the Airports app. [See the PR](https://github.com/fpw/avitab/pull/220)

- Additional runway surface types are now supported when loading the NAV data. [See the PR](https://github.com/fpw/avitab/pull/222)

- The header clock can be clicked to cycle through four modes: real world clock, sim zulu time, sim local time, stopwatch. [See the PR](https://github.com/fpw/avitab/pull/223).
    - Real world clock is the local time on your PC. It is annotated `{hh:mm}`
    - Simulation zulu time is annotated with a `z` suffix.
    - Simulation local time has no further annotation.
    - The stopwatch is annotated with a `+` prefix. Click-and-hold to reset. A short-click will cycle to the next clock mode and the stopwatch will continue counting in the background.

- Airport search will match FAA and local names as well as ICAO codes. Airport summary information also shows the elevation, distance and direction. [See the PR](https://github.com/fpw/avitab/pull/225)

- Closed airports are no longer loaded into the NAV database, and obsolete prefixes are now removed. [See the PR](https://github.com/fpw/avitab/pull/226)

- Extended (UTF-8) text overlayed on the map is now rendered correctly. [See the PR](https://github.com/fpw/avitab/pull/227)

- The selected online map source is stored in the preferences and will be used on the next launch. [See the PR](https://github.com/fpw/avitab/pull/229)

[dave6502](https://github.com/dave6502) contributed a couple of enhancements:

- Extend fix identification to support qualification with airport ID in addition to region ID. [See the changes](https://github.com/TeamAvitab/avitab/commit/9771c0d)

- Add bindable X-Plane commands to switch tabs in Charts app. [See the changes](https://github.com/TeamAvitab/avitab/commit/1b9f277)

Many thanks to all our contributors.

### Installable Package Updates

The Avitab installable package no longer includes files that contain user settings. This is intended to make updating an existing installation easier (simply copy (merge) the new files onto the existing installation), and less annoying (when settings are unintentionally reset). Default versions of these settings files are created only if they do not exist.

The Avitab installable package no longer includes information about integrating Avitab into aircraft cockpit models. This information can now be found in the [Avitb wiki](https://github.com/TeamAvitab/avitab/wiki/X%E2%80%90Plane-Aircraft-Integration).

### Feature Removal Notices

We plan to remove support for the X-Plane map source from the Maps app. Removing this feature will allow us to drop the use of an obsolete third-party library and bring the rest of the Avitab codebase into the modern era.

We plan to remove the route generation feature from the Routes app. Avitab's generation of routes is not reliable, and we doubt anyone is using this. Navigraph's free [SimBrief](https://www.simbrief.com/home/) is a much better way to create route plans and these can then be exported as X-Plane FMS files and imported into Avitab for overlay in the Maps app. Removing this feature will simplify the planned migration to a SQLite-based navigation database, part of the longer-term development roadmap.

Since we are almost certain that neither of these features will be missed, we will probably remove them before the next release. Please let us know if either of these are features that you absolutely rely on.

### Disclaimer

Avitab is provided for free in the hope that it will be found useful for flight simulation enthusiasts and gamers. We intend that the plugin will be reliable, however this release of Avitab has had limited testing, constrained by the systems and time available to the authoring team.

If you find a bug, please report it, and if you would like a new feature, please ask. We will make all reasonable efforts to respond, but be aware that we do other things for fun as well as this.

If you find our response times to be unacceptably slow in the 24/7 attention-deficit world of 2026, then, if you have C++ skills in your locker, fork the [project](https://github.com/TeamAvitab/avitab) and submit your ideas as a PR. Contributions are welcome.

All that having been said - enjoy!

*TeamAvitab*, April 2026.
