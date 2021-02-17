//========================================================================================
//======================================= Functions ======================================
//========================================================================================
void error(byte nmbr){
  /* Diese Funktion beinhaltet die Fehlerbeschreibungen, kategorisiert nach Nummern.
   * Je nach übergebenes Argument kann so an der entsprechenden Stelle im Code die
   * richtige Fehlermeldung ausgegeben werden.
   */
  switch(nmbr){
    case 1:
      Serial.print("Only the following types of 'G' G-code are allowed:\n");
      for(byte i=0; i < sizeof(supported_G_codes)/sizeof(String); i++){
        Serial.print("\\t• "+supported_G_codes[i]);
        Serial.print("\n");
      }
      break;
    case 2:
      Serial.print("Only the following types of 'M' G-code are allowed:\n");
      for(byte i=0; i < sizeof(supported_M_codes)/sizeof(String); i++){
        Serial.print("\\t• "+supported_M_codes[i]);
        Serial.print("\n");
      }
      break;
    case 3:
      Serial.println("Check your input. Only G-code allowed!");
      arduino_ready(false);
      break;
    case 4:
      Serial.println("Wrong input for X");
      break;
    case 5:
      Serial.println("Wrong input for Y");
      break;
    case 6:
      Serial.println("Wrong input for Z");
      break;
    case 7:
      Serial.println("Wrong input for S");
      break;
    case 8:
      Serial.println("Aborted - movement would be outside x-range");
      break;
    case 9:
      Serial.println("Aborted - movement would be outside y-range");
      break;
    case 10:
      Serial.println("Aborted - movement would be outside z-range");
      break;
    case 11:
      Serial.println("Missing input for G92");
      break;
    case 12:
      Serial.println("Missing input for M92");
      break;
    case 13:
      Serial.println("Missing input for G0");
      break;
    case 14:
      Serial.println("No macro defined");
      break;
    case 15:
      Serial.println("Wrong input for G28");
      break;
    case 16:
      Serial.println("Endstop was not hit on second approach!");
      break;
    case 17:
      Serial.println("Cannot set position outside x-range!");
      break;
    case 18:
      Serial.println("Cannot set position outside y-range!");
      break;
    case 19:
      Serial.println("Cannot set position outside z-range!");
      break;
    case 20:
      Serial.println("Wrong input for G90");
      break;
    case 21:
      Serial.println("Wrong input for G91");
      break;
    case 22:
      Serial.println("M17 does not need any further input");
      break;
    case 23:
      Serial.println("M18 does not need any further input");
      break;
    case 24:
      Serial.println("M114 does not need any further input");
      break;
    case 25:
      Serial.println("M119 does not need any further input");
      break;
    case 26:
      Serial.println("Wrong input for G101");
      break;
    case 27:
      Serial.println("Wrong input for G102");
      break;
    case 28:
      Serial.println("Missing input for G101");
      break;
    case 29:
      Serial.println("Missing input for G102");
      break;
    case 30:
      Serial.println("Step sizes must be positive");
      break;
    case 31:
      Serial.println("Missing input for G100");
      break;
    case 100:
      Serial.println("PRINTING PROCESS STOPPED!");
      READY = false;
      break;
    case 101:
      Serial.println("Process paused...");
      break;
    default:
      Serial.println("Error");
      break;
  }
  /* Wenn es zu einem Fehler kommt, muss auch das ggf. aufgerufene
   * Macro beendet werden. Bei einer Makro-Ausführung wird stets
   * überprüft, ob 'macro_ok' = true ist, daher kann die Ausführung
   * durch Ändern dieser Variable auf false abgebrochen werden.
   */
  macro_ok = false;
}

//***********************************************************************************************************

void run_input(String str){
  /* Sortiert serielle Eingabe nach 'G'-G-Code und 'M'-G-Code. In
   * jenen Fällen wird der erste char gelöscht, und der Rest des
   * Strings an die entsprechende 'G'- bzw. 'M'-Identifikations-
   * funktion gegeben (siehe "gcode.ino").
   * Ist die Eingabe nichts davon, wird ein Fehler ausgegeben.
   */
  String msg = str;
  while(msg[0] == ' '){
    msg.remove(0,1);
  }
  switch (msg[0]){
    case 'g':
    case 'G':
      G_gcode(msg);
      break;
    case 'm':
    case 'M':
      M_gcode(msg);
      break;
    default:
      error(3);
      break;
  }
}

