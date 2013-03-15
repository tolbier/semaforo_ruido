/*
 relay_handler
 --------------
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
    Archivo: relay_handler.ino
    Fecha:15-MAR-2013
    Version: 0.73
    Autor: Lluis Toyos - info@korama.es - KORAMA Soluciones Digitales - http://korama.es

    La funcion de este codigo es manejar los reles del Semaforo de Ruido.
    
 -------------------
 */
#include "Arduino.h"

void init_relay_handler(){
    for (int i=0; i<=3;i++) pinMode(rele[i], OUTPUT);  
}

void changeRele(){
  int tiempo = millis()/BLINKING_DELAY;
  //Si tiempo es par
  for (int i=0; i<=3;i++) digitalWrite(rele[i],(tiempo %8 )>=4);
    
}

void activateLevel(int level){
  Serial.print("Activado Nivel: ");
  Serial.println(level);
  
  for (int i=0;i<numberOfLevels;i++){
    boolean activado=false;
    
    
    
    activado=(level==i);
    if (!activado && (i==1)){
        activado=  (level==i+1); 
    }
    digitalWrite(rele[i],activado);
    levelsActivated[i]=activado;
  }
  if (PLAYMP3_BOOL) playMP3Level(level);
  
  last_activate_time=millis();
}
