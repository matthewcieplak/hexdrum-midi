# Hexdrum
Hex drum is a midi-capable port of the [Big Button sequencer by Look Mum No Computer](https://www.lookmumnocomputer.com/projects#/big-button).

The code is completely rewritten for easier editing and optimizations and to add a few new features:
* MIDI input (clock and note-to-gate)
* Note audition 
* 32nd note fills
* Clear all/shift all banks
* Mute/solo

![Hexdrum](https://github.com/matthewcieplak/hexdrum-midi/raw/master/fritzing/big%20button%20midi_bb.png)

## MIDI input

MIDI input is received by default on Channel 16 and notes received on 6 standard MIDI GM drum notes (C1, D1, E1, F1, G1, A1) are transmitted to the six gate outputs.

MIDI input is acheived through a standard 6n138 optocoupler on a small daughterboard. MIDI THRU can be added with a 2n3904 transistor and a few extra resistors.

--- Stripboard layout ---
![Hexdrum MIDI input stripboard layout](https://github.com/matthewcieplak/hexdrum-midi/raw/master/fritzing/big%20button%20midi_bb.png)


--- Schematic ---
![Hexdrum MIDI input stripboard layout](https://github.com/matthewcieplak/hexdrum-midi/raw/master/fritzing/big%20button%20midi_schem.png)


