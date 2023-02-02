# psx2cdi

psx2cdi by @danhans42 - 23082022

    ** What is it? **
    Simple sketch to get a PlayStation 1 pad working as Philips CD-i controller. The one 
    I own has been eaten alive by battery leakage and no way am I paying eBay prices for 
    a new one. So made this.. plus I really like the OG PlayStation pad.
    This is basically a mashup of the DumpButtonsHwSpi example from PsxNewLib and the 
    CDi pad routines from SNES2CDi.
    Only works with digital buttons currently - no analogue sorry.
    
    ** Software **
    
    PSX Pad side provided by PsxNewLib - https://github.com/SukkoPera/PsxNewLib
    CD-i side used the SNES2CDi code as a base - https://github.com/anarterb/SNEStoCDi.
    
    Built using PlatformIO/Arduino
    
    Just build it and upload it, jobs a good one. I used a Mini Mega 2560 board as its what
    
    I had handy and the pinout in the code is written for that. If you use an 328/32U4 etc
    based board change pinouts accordingly.
    
    I will add some diagrams and further comments to the code in due course.
    
    ** Usage **
    
    Select & Start alter the direction movement speed. Writes changed setting to EEPROM 
    same as SNES2CDi. Other button info see source below. Mapping can be changed below.
    
    ** Hardware PSX Pad Side **
    
    We are using hardware SPI - so connections vary depending on board used. The code below 
    is setup for Arduino Mega2560. So use your ATmegas SPI PINs and assign your SS pin
    to ATT as needed. 
    
    Eg Arduino Uno (328) connect CMD: Pin 11, DATA: Pin 12, CLK: Pin 13 and 
    ATT: Pin 10.
    
    Eg. Arduino Mega2560, connect CMD: Pin 50, DATA Pin 51, CLK: Pin 52 and ATT: Pin 53
    Will add some diagrams shorltly
    
    For more info on this have a look at the repo for PsxNewLib
    
    You will need to pullup CMD/DATA (MOSI/MISO) to +3.3v via 1k resistors.
    
    ** Hardware CDI Side **
    
    RTS pin (A1) goes to CDi pin 7, TX (12) to CDi pin 2, GND to CDi pin 5 and +5v 
    to CDi pin 8.
    
    Make sure you dont power the arduino from both the CDi and pc at the same time!!
    Either put a diode in series or unplug it from the CDi before connecting to a PC.
    If you change pin assignments for the RTS pin please ensure its an analogue input -
    reasoning is the same as SNES2CDi.
    
    ** Thanks/Shouts/Etc **
    
    Thanks to SukkoPera (https://github.com/SukkoPera) for newpsxlib
    and to Laurent Berta for snes2cdi (https://github.com/anarterb/SNEStoCDi)
    
    Without the above this would be nothing.
    
    Shout outs to cdi discord & psx.dev discord.
