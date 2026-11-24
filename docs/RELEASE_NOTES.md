# Avitab Release Notes

## Avitab v1.3.0 (unreleased)

This is an unreleased (development) build.

## Avitab v1.2.0 (November 2026)

This is a periodic update that consolidates contributed features and bug fixes.
Many thanks to all contributors, especially [sfrsfrsfr](https://github.com/sfrsfrsfr)
for many of the enhancements and bug fixes.

### Enhancements

#### Heading-up mode

The Map application now supports a heading-up tracking mode.
A new toolbar button (UP arrow) has been added and is used to toggle between
regular (north up) and heading-up tracking modes.

When heading-up tracking mode is active the user aircraft location is placed 
near the bottom of the frame and the map is rotated so that the current track 
of the aircraft is vertical.

Map rotation is only supported when heading-up tracking is enabled. 
If the map is moved (mouse drag) when in heading-up mode tracking will 
be disabled and the map will revert to north-up display mode.

#### RNP/GLS approaches

RNP/GLS approaches have been added to the navigation database. They can be
overlaid onto maps and georeferenced charts by enabling the RNP overlay
filter (in the Map application).

#### Download of georeference databases

Chart georeference metadata can now be stored online and referenced in 
the Avitab preferences file. Avitab will keep its local chart calibration 
dataset up-to-date from this source.

The initial default dataset is for the UK AIP charts and is hosted in the 
TeamAvitab github project.

For further information about creating calibration datasets 
please contact the Avitab team.

#### Night mode

A global setting for night mode can now be toggled through the global settings 
dialog (next to the FPS report in the header bar). This will affect Navigraph 
map tile sources and the default display of Airport app charts. Charts in the 
Airport app can be individually toggled between day and night mode irrespective 
of the global setting.

#### Other improvements

- The Airport app will display the METAR string (if available), or a local weather
report if not. (METAR and weather reports are only available with X-Plane v12.)

- The current clock mode (real world clock, sim zulu time, sim local time, stopwatch)
is now saved at close and restored on next launch.

- When zooming the map with the mouse wheel and not in a tracking mode the focus of 
the zoom will follow the location of the mouse pointer. If a tracking mode is active 
the zoom will focus at the user aircraft location.

- Other aircraft overlays now look like planes. (The size of the icon does not 
represent the category of the aircraft!)

- Clipboard text pasting into the Notes app is now supported on all platforms 
(previously only on Windows).

- The Esri World Topo tile server has been added to the default list created 
when Avitab is first run. Existing installations will be unaffected, but if 
this tile server is preferred then the existing `mapconfig.json` file can be 
deleted and Avitab will create a new one when it is next started. (To merge 
old and new settings, rename the old file and then merge required items from 
it into the new one.)

### Bug fixes

- Mouse wheel scrolling in multi-page documents now changes pages when scrolling 
beyond the bottom or top of the page.

- Overlays shown in the Airport and Charts apps are filtered identically to the 
Map app. There is no separate settings dialog, so filtering needs to be configured 
in the Map app first.

> There are hidden settings in the Avitab preferences file that allow overlays to 
be permanently disabled for the Airport and Charts apps. Look for the keys 
`show_overlays_in_airport_app` and `show_overlays_in_charts_app` and change their 
values to false (default is true).

### Removed features

- X-Plane maps are no longer available as a Map tile source. The feature depended 
on a 3rd-party library that is no longer maintained and was blocking other project 
improvements.

- METAR data for XP11 is no longer available. Continued support for this was 
incompatible with other project improvements and the effort to maintain this was 
not considered to be worthwhile.

### Known issues

The following issues are known, but were deferred for fixing in a future release.

- In the Airport application, when displaying a document or chart without geo-referencing data, the tracking
mode button will still be shown and enabled. To use focussed mouse wheel scrolling the track button
needs to be toggled off first.

- In the Map application, when tracking mode is enabled, clicking on an overlaid NAVaid highlights
it (shows detailed information) but as the NAVaid is moved away from the click-point a different NAVaid
will be highlighted when it becomes closer to the click-point.

- The global nightmode UI control is only available through the header toolbar settings dialog. If this has been disabled by the current aircraft's panel integration configuration then it will not be possible to change the global nightmode setting. Workaround this by first selecting a basic aircraft without an integrated Avitab panel. Set the desired nightmode setting in the Avitab window, and then reconfigure the flight with the desired aircraft. The nightmode setting can also be modified in the Avitab preferences file before starting X-Plane.

There are also a number of enhnacements requested or planned. 
See the [Avitab issues](https://github.com/TeamAvitab/avitab/issues) for the latest full list of bugs and enhancements.

## Avitab v1.1.0 (June 2026)

This is small feature update that adds the **Aircraft Managed** panel integration mode which enables aircraft
using the latest cockpit digital instrument API for X-Plane 12 to support Avitab in their cockpit or EFB models.

A full description of Avitab's aircraft integration modes and integration guidance for aircraft designers
is available to read in the [Avitab Wiki](https://github.com/TeamAvitab/avitab/wiki/X%E2%80%90Plane-Aircraft-Integration).

The new panel integration mode needs to be explicitly enabled and supported by the aircraft designer, so there will be
no immediate change in behaviour with aircraft already installed at the time of this Avitab release. Avitab users
**cannot** simply switch to the new integration mode, but updating Avitab now will mean that it is ready to support
new and updated aircraft when they become available.

Existing aircraft that had an Avitab panel integration that stopped working sometime around the transition to X-Plane 12
will need to be updated to use this new panel integration mode.

## Avitab v1.0.2 (April 2026)

This is a bugfix/enhancement release to [prefer Navigraph daytime-mode maps](https://github.com/TeamAvitab/avitab/issues/12).
More users have indicated a preference for daytime maps, so this is now the default.
Users who prefer the night-time maps can edit the Avitab.prf file and change the `night_mode` setting to `true`.
(The preferences file should only be modified when X-Plane is not running, and it will be required to run Avitab and select a Navigraph map source at least once before the new setting will appear in the file.)

A more consistent approach to day/night mode is part of the Avitab roadmap, so this solution is deliberately low-end.

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
