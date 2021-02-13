# Differential Repeater

The Differential Repeater (named after Gilles Deleuze's great work of philosophy Difference and Repetition)
is a large scale sequencer realm `[16][16][256]`  where sequences and patterns may be continually modified
by constructive (additive, no overwrite) and destructive (negating, overwrtite) algorithms.

## Examples of Contstuction/Destruction algorithms
### Constructive
- Fugue Copy (shoots a 5th copy of src to target)
- Maybe Copy (imperfect copy that possibly forgets notes)
### Destructive
- Forget (like Maybe Copy but in place)
- Spread (push notes farther apart in pitch)
- Disperse (push notes farther apart in time)

These are being created and destoryed all the time at the moment, because this project is currently in development.

## Data structures
In order to get the most amount of steps and tracks, MIDI note data is compressed down to a byte...
```
uint8_t degree:4;
uint8_t octave:3;
uint8_t isOn:1;
```
The scale degree and octave then are passed to a note builder which creates actual MIDI note values based on the chosen scale.
No velocity information is stored. The assumption is that will be generated at the time of building the note. 
Certain concessions (some very easy ones) were made with this apporach.
For one, the storing of velocity was thrown out entirely. In fact any MIDI message that
could be seen as 'expressive' would ideally be  handled by built in LFO's or CV and passed in real-time and
not stored. It is highly unlikley that one would use this 'instrument' for surgical precision
in composition. Rather, one might hit a few notes, run some clock signals in to trigger the algorithm
and come back in a half hour or so to see what part of a contrapunctal galaxy they now have charted.

## Acknowledgements
In order to get the project up and running I pilfered some handy implementation from Adafruit's
FifteenStep for things like the MIDI callback, shuffle and quantize, as well as a general layout. Over the
course of development these things have or still might end up implememnted differently but, it was a big help to get
started and helped to bring to light some things I had not thought about. So, thanks to that project.

Research is also being done on Scala tuning approaches. Having 14 bit pitchbend per note would
be very handy for some really bizarre scales.





