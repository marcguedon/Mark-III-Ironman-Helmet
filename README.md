# Mark III Ironman Helmet

First of all, here is the [link to the project](http://thedraill.e-monsite.com/pages/projects/mark-3-ironman-helmet.html).

## Version 1

Version 1 has a TTP223B sensor that can detect the approach of a hand through plastic, as well as 2 servo motors to open/close the helmet and 2 LEDs for the eyes.

This version only needs the ServoEasing library. So, in the Library Manager tab of the Arduino IDE, filter with "ServoEasing". The library to install is [Armin's](https://docs.arduino.cc/libraries/servoeasing/).

[Demo video](https://www.youtube.com/watch?v=RCTxJQw5CCg)

## Version 2 (with voice recognition)

Version 2 adds a voice recognition module that allows the helmet to react to a few voice commands.

In addition to the ServoEasing library (see Version 1), a modified version of the VoiceRecognitionV3 library (from [Elechouse Team](https://github.com/elechouse/VoiceRecognitionV3)) is used (to make it works with Serial1). The modified version (made by [johnwasser](https://forum.arduino.cc/t/combine-two-voice-recognition-module/910286/10)) of the library is already present with the code.

[Demo video](https://www.youtube.com/watch?v=vpdiGfNVcfc)
