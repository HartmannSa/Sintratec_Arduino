//=========================================================================================================================
//==================================== G-Codes ============================================================================
//=========================================================================================================================

void G_gcode(String str){ // input: serial input string, but first character removed
  /* Wurde von der 'run_input'-Funktion ein "G" oder "g" als erster char erkannt, wird
   *  diese Funktion aufgerufen. Zunächst löscht sie alle Leerzeichen und überprüft,
   *  ob auf das "G" (oder "g") nur Ziffern folgen. Dementsprechend wird der Rest-String
   *  an die jeweilige G-Code-Funktion übergeben.
   *  Andernfalls: Error
   */
  String msg = remove_spaces(str);
  msg.remove(0,1);
  if(!isDigit(msg[0])){
    error(1);
    arduino_ready(false);
    return; 
  }
  byte ind = 0;
  while (isDigit(msg[ind])){
    ind++;
  }
  int Gnumber = msg.substring(0,ind).toInt();
  msg = remove_spaces(msg.substring(ind));
  switch (Gnumber){
    case 0:
      G0(msg);
      break;
    case 28:
      G28(msg);
      break;
    case 90:
      if(msg.length()==0){
        G90();
      }else{
        error(20);
        arduino_ready(false);
      }
      break;
    case 91:
      if(msg.length()==0){
        G91();
      }else{
        error(21);
        arduino_ready(false);
      }
      break;
    case 92:
      G92(msg);
      break;
    case 100:
      G100(msg);
      break;
    default:
      error(1);
      arduino_ready(false);
      break;
  }
}

//***********************************************************************************************************

void M_gcode(String str){ // input: serial input string, but first character removed
  /* Wurde von der 'run_input'-Funktion ein "M" oder "m" als erster char erkannt, wird
   *  diese Funktion aufgerufen. Zunächst löscht sie alle Leerzeichen und überprüft,
   *  ob auf das "M" (oder "m") nur Ziffern folgen. Dementsprechend wird der Rest-String
   *  an die jeweilige M-Code-Funktion übergeben.
   *  Andernfalls: Error
   */
  String msg = remove_spaces(str);
  msg.remove(0,1);
  if(!isDigit(msg[0])){
    error(2);
    arduino_ready(false);
    return; 
  }
  byte ind = 0;
  while (isDigit(msg[ind])){
    ind++;
  }
  int Mnumber = msg.substring(0,ind).toInt();
  msg = remove_spaces(msg.substring(ind));
  switch (Mnumber){
    case 17:
      if(msg.length()==0){
        M17();
      }else{
        error(22);
        arduino_ready(false);
      }
      break;
    case 18:
      if(msg.length()==0){
        M18();
      }else{
        error(23);
        arduino_ready(false);
      }
      break;
    case 92:
      M92(msg);
      break;
    case 114:
      if(msg.length()==0){
        M114();
      }else{
        error(24);
        arduino_ready(false);
      }
      break;
    case 119:
      if(msg.length()==0){
        M119();
      }else{
        error(25);
        arduino_ready(false);
      }
      break;
    case 810 ... 819:
      M810_819(Mnumber-810,msg);
      break;
    default:
      error(2);
      arduino_ready(false);
      break;
  }
}

//=========================================================================================================================================
//            EACH G-CODE FUNCTION:
//=========================================================================================================================================


