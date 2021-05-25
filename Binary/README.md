# Binary

This module is a 1V/Oct CV generator that takes several gate inputs and spits out 1V/Oct CV quantised by several choices of scale.

It's split into 2 identical channels. Each channel takes 6 gates, split into 3 for 'note' and 3 for 'octave'. Each group of 3 are combined as binary digits to derive a number from 0-7. The 'note' number corresponds to a degree on a scale. The 'octave' number.. defines the octave :)

A single potentiometer chooses the scale used on both channels, so both channels should sound fairly good together.

When any of the gates on a channel changes state, a trigger output goes HIGH, which can be used to EG trigger an envelope generator. You don't need to use all of the 12 possible gates - unconnected gates are considered LOW.
