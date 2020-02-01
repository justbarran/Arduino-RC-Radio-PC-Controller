# Arduino RC Radio PC Controller
 Code to use an arduino pro micro or arduino Due to read RC Radio reciever signals and push them to a computer as a PC Controller

See my Channel https://youtube.com/c/justbarran for more infomation on how I did this project. 

You need the arduino JoystickLibrary writen by MHeironimus. 
https://github.com/MHeironimus/ArduinoJoystickLibrary

There are two versions of the code included. 

"rc_game_controller" uses Arduino function PulseIN. The number of channels is limited by the number of pins on the Due or Pro Micro.

"rc_game_controller_interrupts" uses a change of pin interrupt. On the Pro Micro, this is limited to 5 channels (5 interrupt pins on 0,1,2,3 and 7. While the Due supports interrupts on all pins.

Note you may have to edit the code and change the channel pins depending on your circuit or receiver outputs.
