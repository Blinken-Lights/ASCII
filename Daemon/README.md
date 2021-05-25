# DAEMON

> In multitasking computer operating systems, a daemon (/ˈdiːmən/ or /ˈdeɪmən/)[1] is a computer program that runs as a background process, rather than being under the direct control of an interactive user.

Wikipedia contributors. "Daemon (computing)." Wikipedia, The Free Encyclopedia. Wikipedia, The Free Encyclopedia, 1 May. 2021. Web. 25 May. 2021.

'Daemon' is a cool sounding word, but it's also an appropriate description of this module.

It's intended to be the most stripped down and cheapest multifunction module possible, at the opposite end of the spectrum to eg. Ornament and Crime or Disting.

'Daemon' is based on a cheap and tiny microcontroller, the ATTiny85, and if you want an even more stripped down experience, the ATTiny45 can be used instead.

CV inputs are as basic as possible - just straight in to the analogue inputs of the μC with Schottky diodes as over voltage protection. 2 of the 3 CV inputs have passive attenuators (potentiometers) and are normalled to +V so can double as on-board knob inputs.

CV is output via PWM buffered through an op-amp. The ATTiny85 has fairly good fast PWM, so audio rate waveforms can be output acceptably. Both CV channels are hooked up to LEDs for some visual feedback.

CV in/output range is from 0 to 5V.

All 5 CV inputs can be reassigned as digital in/outputs if that fits the sketch that's running.

Finally, the reset pin is hooked up to a push button on the board. The ATTiny85 is able to remember how many times it's been reset, which can be exploited to, for example, page through different functions. That choice will be remembered when the module is powered back on.

Possible applications (of which I have coded NONE so far!) are:

- LFO .. single or dual, syncable, voltage controllable, different waveforms ETC..
- VCO .. reasonable 1V/Oct tracking should be possible
- Quantiser .. as above, should be enough accuracy on the in/out CV to track properly
- Bernoulli Gate
- Sample and Hold
- Envelope Generator .. EG voltage controlled AR or AD

This repo contains BOM and part position files that should be compatible with JLCPCB.
