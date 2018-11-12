# light-displays
Various projects that emit photons in various pleasing patterns.


This repository contains these projects:

- [ARLENE](Arlene) - Arduino microcontroller connected to eight solid-state relays, ostensibly to electrical fairy lights of some kind.  One button controls pattern displayed and speed.

- [LED Sign](LED_Sign) - Arduino controlled standalone display controller.  It drives some recycled/recovered LED panels, approx 170x16px resolution.  It implements a display buffer which it has routines to render text and graphics into, then blits it out to our weird display hardware.

- [Light Strand Engine](Light_Strand_Engine) - Arduino connected to a strand of WSxxx controlled RGB LEDs, and a couple buttons.  A handful of patterns can be displayed and animated on the string of lights.
