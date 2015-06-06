/*
  mic_handler
 ----------------- 
     This file is part of the Project  SEMAFORO_RUIDO OPEN_SOURCE
     - A Hardware+Software System to detect the noise levels and to sign them
     through a 4 colored LED system

    Copyright (C) 2014 Lluis Toyos

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

---------------    
    Archivo: mic_handler.ino
    Fecha:20-ENE-2014
    Version: 0.80
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es

    Este codigo se encarga de manejar los datos recibidos desde el Sensor de Sonido 
 -------------------
 */

#define SHORT_INTERVAL_MILLIS 200
#define INTERVAL_MILLIS 2000

const int numberOfSamples =256; // Numero de muestras que seran tomadas para calcular la amplitud en un periodo de tiempo
const int averagedOver= 8; //Numero de muestreos utilizados para calcular la media 
                                 //Cuanto mayor es el numero, mas tiempo tarda en reflejar cualquier aumento en el volumen de sonido
const int initialThresholdOffset = 0;

float readingTable[]={0.0 , 1.0, 10.0,20.0,40.0,74.0,100.0,150.0,310.0,420.0,500.0,535.0,1024.0};
float noiseDbTable[]={33.0, 70.0,78.0,84.4,88.0,91.5,93.0, 95.4, 97.5, 99.8, 102.0,102.7,130.0};




float getAmplitudeReading() {
  unsigned long initMillis = millis();
  unsigned long sumOfReadings = 0;
  float average = 0.0;
  float i = 0.0;
  while (millis()-initMillis< SHORT_INTERVAL_MILLIS){
    unsigned int reading =  max(analogRead(mic_pins[0]),analogRead(mic_pins[1]));     //take a reading

    i+=1.0;
    average= (average* (i-1.0) + (float)reading)/i;
 
  }

  average*=2.0;

  //calculate running average
  
  return average;
  
}

float getAmplitudeReadingMaxOnLastInterval() {
  unsigned long initMillis = millis();
  float maxReading = 0.0;
  while (millis()-initMillis< INTERVAL_MILLIS){
    float currentReading = getAmplitudeReading();
    if (currentReading>maxReading) maxReading = currentReading;
  }
  return maxReading;
  
}


void evaluateAmplitude(float amplitudeAvg){
  int i = numberOfLevels -1;
  int nivelActivacion=0;

  setThresholdPotenciometro();

  do{
    if (((int)amplitudeAvg)> threshold[i]){
       
      nivelActivacion=i;
      break;
    }   
    i--;
  }while (i>=0);
  activateLevel(nivelActivacion);
}
void setThresholdPotenciometro(){
  int thresholdMinValue= map(analogRead(pot_pin[1]),1024,0,1,512);
  int thresholdMaxValue= map(analogRead(pot_pin[2]),1024,0,1,512);
  if (thresholdMaxValue<thresholdMinValue) thresholdMaxValue = thresholdMinValue;
  
  threshold[3]=thresholdMaxValue;
  threshold[2]= (thresholdMinValue + thresholdMaxValue)/2;
  threshold[1]=thresholdMinValue ;
  
}

float getNoiseDb(long lreading){
  float reading = (float) lreading;
  int sizeReadingTable = sizeof(readingTable)/sizeof(float);
  int i=0;
  while (!(readingTable[i]>=reading)  && ! (i==(sizeReadingTable-1))) i++;
  
  if  (readingTable[i]>=reading){
    float cotMaxReading = readingTable[i] ;
    float cotMaxNoiseDb = noiseDbTable[i] ;
    float cotMinReading = 0.0;
    float cotMinNoiseDb = 0.0;
    if (i>0){
      cotMinReading = readingTable[i-1] ;
      cotMinNoiseDb = noiseDbTable[i-1];
    }else return noiseDbTable[0];
    return cotMinNoiseDb + (((reading - cotMinReading)/ (cotMaxReading - cotMinReading))* (cotMaxNoiseDb -cotMinNoiseDb));
    
    
  }
  return noiseDbTable[0];
    
    
}
