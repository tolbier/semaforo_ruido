/*
  mp3_player
 -----------------
     This file is part of SEMAFORO_RUIDO.

    SEMAFORO_RUIDO is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SEMAFORO_RUIDO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SEMAFORO_RUIDO.  If not, see <http://www.gnu.org/licenses/>.
---------------    
    Archivo: mp3_player.ino
    Fecha:20-ENE-2014
    Version: 0.80
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es
    NOTA:
        Este codigo esta basado en el programa MP3 Player de Nathan Seidle  - Sparkfun
        http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3_Player_Example.pde
        
        Su funcion es reproducir archivos MP3 a traves de la tarjeta MP3 Player Arduino Shield de Sparkfun
        Ver: https://www.sparkfun.com/products/10628
        
 -------------------
 */



#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h> 
#include "Arduino.h"

//Create the variables to be used by SdFat Library
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile track;

//This is the name of the file on the microSD card you would like to play
//Stick with normal 8.3 nomeclature. All lower-case works well.
//Note: you must name the tracks on the SD card with 001, 002, 003, etc. 
//For example, the code is expecting to play 'track002.mp3', not track2.mp3.
char trackName[] = "12345678.123";
int trackNumber = 1;

boolean mp3shield_present = false; 

char errorMsg[100]; //This is a generic array used for sprintf of error messages

#define TRUE  0
#define FALSE  1

#define MP3LOG 0           // Imprime en Serial algun registro de log durante la reproduccion MP3

//MP3 Player Shield pin mapping. See the schematic
#define MP3_XCS 6 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS 7 //Data Chip Select / BSYNC Pin
#define MP3_DREQ 2 //Data Request Pin: Player asks for more data
#define MP3_RESET 8 //Reset is active low
//Remember you have to edit the Sd2PinMap.h of the sdfatlib library to correct control the SD card.

//VS10xx SCI Registers
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F


#define MaxNumberOfFilesPerLevel 10

int numberOfFilesPerLevel[numberOfLevels];

int init_mp3_player() {
  mp3shield_present = false; 
  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  pinMode(MP3_RESET, OUTPUT);
 
  digitalWrite(MP3_XCS, HIGH); //Deselect Control
  digitalWrite(MP3_XDCS, HIGH); //Deselect Data
  digitalWrite(MP3_RESET, LOW); //Put VS1053 into hardware reset



  //Setup SD card interface
  pinMode(9, OUTPUT);       //Pin 10 must be set as an output for the SD communication to work.
  if (!card.init(SPI_FULL_SPEED,9))  {
    Serial.println("Error: Card init"); //Initialize the SD card and configure the I/O pins.
    return (1);
  }
  if (!volume.init(&card)) {
    Serial.println("Error: Volume ini"); //Initialize a volume on the SD card.
    return (2);
  }
  if (!root.openRoot(&volume)){
    Serial.println("Error: Opening root"); //Open the root directory in the volume. 
    return (3);
  }

  //We have no need to setup SPI for VS1053 because this has already been done by the SDfatlib

  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is 1.0x after power up. 
  //Therefore, max SPI speed is 1.75MHz. We will use 1MHz to be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV16); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF); //Throw a dummy byte at the bus
  //Initialize VS1053 chip 
  delay(10);
  digitalWrite(MP3_RESET, HIGH); //Bring up VS1053
  //delay(10); //We don't need this delay because any register changes will check for a high DREQ

  // Mp3SetVolume(0, 0); //Set initial volume (20 = -10dB) LOUD
  //Mp3SetVolume(40, 40); //Set initial volume (20 = -10dB) Manageable
  //Mp3SetVolume(27, 127); //Set initial volume (20 = -10dB) More quiet

  //Let's check the status of the VS1053
  int MP3Mode = Mp3ReadRegister(SCI_MODE);
  int MP3Status = Mp3ReadRegister(SCI_STATUS);
  int MP3Clock = Mp3ReadRegister(SCI_CLOCKF);

  Serial.print("SCI_Mode (0x4800) = 0x");
  Serial.println(MP3Mode, HEX);

  Serial.print("SCI_Status (0x48) = 0x");
  Serial.println(MP3Status, HEX);

  int vsVersion = (MP3Status >> 4) & 0x000F; //Mask out only the four version bits
  Serial.print("VS Version (VS1053 is 4) = ");
  Serial.println(vsVersion, DEC); //The 1053B should respond with 4. VS1001 = 0, VS1011 = 1, VS1002 = 2, VS1003 = 3

  Serial.print("SCI_ClockF = 0x");
  Serial.println(MP3Clock, HEX);

  //Now that we have the VS1053 up and running, increase the internal clock multiplier and up our SPI rate
  Mp3WriteRegister(SCI_CLOCKF, 0x60, 0x00); //Set multiplier to 3.0x

  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is now 3x.
  //Therefore, max SPI speed is 5MHz. 4MHz will be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV4); //Set SPI bus speed to 4MHz (16MHz / 4 = 4MHz)

  MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
  Serial.print("SCI_ClockF = 0x");
  Serial.println(MP3Clock, HEX);

  init_numberOfFilesPerLevel();
  
  mp3shield_present=true;
  return (0);
  //MP3 IC setup complete
}


