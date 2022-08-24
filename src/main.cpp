/*
    PSX2CDI by @danhans42 - 23082022

    github.com/danhans42/psx2cdi

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

*/

#include <Arduino.h>
#include <DigitalIO.h>
#include <PsxControllerHwSpi.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

//Pin Assingments

SoftwareSerial vSerial(11, 12, true); //rxpin, txpin, inversion (we dont use RX pin here but define it anyway)

const int RTSpin = A1;                //rts pin
const int RTSthreshold = 328;         // threshold for the CDi RTS analog detection
const byte PSX_ATT = 53;              //att line
const byte PSX_PADLED = 13;           //led on when pad detected


byte spd;                             // speed scaling

bool btnRpressed = false;
int padbyte0, padbyte1, padbyte2, oldpadbyte0, oldpadbyte1, oldpadbyte2, x, y;
bool firstId = true;
bool btnLpressed = false;
bool btnSEpressed = false;


// true if RTS asserted
bool assertRTS() {
    if(analogRead(RTSpin) < RTSthreshold) return false;
    else return true;
}

// send back the correct Dpad value depending on the speed setting
int adjustSpeed(int val) {
    if(val==127 || spd==5) return val;
    if(val==254) {
        if(spd==4) return 202;
        if(spd==3) return 170;
        if(spd==2) return 149; 
        if(spd==1) return 130;
    }
    else {
        if(spd==4) return 53;
        if(spd==3) return 85;
        if(spd==2) return 106; 
        if(spd==1) return 125;
    }
    return 0;
}

// change speed setting and save it to the EEPROM
void changeSpeed(byte newspeed)
{
    if(newspeed<1) newspeed=1;
    else if(newspeed>5) newspeed=5;
    spd=newspeed;
    EEPROM.write(0, spd);
}

byte psxButtonToIndex (PsxButtons psxButtons) {
	byte i;
    for (i = 0; i < PSX_BUTTONS_NO; ++i) {
        if (psxButtons & 0x01) {
            break;
        }
        psxButtons >>= 1U;
    }
    return i;
}

void ReadButtons (PsxButtons psxButtons) {
    
    static PsxButtons lastB = 0;

    if (psxButtons != lastB) {
        lastB = psxButtons;     // Save it before we alter it
        for (byte i = 0; i < PSX_BUTTONS_NO; ++i) {
            byte b = psxButtonToIndex (psxButtons);
            psxButtons &= ~(1 << b);
        }
    }

    // manage speed control
    if(psxButtons & PSB_START) {
        if(!btnRpressed) changeSpeed(spd+1); // speed : up
        btnRpressed = true;
    } 
    else btnRpressed = false;
    
    if(psxButtons & PSB_SELECT) {
        if(!btnLpressed) changeSpeed(spd-1); // speed : down
        btnLpressed = true;
    } 
    else btnLpressed = false;

    // Dpad X axis
    x = 127;
    if(psxButtons & PSB_PAD_LEFT) x = 254;
    if(psxButtons & PSB_PAD_RIGHT) x = 1;
    x = adjustSpeed(x);
  
    if(x<127) // right
    {
        x = x ^ 0b01111111;
        x = x + 1;
        padbyte1 = padbyte1 | x;
        padbyte1 = padbyte1 & 0b10111111;
        if((x & 0b01000000) != 0) padbyte0 = padbyte0 | 0b00000001;
    }
    else if(x>127) // left
    {
        x = x ^ 0b01111111;
        x = x + 1;
        padbyte1 = padbyte1 | x;
        padbyte1 = padbyte1 & 0b10111111;
        if((x & 0b01000000) != 0) padbyte0 = padbyte0 | 0b00000011;
        else padbyte0 = padbyte0 | 0b00000010; 
    }

    // Dpad Y axis
    y = 127;
    if(psxButtons & PSB_PAD_UP) y = 254;
    if(psxButtons & PSB_PAD_DOWN) y = 1;
    y = adjustSpeed(y);

    if(y<127) // down
    {
        y = y ^ 0b01111111;
        y = y + 1;
        padbyte2 = padbyte2 | y;
        padbyte2 = padbyte2 & 0b10111111;
        if((y & 0b01000000) != 0) padbyte0 = padbyte0 | 0b00000100;
    }
    else if(y>127) // up
    {
        y = y ^ 0b01111111;
        y = y + 1;
        padbyte2 = padbyte2 | y;
        padbyte2 = padbyte2 & 0b10111111;
        if((y & 0b01000000) != 0) padbyte0 = padbyte0 | 0b00001100;
        else padbyte0 = padbyte0 | 0b00001000;
    } 
    
    // Buttons
    if( psxButtons & PSB_CROSS ) padbyte0 = padbyte0 | 0b00100000;  //button 1 
    if( psxButtons & PSB_CIRCLE ) padbyte0 = padbyte0 | 0b00010000;  //button 2
    if((psxButtons & PSB_SQUARE) || (psxButtons & PSB_TRIANGLE)) padbyte0 = padbyte0 | 0b00110000; // button 3

}

PsxControllerHwSpi<PSX_ATT> psx;
boolean haveController = false;

void setup () {

    pinMode(11, INPUT);
    pinMode(12, OUTPUT);


    fastPinMode (PSX_PADLED, OUTPUT);
    delay (30);

    //debug output
    Serial.begin (115200);
    
    oldpadbyte0 = 0;
    oldpadbyte1 = 0;
    oldpadbyte2 = 0;

    byte eepromData = EEPROM.read(0); // retrieve speed setting from the Arduino's EEPROM
    //  Serial.println ("\nPSX2CDI Pad Adapter\n");
    //  Serial.print ("Checking EEPROM.. ");

    if(eepromData >= 1 && eepromData <= 5) {
        spd = eepromData;
        //Serial.print ("setting found : ");
    }
    else {  
        spd = 3;
        //Serial.print ("no setting found.. using default :");      
    }
    //Serial.println (spd, HEX);
    vSerial.begin(1200); // open serial interface to send data to the CDi
}

void loop () {

    if(!assertRTS()) {
        while(!assertRTS()) { } // wait for CDi to assert the RTS line
        if(firstId) delay(50);//was 100
        else delay(1);
        firstId = false;
        vSerial.write(0b11001010); // send device id ("maneuvering device")
    }

    padbyte0 = 0b11000000;  //initialize data bytes
    padbyte1 = 0b10000000;
    padbyte2 = 0b10000000;

	fastDigitalWrite (PSX_PADLED, haveController);
	
	if (!haveController) {
  		if (psx.begin ()) {
            //Serial.println (F("PSX Controller found!"));
            delay (300);
            haveController = true;
		}
	} 
    else {
	  	if (!psx.read ()) {
            //Serial.println (F("PSX Controller disconnected"));
            haveController = false;
		} 
        else {
            ReadButtons (psx.getButtonWord ());
		}
	}

    if((padbyte0 != oldpadbyte0) || (padbyte1 != 0b10000000) || (padbyte2 != 0b10000000) || ((padbyte0 & 0b00001111) != 0))  // see if state has changed
    {     
        if(assertRTS()) vSerial.write(padbyte0);
        if(assertRTS()) vSerial.write(padbyte1);
        if(assertRTS()) vSerial.write(padbyte2);
    }
    // save state
    oldpadbyte0 = padbyte0; 
    oldpadbyte1 = padbyte1;
    oldpadbyte2 = padbyte2;
	//delay (1000 / 60);
}