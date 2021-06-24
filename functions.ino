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
        Serial.print("• "+supported_G_codes[i]);
        Serial.print("\n");
      }
      break;
    case 2:
      Serial.print("Only the following types of 'M' G-code are allowed:\n");
      for(byte i=0; i < sizeof(supported_M_codes)/sizeof(String); i++){
        Serial.print("• "+supported_M_codes[i]);
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
      Serial.println("Aborted - movement would be outside X-range (min = "+String(X_MIN)+"mm, max = "+String(X_MAX)+"mm)");
      break;
    case 9:
      Serial.println("Aborted - movement would be outside Y-range (min = "+String(Y_MIN)+"mm, max = "+String(Y_MAX)+"mm)");
      break;
    case 10:
      Serial.println("Aborted - movement would be outside Z-range (min = "+String(Z_MIN)+"mm, max = "+String(Z_MAX)+"mm)");
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
      Serial.println("Cannot set position outside X-range (min = "+String(X_MIN)+"mm, max = "+String(X_MAX)+"mm)");
      break;
    case 18:
      Serial.println("Cannot set position outside Y-range (min = "+String(Y_MIN)+"mm, max = "+String(Y_MAX)+"mm)");
      break;
    case 19:
      Serial.println("Cannot set position outside Z-range (min = "+String(Z_MIN)+"mm, max = "+String(Z_MAX)+"mm)");
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
      Serial.println("M120 does not need any further input");
      break;
    case 27:
      Serial.println("M121 does not need any further input");
      break;
    case 30:
      Serial.println("Step sizes must be positive");
      break;
    case 31:
      Serial.println("Missing input for G100");
      break;
    case 32:
      Serial.println("Aborted - X speed value lies outside the allowed range (min = "+String(X_SPEED_MIN)+"mm/s, max = "+String(X_SPEED_MAX)+")");
      break;
    case 33:
      Serial.println("Aborted - Y speed value lies outside the allowed range (min = "+String(Y_SPEED_MIN)+"mm/s, max = "+String(Y_SPEED_MAX)+")");
      break;
    case 34:
      Serial.println("Aborted - Z speed value lies outside the allowed range (min = "+String(Z_SPEED_MIN)+"mm/s, max = "+String(Z_SPEED_MAX)+")");
      break;
    case 35:
      Serial.println("Missing input for G101");
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
   * durch Ändern dieser Variable auf false abgebrochen werden. Dies
   * passiert logischerweise nicht, wenn die Ausführung lediglich
   * pausiert wird (also error(100)).
   */
  if(nmbr!=101){
    macro_ok = false;
  }
}

//***********************************************************************************************************

void run_input(String str){
  /* Sortiert serielle Eingabe nach 'G'-G-Code und 'M'-G-Code. In
   * jenen Fällen wird der erste char gelöscht, und der Rest des
   * Strings an die entsprechende 'G'- bzw. 'M'-Identifikations-
   * funktion gegeben (siehe "gcode.ino").
   * Ist die Eingabe nichts davon, wird ein Fehler ausgegeben.
   */
  String msg = remove_spaces(str);
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
  if (isTriggered(pin)){
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
   *  wenn erfolglos: 'error' & aktuelle Motorpositionen, -stepsizes, -geschw.
   *  zurück
   */
  if(val){
    Serial.println("ok");
  }else{
    Serial.println("error");
  }
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
  Serial.print(Z_SPEED);
  Serial.print("|");
  Serial.println(MODE_SECURE);
}

//***********************************************************************************************************

void check_interrupt(bool motors_moving){
  /* Diese Funktion wird in "movements.ino" und in der main loop genutzt,
   *  um zu überprüfen, ob stop-, pause- oder andere Aufrufe über
   *  die serielle Schnittstelle empfangen werden.
   *  Dementsprechend wird hier reagiert und ggf. gewartet.
   */
  if(Serial.available()>0){
    String msg = Serial.readStringUntil('\n');
    
    if(msg.equals("stop")){
      /* Wenn"stop" empfangen wurde, setze READY auf false (passiert mit "error(100);)
       *  und kehre zurück:
       */
      error(100);
      return;
    }
    
    else if(msg.equals("pause")){
      /* Wenn "pause" empfangen wurde, gebe zunächst eine Message aus...
       */
      error(101);
      msg = " ";
      /* ... und gehe dann in eine while-Schleife, solange bis erneut "pause" empfangen wird,
       *  der Prozess also fortgesetzt wird:
       */
      while(!msg.equals("pause")){
        /* Während der Prozess pausiert ist, akzeptiert der Arduino nur:
         *  "M114" oder "m114" (gebe Motorstatus aus) 
         *  "stop" (breche gesamten Druckprozess ab)
         *  "G100..." oder "g100..." (Geschw.-Änderung)
         */
        delay(0.01);
        msg = Serial.readStringUntil('\n');
        msg = remove_spaces(msg);
        if(msg.equals("M114") || msg.equals("m114")){
          M114();
        }
        else if(msg.equals("stop")){
          error(100);
          return;
        }
        else if(msg.indexOf("G100")>=0 || msg.indexOf("g100")>=0){
          run_input(msg);
        }
      }
      Serial.println("...process continued!");
    }
    
    else{
      if(motors_moving){
        /* Falls sich die Motoren aktuell bewegen, ignoriere
         *  die Eingabe...
         */
        Serial.println("Arduino not ready yet!");
      }else{
        /* .... Andernfalls führe die Eingabe aus:
         */
        run_input(msg);
      }
    }
  }
}

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

bool isTriggered(int pin){
  if(ENDSTOP_INVERTED){
    if(digitalRead(pin)==LOW){
      return true;
    }else if(digitalRead(pin)==HIGH){
      return false;
    }
  }else{
    if(digitalRead(pin)==LOW){
      return false;
    }else if(digitalRead(pin)==HIGH){
      return true;
    }
  }
}
