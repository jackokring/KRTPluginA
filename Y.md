# Scenario 1
## "I want to see some of the features"

So gate the top left gate to a drum or something. Goto `NOW` to check and select channel 1 for editing. Button `1/1`. Ok, now goto `PAT` to see the edit mode of the channel.

Place notes by pressing buttons. This get you a pattern of 16 in length. So you want a 64?

Go to `SEQ` and decide which of the 4 groups of 16 you want different (useually the last), and press button `2T/4` to select the last 16 pattern and a desire for it to play the second pattern.

Run the pattern until the last set of 3 light illuminate, as this places you in the last 16. Then select `PAT` mode and maybe `RUN` to stop at the right place. Now you can edit any channel of pattern 2 and use any mode (other than `PAT`) to select one of the three hot select patterns (patterns decided in `SEQ` mode) as the sequence progresses.

Use `MUTE` mode to mute channels, and these are global. That introduces you to the main features. You have 16 patterns of 16 channels with 12 of them which can be put on a hot select rythum of 64 in length.

You can select any of the 16 to be any of the stored 12, but `SEQ` mode will only place the changed one into the active quarter of the full 64.

Want to copy a sequence. Find it and `RUN` (stop) on it, then press `CPY` and a reference to it will be made note of. Even changing the pattern will change the copy as it is a reference to one of the 16. Then use `SEQ` mode and find the pattern of the 16 (by the hot selects or replacing a hot select by being on it and pressing the wanted pattern number). Button `PST` will paste a copy. There is one exception to this rule, and that is in `PAT` mode (so for whole patterns don't be in `PAT` mode on a `PST`), or only the channel last selected in `NOW` mode is overwritten by the channel selected when `CPY` was pressed. The indicators on `CPY` and `PST` indicate same pattern and same channel.

**Notes:** When not in `PAT` mode the triples are the hot selects (alter the quarter of 64 when they occur). Changing a hot select HAS to be when you are within the quarter of 64 and in `SEQ` mode, as you otherwise would be changing the pattern for the hot select that is currently in play.

The `CPY` buffer does **NOT** save your work as you edit the "copied" pattern. The edits **WILL** be pasted, as in actuality it is the pattern number that is copied, and used as the source for a paste `PST`. Right click `Save Preset` for more digital.

In `SEQ` mode the pattern will immediately change on pressing a quarter note (assuming different pattern content here), as that will become the new pattern for the hot swap indicated in the active quarter of the 64.

## "MIDI?"
Yes, connect `MIDI-CV` gate and CV in (green for fill that socket no spark). Now the 4 octaves around `C4` should activate all the buttons as the mouse is so slow. The pairs of black notes do the MAIN buttons and so change mode, start and stop etc., left to right. The black note groups of three do the triple buttons, and the white group of four around them do the quarters. The white group of three do the triples too, as that was just a logical convienience.

Look mum, no mouse! Try a channel 10 MIDI selection as that's conventional.

## "What no CV?"
Try the [W](README.md#w) module and use a few channels for some chromatic CV addition or subtraction. This definately made the operation of `Y` simpler and more elegant in design.

**Notes:** The keyboard interface is **monophonic** so don't expect to hot swap and stutter `RUN` at the same time.

## "Will I become Colin?"
![Colin Bender](https://github.com/jackokring/KRTPluginADemo/tree/master/docs/Pictures/Screenshot%20from%202021-08-15%2022-43-19.png)

![Y in use](https://github.com/jackokring/KRTPluginADemo/tree/master/docs/Pictures/Screenshot%20from%202021-08-15%2023-15-31.png)

# Scenario 2
## "Looks a bit like a drum machine"
Sure does but just produces gate outputs on the top 2 rows of jacks. 16 channels of gate outputs. Now the buttons have multicolored lights, you can go into `NOW` mode and tap the gates manually. See the red light? That's your currently selected channel for edits in `SEQ` mode. Push the `RUN` button/light for some motion. `TEMPO` away.

## "The pattern is too complex"
Try a bit of `MUTE` mode where red is for muted and gating comes as green. The mutes are global though and are not saved with a pattern change, but remain in place.

## "This machine looks like it is doing some kind of 64 pattern"
No, but yes. Try `SEQ` mode and the red light on the quarter note buttons indicates the active pattern in each of the four groups of 16. Push the button to the left of the lit red button on the bottom row. Did quarter notes 32 to 47 disappear? Have you just accidentally just put pattern 2 somewhere in the 64 sequence?

Just press the button next to the left most red button in the second to last row (in the group of 3). Looks like you hot switched back to pattern 1. Can you guess how to hot switch back? Can you guess how to set any of the 16 patterns in any of the 12 hot swaps?

## "So I can switch edit channels and patterns"
The module saves state and keep going. `CPY` and `PST` can copy and paste patterns (and just a channel to a channel in `SEQ` `PST`). Try the `W` (red) module. Add a few gates as musical intervals onto a CV?

## "So shuffles are controls 3 and 4"
Yes, and `G.LEN` sets the relative length of a gate in its time interval. The pico-drum module from the library is quite a nice drum sound set.