The rallyduino project aims to design both the hardware and software required to build an open source rally computer. To see the system running in a test environment, please checkout the following video: http://www.youtube.com/watch?v=KyW1XwZ2ToM

The current version of the rallyduino software contains the following features:

* 2 independant distance/time/speed counters
* Tracking of distance/speed in miles or kilometers and can be switched on the fly
* Count up and down (With appropriate recalculation of average speed etc)
* Remote control through the use of a Wii 'nunchuck' controller
* Tracks time and average speed along with distance


Currently the software is reasonably specific to the hardware I'm running it on (Though this may change in the future to cater for different Serial->LCD boards). I plan to put together some diagrams illustrating the components, but until then here is a list of what you will need:

Arduino Diecimila (Though derivatives will almost certainly work as well)
A 4x20 HD44780-based LCD
Serial LCD adaptor based on Peter Anderson's command set. These units are available through Peter's website, Modern Devices or Wulfden OR:
An i2c LCD controller compatible with this library http://www.wentztech.com/radio/arduino/files/LCDI2C.html. This is currently experimental, but will likely be the preferred interface in the future
A Wii nunchuck (Preferably with an extension cable so you don't have to mutilate your controller)
Some way to connect all the above together (ie breadboard or arduino prototyping shield and a bunch of wire)
Instructions on how to setup the device can be found at: https://github.com/noisymime/rallyduino/wiki/Setup