void G0(String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G0...' erkannt wurde. 'msg' enthält nur den Rest des Strings hinter
   *  'G0'.
   *  Wenn der String leer oder die Eingabe für X, Y, Z bzw. F falsch ist (wird per Funktion 'xyz_ident()' überprüft - siehe
   *  "string_editing.ino"), wird der entsprechende Fehler ausgegeben und die Funktion abgebrochen.
   *  ...
   */
  if(msg.length()==0){
    error(13);
    arduino_ready(false);
    return;
  }
  float xyz_init[3] = {X_MAX_MOVE*10,Y_MAX_MOVE*10,Z_MAX_MOVE*10};
  float xyz[3] = {X_MAX_MOVE*10,Y_MAX_MOVE*10,Z_MAX_MOVE*10};
  bool ind_changed[3] = {false,false,false};
  bool input_ok = xyz_ident(msg,xyz);
  if(!input_ok){
    arduino_ready(false);
    return;
  }
  /*  ...
   *  Andernfalls wird überprüft, welche Eingaben gemacht wurden, d.h. welche Motoren bewegt werden sollen und ob die Geschw. geändert
   *  werden soll. Diese Informationen werden in 'ind_changed[4]' gespeichert.
   *  ...
   */
  for(int i=0;i<3;i++){
    if(xyz[i]!=xyz_init[i]){
      ind_changed[i]=true;
    }
  }
  /*  ...
   *  Dann wird je nach Positionierungsmodus (relativ oder absolut) der Bewegungsbefehl an die entsprechenden Motoren
   *  gesendet (siehe Funktionen in "movements.ino").
   */
  if(ABSOLUTE_POS){
    if((xyz[0]<X_MIN || xyz[0]>X_MAX)&&ind_changed[0]){
      error(8);
      arduino_ready(false);
      return;
    }else if((xyz[1]<Y_MIN || xyz[1]>Y_MAX)&&ind_changed[1]){
      error(9);
      arduino_ready(false);
      return;
    }else if((xyz[2]<Z_MIN || xyz[2]>Z_MAX)&&ind_changed[2]){
      error(10);
      arduino_ready(false);
      return;
    }else{
      /*Serial.println(ind_changed[0]);
      Serial.println(xyz[0]-X_POS);
      Serial.println(ind_changed[1]);
      Serial.println(xyz[1]-Y_POS);
      Serial.println(ind_changed[2]);
      Serial.println(xyz[2]-Z_POS);*/
      move_steppers(xyz[0]-X_POS,xyz[1]-Y_POS,xyz[2]-Z_POS,ind_changed[0],ind_changed[1],ind_changed[2]);
    }
  }else{
    if((X_POS+xyz[0]<X_MIN || X_POS+xyz[0]>X_MAX)&&ind_changed[0]){
      error(8);
      arduino_ready(false);
      return;
    }else if((Y_POS+xyz[1]<Y_MIN || Y_POS+xyz[1]>Y_MAX)&&ind_changed[1]){
      error(9);
      arduino_ready(false);
      return;
    }else if((Z_POS+xyz[2]<Z_MIN || Z_POS+xyz[2]>Z_MAX)&&ind_changed[2]){
      error(10);
      arduino_ready(false);
      return;
    }else{
      move_steppers(xyz[0],xyz[1],xyz[2],ind_changed[0],ind_changed[1],ind_changed[2]);
    }
  }
  arduino_ready(true);
}

//***********************************************************************************************************

void G28(String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G28...' erkannt wurde. 'msg' enthält nur den Rest des Strings hinter
   *  'G28'.
   *  Wenn der Rest-String leer ist, werden alle Achsen gehomed.
   *  Andernfalls wird der Rest-String durchsucht und beim Fund entsprechender Zeichen nur die entsprechenden Achsen gehomed. Sollten andere
   *  Zeichen als x, y und z gefunden werden, wird ein Fehler ausgegeben und keine Achse gehomed.
   */
  bool home_success = false;
  if(msg.length()==0){
    home_success = home_axis('x');
    if(!home_success){
      arduino_ready(false);
      return;
    }
    home_success = home_axis('y');
    if(!home_success){
      arduino_ready(false);
      return;
    }
    home_success = home_axis('z');
    if(!home_success){
      arduino_ready(false);
      return;
    }
  }else{
    bool xhome = false;
    bool yhome = false;
    bool zhome = false;
    byte ind = 0;
    byte l = msg.length();
    while (ind<l){
      if((msg[ind]=='x' || msg[ind]=='X')&&xhome==false){
        xhome=true;
      }else if((msg[ind]=='y' || msg[ind]=='Y')&&yhome==false){
        yhome=true;
      }else if((msg[ind]=='z' || msg[ind]=='Z')&&zhome==false){
        zhome=true;
      }else{
        error(15);
        arduino_ready(false);
        return;
      }
      ind++;
    }
    if(xhome){
      home_success = home_axis('x');
      if(!home_success){
        arduino_ready(false);
        return;
      }
    }
    if(yhome){
      home_success = home_axis('y');
      if(!home_success){
        arduino_ready(false);
        return;
      }
    }
    if(zhome){
      home_success = home_axis('z');
      if(!home_success){
        arduino_ready(false);
        return;
      }
    }
  }
  arduino_ready(true);
}

