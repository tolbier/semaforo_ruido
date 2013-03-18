/*
  mic_handler
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
    Archivo: mic_handler.ino
    Fecha:15-MAR-2013
    Version: 0.74
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es

    Este codigo se encarga de manejar los datos recibidos desde el Sensor de Sonido 
 -------------------
 */

const int numberOfSamples =256; // Numero de muestras que seran tomadas para calcular la amplitud en un periodo de tiempo
const int averagedOver= 128; //Numero de muestreos utilizados para calcular la media 
                                 //Cuanto mayor es el numero, mas tiempo tarda en reflejar cualquier aumento en el volumen de sonido


float getAmplitudeAvg() {
  
  float amplitudeAvg=0.0; 
  long sumOfReadings = 0;
  for (int i=0; i<numberOfSamples; i++) { //take many readings and average them
    int reading = analogRead(mic_pin);    //take a reading
    sumOfReadings += reading;             //add to the total
  }
  float avgReadings = sumOfReadings*2/(float)numberOfSamples;
  

  //calculate running average
  amplitudeAvg=(((averagedOver-1)*amplitudeAvg)+avgReadings)/ ((float) averagedOver);
  
  return amplitudeAvg;

  
}



void evaluateAmplitude(long amplitudeAvg){
  int i = numberOfLevels -1;
  int nivelActivacion=0;

  setThresholdPotenciometro();

  do{
    if (amplitudeAvg> threshold[i]){
       
      nivelActivacion=i;
      break;
    }   
    i--;
  }while (i>=0);
  activateLevel(nivelActivacion);
}
void setThresholdPotenciometro(){

  int thresholdMaxValue= map(analogRead(pot_pin[2]),1024,0,40,(maxThreshold-5)*2-40);
  int thresholdMinValue= map(analogRead(pot_pin[1]),0,1024,40,(minThreshold-1)*2-40);
  
  threshold[3]=thresholdMaxValue;
  threshold[2]= thresholdMinValue + plusMediumThreshold;
  threshold[1]=thresholdMinValue ;
  
}
