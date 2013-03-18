/*
 semaforo_ruido
 ---------------
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
    Archivo: semaforo_ruido.ino
    Fecha:15-MAR-2013
    Version: 0.74
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es
    
 -------------------
 */
#define delay_between_last_activation 8000 //tiempo en milisegundos de retardo desde la ultima activacion hasta que se pueda ejecutar la siguiente evaluacion
#define maxThreshold 110
#define minThreshold 60
#define plusMediumThreshold 12

#define PLAYMP3_BOOL 1 // 1- Reproduce sonidos a traves de la tarjeta MP3 Player Arduino Shell 
                       // 0 - NO Reproduce sonidos a traves de la tarjeta MP3 Player Arduino Shell 
#define MP3LOG 0           // Imprime en Serial algun registro de log durante la reproduccion MP3

#define BLINKING_ALERT 0   // Si se activa muestra las luces parpadeando un tiempo cuando hay un cambio de sonido
#define BLINKING_DELAY 100

#define numberOfLevels 4   //Numero de niveles de sonido

int mic_pin = A3;              //Define en que pin esta conectada la entrada de micro
int pot_pin[] = {  A0,A1,A2};  //Define en que entradas estan las entradas de los 3 potenciometros
int rele[]={3,A5,A4,5};       //Define los pins donde estan conectados los 4 reles


int threshold[]={0,20,40,60};
char buffer_formato_serial[50];

boolean levelsActivated[]={1,0,0,0}; //recoge el estado de los 4 reles 

unsigned long last_activate_time;

// Esta rutina se ejecuta al inicio, y cada vez que pulsamos RESET
void setup() {                
  init_mp3_player();
  init_numberOfFilesPerLevel();
  init_relay_handler();
}

// La rutina "loop" se ejecuta una y otra vez indefinidamente :
void loop() {

  long amplitudeAvg=  (long) getAmplitudeAvg();
  
  //Si ha pasado el tiempo delay_between_last_activation, evaluamos la AmplitudMedia
  if ((millis() - last_activate_time) > delay_between_last_activation) evaluateAmplitude(amplitudeAvg);

  //mostramos resultados en Serial
  printThresholds();
  printLevels();
  printSound(amplitudeAvg);
  
}


void printSound(long amplitudeAvg){
  int length = map(amplitudeAvg,0,512,0,160); 

  Serial.print(amplitudeAvg);
  Serial.print("-");
  for (int i = 0; i<= length; i++) Serial.print("*");
  Serial.println();


}
void printThresholds(){
  for (int i=0;i<numberOfLevels;i++){
    Serial.print(threshold[i]);
    Serial.print(" ");
  }
  
}

void printLevels(){
  for (int i=0;i<numberOfLevels;i++){
    Serial.print((levelsActivated[i])?"I":"O");
    Serial.print(" ");
  }

  
}