//***********************************************************************************************************

String btn_state_str(int pin){
  /* Diese Funktion legt fest, bei welchem state eines Endstops
   *  "triggered" oder "open" ausgegeben wird.
   */
  if (digitalRead(pin)==HIGH){
    return "triggered";
  }else{
    return "open";
  }
}

//***********************************************************************************************************

void arduino_ready(bool val){
  /* Diese Funktion wird nach jeder Ausführung einer Eingabe über die serielle
   *  Schnnitstelle aufgerufen, unabbhängig davon, ob die Durchführung erfolgreich
   *  oder nicht erfolgreich war (wird über das Argument festgelegt). 
   *  Wenn erfolgreich: gebe 'ok' & aktuelle Motorpositionen, -stepsizes, -geschw.
   *  zurück
   *  wenn erfolglos: 'error'
   */
  if(val){
    Serial.println("ok");
    Serial.print(X_POS);
    Serial.print("|");
    Serial.print(Y_POS);
    Serial.print("|");
    Serial.print(Z_POS);
    Serial.print("|");
    Serial.print(X_STEP_SIZE);
    Serial.print("|");
    Serial.print(Y_STEP_SIZE);
    Serial.print("|");
    Serial.print(Z_STEP_SIZE);
    Serial.print("|");
    Serial.print(X_SPEED);
    Serial.print("|");
    Serial.print(Y_SPEED);
    Serial.print("|");
    Serial.println(Z_SPEED);
  }else{
    Serial.println("error");
  }
}

//***********************************************************************************************************

void check_interrupt(){
  /* Diese Funktion wird in "movements.ino" genutzt, um während einer
   *  Motorbewegung zu überprüfen, ob stop- oder pause-Aufrufe über
   *  die serielle Schnittstelle empfangen werden.
   *  Dementsprechend wird hier reagiert und ggf. gewartet.
   */
  if(Serial.available()>0){
    String temp = Serial.readStringUntil('\n');
    if(temp.equals("stop")){
      error(100);
      return;
    }else if(temp.equals("pause")){
      error(101);
      temp = " ";
      while(!temp.equals("pause")){
        delay(0.01);
        temp = Serial.readStringUntil('\n');
        if(temp.equals("M114") || temp.equals("m114")){
          M114();
        }
      }
      Serial.println("Continue process");
    }else{
      Serial.println("Arduino not ready yet!");
    }
  }
}

//***********************************************************************************************************

/*void update_POS(bool xmove,bool ymove,bool zmove,float xdis,float ydis,float zdis){
  /* Diese Funktion wird per loop während einer Bewegung aufgerufen und aktualisiert
   *  stetig die in den Variablen X_POS, Y_POS und Z_POS gespeicherten Positionen
   *  der Motoren.
   *  Die booleans geben an, welche Variablen, und die floats, mit welchen
   *  Werten die Variablen geändert werden.
   *//*
  if(xmove){
    X_POS += xdis;
  }
  if(ymove){
    Y_POS += ydis;
  }
  if(zmove){
    Z_POS += zdis;
  }
}*/

//***********************************************************************************************************

int indexOfMin(float arry[3]){
  /* Diese Funktion gibt den Index des kleinsten Elements aus dem Array 'arry' zurück.
   *  'arry' darf nur 3 Elemente enthalten
   */
  int ind=-1;
  int l=3;
  float tmp=10000000;
  for(int i=0;i<l;i++){
    if(arry[i]<tmp){
      tmp = arry[i];
      ind = i;
    }
  }
  return ind;
}

//***********************************************************************************************************

/*int indexOfMax(float arry[3]){
  /* Diese Funktion gibt den Index des größten Elements aus dem Array 'arry' zurück.
   *  'arry' darf nur 3 Elemente enthalten
   *//*
  int ind=-1;
  int l=3;
  float tmp=-10000000;
  for(int i=0;i<l;i++){
    if(arry[i]>tmp){
      tmp = arry[i];
      ind = i;
    }
  }
  return ind;
}*/
