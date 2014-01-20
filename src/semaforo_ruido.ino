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
    Fecha:20-ENE-2014
    Version: 0.80
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es
    
 -------------------
 */
#define delay_between_last_activation 8000 //tiempo en milisegundos de retardo desde la ultima activacion hasta que se pueda ejecutar la siguiente evaluacion
                         

#define numberOfLevels 4   //Numero de niveles de sonido
#define DOUBLE_ORANGE 0   // TO-DO: explicar 

int mic_pins[] = {A0, A1} ;              //Define en que pins estan conectadas las entradas de micro
int pot_pin[] = {A4,A2,A3};  //Define en que entradas estan las entradas de los 3 potenciometros
int rele[]={A5,4,5,10};       //Define los pins donde estan conectados los 4 reles




int threshold[]={0,20,40,60};
char buffer_formato_serial[50];

boolean levelsActivated[]={1,0,0,0}; //recoge el estado de los 4 reles 

unsigned long last_activate_time;

// Esta rutina se ejecuta al inicio, y cada vez que pulsamos RESET
void setup() {   
  Serial.begin(57600); //Use serial for debugging 
  Serial.println("MP3 Testing");  
  init_mp3_player();
 

  pinMode(A0, INPUT); 
  pinMode(A1, INPUT);
  init_led_handler();
}

// La rutina "loop" se ejecuta una y otra vez indefinidamente :
void loop() {

  float amplitudeAvg=   getAmplitudeAvg();
  
  //Si ha pasado el tiempo delay_between_last_activation, evaluamos la AmplitudMedia
  if ((millis() - last_activate_time) > delay_between_last_activation) evaluateAmplitude(amplitudeAvg);

  //mostramos resultados en Serial
  printThresholds();
  printLevels();
  printSound(amplitudeAvg);
  
}


void printSound(long amplitudeAvg){
  Serial.print(getNoiseDb(amplitudeAvg));
  Serial.println("dB");


}
void printThresholds(){
  for (int i=0;i<numberOfLevels;i++){
    Serial.print(getNoiseDb(threshold[i]));
    Serial.print("dB ");
  }
  
}

void printLevels(){
  for (int i=0;i<numberOfLevels;i++){
    Serial.print((levelsActivated[i])?"I":"O");
    Serial.print(" ");
  }

  
}

