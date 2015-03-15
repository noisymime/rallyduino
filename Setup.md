# Hardware #
The hardware setup of the rallyduino is fairly simple electronically. Probably the hardest part of the installation is the fitting of a hall effect sensor to the wheel to pickup speed. Note that it may also be possible to use the digital VSS line on a modern car, however I have not tested with this.

Whilst there are a number of ways that the code will work, the 'standard' wiring setup is shown below:
![http://www.noisymime.org/blogimages/Rallyduino_bb.png](http://www.noisymime.org/blogimages/Rallyduino_bb.png)

This will allow you full control of the interface using a Nintendo Wii controller.

# Software #
Setup of the software should be fairly straightforward. There are 3 configurable settings, found in the config.h file.

The first and most important, is selecting which serial LCD controller you want to use. Currently there are 2 controllers to choose from; lcd\_117\_h (For the controllers sold at http://www.phanderson.com/lcd106/lcd107.html) or lcd\_i2c\_h (For the i2c controller available from http://www.wentztech.com/radio/arduino/files/LCDI2C.html). I recommend using the second controller where possible as it seems both more stable and provides more functionality.

The other 2 options are simply to set the screen size (in characters) Currently the code will only reliably work with 20x4 displays, however 20x2 is also possible.

Once these options are set, simply compile and upload to your arduino.

# Configuration #
There are 2 'bootup' options available once the code is loaded onto the arduino. These are triggered by pressing certain buttons on the Wii controller and then powering the arduino. The current 2 options are:
# Hold down the C button to enter the calibration screen. The number entered represents the distance the car will travel per pulse. Eg If your tire circumference is 1700mm and there are 4 wheel studs, the calibration number would be 1700/4 or 0425
# Hold the Z button down and hold the joystick in the Up position. This will start a routine that simulates pulses at around 170km/h. This gives an overview of the functionality without needing a probe attached.