//#define MaxNumberOfFilesPerLevel 10
//int numberOfFilesPerLevel[numberOfLevels];
void init_numberOfFilesPerLevel(){
   //Recorrer los 4 niveles
   Serial.print("numberOfFilesPerLevel: ");
   for (int i =0 ; i<numberOfLevels; i++){
     //para cada nivel, averiguar cual es el numero de archivos que hay disponibles, 
     //y guardarlo en numberOfFilesPerLevel
     numberOfFilesPerLevel[i]= getNumberOfFilesPerLevel(i);
     Serial.print(numberOfFilesPerLevel[i]);
     Serial.print(" ");
   }
   Serial.println();
}

int getNumberOfFilesPerLevel(int level){
  int i=0;
  boolean encontrado=false;
  while (i<MaxNumberOfFilesPerLevel && ! encontrado){
    encontrado = ! existsFileLevelNumber(level,i);
    i++;
  }
  if (encontrado){
    i--;
  }
  //Devuelve el numero de files encontrados
  return i;
  
}

boolean existsFileLevelNumber(int level,int numFile){
  boolean retorno;
  sprintf(trackName, "lev%1d_%03d.mp3", level,numFile);
  retorno =track.open(&root, trackName, O_READ) ; //Open the file in read mode.
  if (retorno) track.close();
  return retorno;
}
void playLevelWarningMp3(int level){
  
//  sprintf(trackName, "lev%d_%03d.mp3", level, trackNumber);
}