//***********************************************************************************************************

void G90(){ // absolute positioning
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G90' erkannt wurde.
   * Ändert den Positionierungsmodus zu absolut.
   */
  Serial.println("Absolute positioning");
  ABSOLUTE_POS = true;
  arduino_ready(true);
}

//***********************************************************************************************************

void G91(){ // relative positioning
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G91' erkannt wurde.
   * Ändert den Positionierungsmodus zu relativ.
   */
  Serial.println("Relative positioning");
  ABSOLUTE_POS = false;
  arduino_ready(true);
}

//***********************************************************************************************************

void G92(String msg){ // set position
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G91' erkannt wurde. 'msg' enthält den
   *  Rest des Strings hinter "G92". Dieser darf nicht leer sein, ansonsten: error.
   *  Wenn die Eingabe für X, Y, bzw. Z falsch ist (wird per Funktion 'xyz_ident()' überprüft - siehe
   *  "string_editing.ino"), wird der entsprechende Fehler ausgegeben und die Funktion abgebrochen.
   *  ...
   */
  if(msg.length()==0){
    error(11);
    arduino_ready(false);
    return;
  }
  float xyz_init[3] = {X_POS,Y_POS,Z_POS};
  float xyz[3] = {X_POS,Y_POS,Z_POS};
  bool ind_changed[3] = {false,false,false};
  bool input_ok = xyz_ident(msg,xyz);
  if(!input_ok){
    arduino_ready(false);
    return;
  }
  /*  ...
   *  Dann wird überprüft, welche Eingaben gemacht wurden, d.h. welche Motorenpositionen geändert werden
   *  sollen. Diese Informationen werden in 'ind_changed[3]' gespeichert.
   *  ...
   */
  for(int i=0;i<3;i++){
    if(xyz[i]!=xyz_init[i]){
      ind_changed[i]=true;
    }
  }
  /*  ...
   *  Nun wird überprüft, ob die angegebenen Positionsänderungen zulässig sind, also im Bereich der
   *  maximalen Verfahrwege liegen.
   *  ...
   */
  if((xyz[0]<X_MIN || xyz[0]>X_MAX) && ind_changed[0]){
    error(17);
    arduino_ready(false);
    return;
  }if((xyz[1]<Y_MIN || xyz[1]>Y_MAX) && ind_changed[1]){
    error(18);
    arduino_ready(false);
    return;
  }if((xyz[2]<Z_MIN || xyz[2]>Z_MAX) && ind_changed[2]){
    error(19);
    arduino_ready(false);
    return;
  }
  /*  ...
   *  Zuletzt werden die Motorpositionen abhängig von 'ind_changed[3]' gesetzt.
   */
  for(int i=0;i<3;i++){
    if(ind_changed[i]){
      switch (i){
        case 0:
          X_POS = xyz[0];
          break;
        case 1:
          Y_POS = xyz[1];
          break;
        case 2:
          Z_POS = xyz[2];
          break;
        default:
          break;
      }
    }
  }
  if(ind_changed[0] || ind_changed[1] || ind_changed[2]){  
    Serial.println("Position set");
    arduino_ready(true);
  }else{
    Serial.println("Motors are already at those positions");
    arduino_ready(true);
  }
}

