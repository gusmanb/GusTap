# GusTap 1.0

GusTap is a .tap player for the ZX Spectrum.

I decided to create this because after using TZXDuino for a while and trying to adapt it to my needs it was worth to start the code from zero than trying to modify it.

The solution is composed of three projects: 
* GusTap, the main player, project for Arduino,
* TapPlayer, the universal .tap library for Arduino
* GusTapOrganizer, a command line tool to organize your tap's and enable long names

## Functionalities

The basic hardware for GusTap is any Arduino (I used a MiniPro), an I2C 0.91" screen, a SD card module, three buttons and a 3.5mm jack.
I will publish an schematic and a wiring diagram soon (using a universal drilled pcb).

The code is prepared in such a way that reusing it is very easy. You can import just the parts that you need what will allow you to create your own tap player implementing whatever you want, per examle you could create a player that reads the tap files from a remote source using TCP, or bluetooth or whatever you prefer.
Use GusTap as reference if you want to create your own player.

One of the functionalities that the library exposes is a tap browser for SD cards. This browser if used alltogether with GusTapOrganizer will give the player the ability to show long file names even in FAT16 cards (this is one of the main reasons I created this project).

Also, you can use GusTapOrganizer by itself, it will sort all the tap files in the specified folder into subfolders with a 3-depth structure to make very easy to navigate the SD card from the device.

I will add more info soon.

Cheers!