void playMP3Level(int level){
  if (! mp3shield_present) return;
  
  int numberOfFiles = numberOfFilesPerLevel[level];
  int selectedNumber= random(0,numberOfFiles);
  
  sprintf(trackName, "lev%1d_%03d.mp3", level,selectedNumber);
  playMP3(trackName);
}
  
  
//PlayMP3 pulls 32 byte chunks from the SD card and throws them at the VS1053
//We monitor the DREQ (data request pin). If it goes low then we determine if
//we need new data or not. If yes, pull new from SD card. Then throw the data
//at the VS1053 until it is full.
void playMP3(char* fileName) {

  if (!track.open(&root, fileName, O_READ)) { //Open the file in read mode.
    sprintf(errorMsg, "Failed to open %s", fileName);
    Serial.println(errorMsg);
    return;
  }

  Serial.println("Track open");

  uint8_t mp3DataBuffer[32]; //Buffer of 32 bytes. VS1053 can take 32 bytes at a go.
  //track.read(mp3DataBuffer, sizeof(mp3DataBuffer)); //Read the first 32 bytes of the song
  int need_data = TRUE; 
  long replenish_time = millis();

  Serial.println("Start MP3 decoding");

  while(1) {
    while(!digitalRead(MP3_DREQ)) { 
      setVolumeFromPotenciometro();
      //DREQ is low while the receive buffer is full
      //You can do something else here, the buffer of the MP3 is full and happy.
      //Maybe set the volume or test to see how much we can delay before we hear audible glitches

      //If the MP3 IC is happy, but we need to read new data from the SD, now is a great time to do so
      if(need_data == TRUE) {
        if(!track.read(mp3DataBuffer, sizeof(mp3DataBuffer))) { //Try reading 32 new bytes of the song
          //Oh no! There is no data left to read!
          //Time to exit
          break;
        }
        need_data = FALSE;
      }

      //Serial.println("."); //Print a character to show we are doing nothing

      //This is here to show how much time is spent transferring new bytes to the VS1053 buffer. Relies on replenish_time below.
      if (MP3LOG){
        Serial.print("Time to replenish buffer: ");
        Serial.print(millis() - replenish_time, DEC);
        Serial.print("ms");
      }

      //Test to see just how much we can do before the audio starts to glitch
      long start_time = millis();
      //delay(150); //Do NOTHING - audible glitches
      //delay(135); //Do NOTHING - audible glitches
      //delay(120); //Do NOTHING - barely audible glitches
      delay(100); //Do NOTHING - sounds fine

      if (MP3LOG){
        Serial.print(" Idle time: ");
        Serial.print(millis() - start_time, DEC);
        Serial.println("ms");
      }

      //Look at that! We can actually do quite a lot without the audio glitching

      //Now that we've completely emptied the VS1053 buffer (2048 bytes) let's see how much 
      //time the VS1053 keeps the DREQ line high, indicating it needs to be fed
      replenish_time = millis();
    }


    if(need_data == TRUE){ //This is here in case we haven't had any free time to load new data
      if(!track.read(mp3DataBuffer, sizeof(mp3DataBuffer))) { //Go out to SD card and try reading 32 new bytes of the song
        //Oh no! There is no data left to read!
        //Time to exit
        break;
      }
      need_data = FALSE;
    }

    //Once DREQ is released (high) we now feed 32 bytes of data to the VS1053 from our SD read buffer
    digitalWrite(MP3_XDCS, LOW); //Select Data
    for(int y = 0 ; y < sizeof(mp3DataBuffer) ; y++) {
      SPI.transfer(mp3DataBuffer[y]); // Send SPI byte
    }

    digitalWrite(MP3_XDCS, HIGH); //Deselect Data
    need_data = TRUE; //We've just dumped 32 bytes into VS1053 so our SD read buffer is empty. Set flag so we go get more data
  }

  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating transfer is complete
  digitalWrite(MP3_XDCS, HIGH); //Deselect Data
  
  track.close(); //Close out this track

  sprintf(errorMsg, "Track %s done!", fileName);
  Serial.println(errorMsg);
}

//Write to VS10xx register
//SCI: Data transfers are always 16bit. When a new SCI operation comes in 
//DREQ goes low. We then have to wait for DREQ to go high again.
//XCS should be low for the full duration of operation.
void Mp3WriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte){
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
  digitalWrite(MP3_XCS, LOW); //Select control

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x02); //Write instruction
  SPI.transfer(addressbyte);
  SPI.transfer(highbyte);
  SPI.transfer(lowbyte);
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  digitalWrite(MP3_XCS, HIGH); //Deselect Control
}

//Read the 16-bit value of a VS10xx register
unsigned int Mp3ReadRegister (unsigned char addressbyte){
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
  digitalWrite(MP3_XCS, LOW); //Select control

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x03);  //Read instruction
  SPI.transfer(addressbyte);

  char response1 = SPI.transfer(0xFF); //Read the first byte
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  char response2 = SPI.transfer(0xFF); //Read the second byte
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete

  digitalWrite(MP3_XCS, HIGH); //Deselect Control

  int resultvalue = response1 << 8;
  resultvalue |= response2;
  return resultvalue;
}

//Set VS10xx Volume Register
void Mp3SetVolume(unsigned char leftchannel, unsigned char rightchannel){
  Mp3WriteRegister(SCI_VOL, leftchannel, rightchannel);
}

void setVolumeFromPotenciometro(){
  int sensorValue= analogRead(pot_pin[0]); 
  int volumeValue= map(sensorValue,1024,0,127,0);
  Mp3SetVolume(volumeValue, volumeValue);
}