//***********************************************************************************************************

void G100(String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G100' erkannt wurde. 'msg' enthält den
   *  Rest des Strings hinter "G100". Dieser darf nicht leer sein, ansonsten: error.
   *  Wenn die Eingabe für X, Y, bzw. Z falsch ist (wird per Funktion 'xyz_ident()' überprüft - siehe
   *  "string_editing.ino"), wird der entsprechende Fehler ausgegeben und die Funktion abgebrochen.
   *  ...
   */
  if(msg.length()==0){
    error(31);
    arduino_ready(false);
    return;
  }
  float xyz_init[3] = {X_STEP_SIZE,Y_STEP_SIZE,Z_STEP_SIZE};
  float xyz[3] = {X_STEP_SIZE,Y_STEP_SIZE,Z_STEP_SIZE};
  bool ind_changed[3] = {false,false,false};
  bool input_ok = xyz_ident(msg,xyz);
  if(!input_ok){
    arduino_ready(false);
    return;
  }
  /*  ...
   *  Ansonsten wird überprüft, welche Eingaben gemacht wurden, d.h. welche Motor-Stepsizes geändert werden
   *  sollen. Diese Informationen werden in 'ind_changed[3]' gespeichert.
   *  ...
   */
  for(int i=0;i<3;i++){
    if(xyz[i]!=xyz_init[i]){
      ind_changed[i]=true;
    }
  }
  /* Zuletzt werden die Stepsizes angepasst:
   */
  for(int i=0;i<3;i++){
    if(ind_changed[i]){
      switch (i){
        case 0:
          if(xyz[0]>0){
            X_SPEED = xyz[0];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        case 1:
          if(xyz[1]>0){
            Y_SPEED = xyz[1];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        case 2:
          if(xyz[2]>0){
            Z_SPEED = xyz[2];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        default:
          break;
      }
    }
  }
  if(ind_changed[0] || ind_changed[1] || ind_changed[2]){  
    Serial.println("Speeds set");
    arduino_ready(true);
  }else{
    Serial.println("Speeds are already set to those values");
    arduino_ready(true);
  }
}
/*
//***********************************************************************************************************

void G101(String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G101' erkannt wurde. 'msg' enthält den
   *  Rest des Strings hinter "G101". Dieser darf nicht leer sein, ansonsten: error.
   *  Es wird überprüft, ob 'msg' das richtige Format, also 's' oder 'S' gefolgt von einem float value hat.
   *  Ist dies der Fall, wird die X und Y Geschwindigkeit (X_SPEED und Y_SPEED) dem Wert angepasst, ansonsten
   *  error.
   
  if(msg.length()==0){
    error(28);
    arduino_ready(false);
    return;
  }
  if(msg[0]!='s' && msg[0]!='S'){
    error(26);
    arduino_ready(false);
    return;
  }
  bool dot_found = false;
  msg = msg.substring(1);
  for(int i=0;i<msg.length();i++){
    if(!isDigit(msg[i]) && msg[i]!='.'){
      error(7);
      arduino_ready(false);
      return;
    }else if(msg[i]=='.'){
      if(dot_found){
        error(7);
        arduino_ready(false);
        return;
      }
      dot_found = true;
    }
  }
  float val = msg.toFloat();
  X_SPEED = val;
  Y_SPEED = val;
  Serial.println("X/Y speed changed");
  arduino_ready(true);
}

//***********************************************************************************************************

void G102(String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'G102' erkannt wurde. 'msg' enthält den
   *  Rest des Strings hinter "G102". Dieser darf nicht leer sein, ansonsten: error.
   *  Es wird überprüft, ob 'msg' das richtige Format, also 's' oder 'S' gefolgt von einem float value hat.
   *  Ist dies der Fall, wird die Z Geschwindigkeit (Z_SPEED) dem Wert angepasst, ansonsten: error.
   
  if(msg.length()==0){
    error(29);
    arduino_ready(false);
    return;
  }
  if(msg[0]!='s' && msg[0]!='S'){
    error(27);
    arduino_ready(false);
    return;
  }
  bool dot_found = false;
  msg = msg.substring(1);
  for(int i=0;i<msg.length();i++){
    if(!isDigit(msg[i]) && msg[i]!='.'){
      error(7);
      arduino_ready(false);
      return;
    }else if(msg[i]=='.'){
      if(dot_found){
        error(7);
        arduino_ready(false);
        return;
      }
      dot_found = true;
    }
  }
  float val = msg.toFloat();
  Z_SPEED = val;
  Serial.println("Z speed changed");
  arduino_ready(true);
}
*/
//***********************************************************************************************************

void M17(){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M17' erkannt wurde.
   * Gibt Strom auf die Motoren.
   */
  digitalWrite(X_ENABLE_PIN,LOW);
  digitalWrite(Y_ENABLE_PIN,LOW);
  digitalWrite(Z_ENABLE_PIN,LOW);
  Serial.println("Steppers enabled");
  arduino_ready(true);
}

//***********************************************************************************************************

void M18(){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M18' erkannt wurde.
   * Macht Motoren stromlos.
   */
  digitalWrite(X_ENABLE_PIN,HIGH);
  digitalWrite(Y_ENABLE_PIN,HIGH);
  digitalWrite(Z_ENABLE_PIN,HIGH);
  Serial.println("Steppers disabled");
  arduino_ready(true);
}

//***********************************************************************************************************

void M92(String msg){ // set step sizes
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M92' erkannt wurde. 'msg' enthält den
   *  Rest des Strings hinter "M92". Dieser darf nicht leer sein, ansonsten: error.
   *  Wenn die Eingabe für X, Y, bzw. Z falsch ist (wird per Funktion 'xyz_ident()' überprüft - siehe
   *  "string_editing.ino"), wird der entsprechende Fehler ausgegeben und die Funktion abgebrochen.
   *  ...
   */
  if(msg.length()==0){
    error(12);
    arduino_ready(false);
    return;
  }
  float xyz_init[3] = {X_STEP_SIZE,Y_STEP_SIZE,Z_STEP_SIZE};
  float xyz[3] = {X_STEP_SIZE,Y_STEP_SIZE,Z_STEP_SIZE};
  bool ind_changed[3] = {false,false,false};
  bool input_ok = xyz_ident(msg,xyz);
  if(!input_ok){
    arduino_ready(false);
    return;
  }
  /*  ...
   *  Ansonsten wird überprüft, welche Eingaben gemacht wurden, d.h. welche Motor-Stepsizes geändert werden
   *  sollen. Diese Informationen werden in 'ind_changed[3]' gespeichert.
   *  ...
   */
  for(int i=0;i<3;i++){
    if(xyz[i]!=xyz_init[i]){
      ind_changed[i]=true;
    }
  }
  /* Zuletzt werden die Stepsizes angepasst:
   */
  for(int i=0;i<3;i++){
    if(ind_changed[i]){
      switch (i){
        case 0:
          if(xyz[0]>0){
            X_STEP_SIZE = xyz[0];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        case 1:
          if(xyz[1]>0){
            Y_STEP_SIZE = xyz[1];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        case 2:
          if(xyz[2]>0){
            Z_STEP_SIZE = xyz[2];
          }else{
            arduino_ready(false);
            return;
          }
          break;
        default:
          break;
      }
    }
  }
  if(ind_changed[0] || ind_changed[1] || ind_changed[2]){  
    Serial.println("Step sizes set");
    arduino_ready(true);
  }else{
    Serial.println("Step sizes are already set to those values");
    arduino_ready(true);
  }
}

//***********************************************************************************************************

void M114(){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M114' erkannt wurde.
   * Gibt (aktuelle) Motoreigenschaften seriell zurück.
   */
  if(ABSOLUTE_POS){
    Serial.println("Positioning: absolute");
  }else{
    Serial.println("Positioning: relative");
  }
  Serial.println("current position [mm]: X = "+String(X_POS)+", Y = "+String(Y_POS)+", Z = "+String(Z_POS));
  Serial.println("min. positions [mm]: X = "+String(X_MIN)+", Y = "+String(Y_MIN)+", Z = "+String(Z_MIN));
  Serial.println("max. positions [mm]: X = "+String(X_MAX)+", Y = "+String(Y_MAX)+", Z = "+String(Z_MAX));
  Serial.println("Step sizes [steps/mm]: X = "+String(X_STEP_SIZE)+", Y = "+String(Y_STEP_SIZE)+", Z = "+String(Z_STEP_SIZE));
  Serial.println("X speed [mm/s]: "+String(X_SPEED));
  Serial.println("Y speed [mm/s]: "+String(Y_SPEED));
  Serial.println("Z speed [mm/s]: "+String(Z_SPEED));
  arduino_ready(true);
}

//***********************************************************************************************************

void M119(){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M119' erkannt wurde.
   * Gibt Endstop-Stati seriell zurück.
   * 'btn_state_str' gibt an, ob "triggered" oder "open" bei entsprechendem Status (HIGH oder LOW) geprinted
   * wird
   */
  Serial.print("X_MIN: ");
  Serial.println(btn_state_str(X_MIN_PIN));
  Serial.print("X_MAX: ");
  Serial.println(btn_state_str(X_MAX_PIN));
  Serial.print("Y_MIN: ");
  Serial.println(btn_state_str(Y_MIN_PIN));
  Serial.print("Y_MAX: ");
  Serial.println(btn_state_str(Y_MAX_PIN));
  Serial.print("Z_MIN: ");
  Serial.println(btn_state_str(Z_MIN_PIN));
  Serial.print("Z_MAX: ");
  Serial.println(btn_state_str(Z_MAX_PIN));
  arduino_ready(true);
}

//***********************************************************************************************************

void M810_819(int macroNumber, String msg){
  /* Diese Funktion wird ausgeführt, wenn in der seriellen Eingabe 'M810...', 'M811...', ..., 'M819...'
   *  erkannt wurde.
   *  Speichert übergebenen String als Makro mit der Nummer 'macroNumber' oder führt hinterlegtes Makro aus,
   *  falls der String 'msg' leer ist
   */
  byte l = msg.length();
  if(l!=0){                                 // saves macro
    macro[macroNumber] = msg;
    Serial.println("Macro M"+String(macroNumber)+" defined");
    arduino_ready(true);
  }else{                                    // executes macro
    String command = macro[macroNumber];
    byte cl = command.length();
    if(cl==0){
      /* Falls Makro ohne Inhalt bspw. durch "M810" ausgeführt werden soll: error */
      error(14);
      arduino_ready(false);
      return;
    }else{
      Serial.println("+ + + + + + + + Executing macro + + + + + + + + + +");
      byte ind = 0;
      byte from = 0;
      macro_ok = true;
      while(ind<=cl && macro_ok && READY){
        /* Durchsuche hinterlegtes Makro (= String) nach Trennzeichen '|' und führe
         *  einzelne G-Codes (= Substrings) aus.
         *  Abbruch wenn Fehler auftreten ('macro_ok' = false) oder Druckprozess ge-
         *  stoppt wird ('READY' = false)
         */
        if(command[ind]=='|' || ind==cl){
          run_input(command.substring(from,ind));
          from = ind+1;
        }
        ind++;
      }
      if(macro_ok){
        Serial.println("+ + + + + + + + Executing macro done  + + + + + + +");
      }else{
        Serial.println("+ + + + + + + + Executing macro aborted + + + + + +");
      }
    }
  }
}
