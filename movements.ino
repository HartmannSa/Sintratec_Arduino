//========================================================================================
//==================================== Movements =========================================
//========================================================================================

void move_steppers(float xdis, float ydis, float zdis, bool xmove, bool ymove, bool zmove){
  /* Bewegt die Motoren um den jeweiligen float 'xdis, 'ydis' und 'zdis', sofern zugehöriger
   *  boolean 'xmove', etc. true ist. */
  /* 
   *  Passe die Drehrichtungen entsprechend den Vorzeichen der Verfahrwege an:
   *  (ACHTUNG: Darauf achten, wie die Motoren auf dem RAMPS Board eingesteckt sind!)
   */
  int x_dir = -1;
  int y_dir = -1;
  int z_dir = -1;
  if(xdis<0){                       // TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Was ist vorwärts/rüvckwärts am Sintratec drucker? (Teste am Drucker!)
    digitalWrite(X_DIR_PIN,LOW);
  }else{
    digitalWrite(X_DIR_PIN,HIGH);
    x_dir = 1;
  }
  if(ydis<0){
    digitalWrite(Y_DIR_PIN,LOW);
  }else{
    digitalWrite(Y_DIR_PIN,HIGH);
    y_dir = 1;
  }
  if(zdis<0){
    digitalWrite(Z_DIR_PIN,LOW);
  }else{
    digitalWrite(Z_DIR_PIN,HIGH);
    z_dir = 1;
  }
  /* Berechne die zu machenden Steps eines Motors, speichere sie in 'xyz_steps[3]' und aktiviere
   *  diejenigen Motoren, welche verfahren sollen. Letzteres wird durch die übergebenen booleans
   *  xmove, ymove und zmove indiziert.
   */
  float xyz_steps[3] = {0,0,0};
  float xyz_dis[3] = {abs(xdis),abs(ydis),abs(zdis)};
  byte ind_MaxSteps = 0;
  if(xmove){
    digitalWrite(X_ENABLE_PIN,LOW);
    xyz_steps[0] = X_STEP_SIZE*xyz_dis[0];
  }
  if(ymove){
    digitalWrite(Y_ENABLE_PIN,LOW);
    xyz_steps[1] = Y_STEP_SIZE*xyz_dis[1];
  }
  if(zmove){
    digitalWrite(Z_ENABLE_PIN,LOW);
    xyz_steps[2] = Z_STEP_SIZE*xyz_dis[2];
  }
  /*
   * Nun werden Werte berechnet, welche für die Bewegungen (siehe weiter unten in der while Schleife)
   * relevant sind.
   * - xyz_delays[3] gibt jeweils die Zeit zwischen zwei "steps" eines Motors an, welche abgewartet werden muss,
   * damit die Verfahrgeschwindigkeit der jeweiligen vorgegebenen Geschwindigkeit (X_SPEED, Y_SPEED bzw. Z_SPEED)
   * entspricht. Motorbewegungen bestehen bei Stepper Motoren immer aus einzelnen Steps (google das).
   * - ind_MinDelay gibt den Index von xyz_delays[3] an, bei dem die kürzeste Wartezeit hinterlegt ist. Dies ist
   * wichtig für...
   * - x_timeAdd, y_timeAdd und z_timeAdd: Je nach Verhältnis der kürzesten Wartezeit und der jeweiligen Wartezeit
   * des entsprechenden Motors wird ein Summand definiert, welche in der while Schleife immer auf den entsprechenden
   * Counter des jeweiligen Motors aufaddiert wird:
   * - x_timeCnt, y_timeCnt und z_timeCnt: Sobald ein Wert >= 1 ist, wird ein Step des jeweiligen Motors durchgeführt. Damit
   * wird gewährleistet, dass sich die Achsen in der while Schleife (weiter unten) alle mit ihrer individuellen Ge-
   * schwindigkeit drehen.
   * - x_stepCnt, y_stepCnt und z_stepCnt: Zählen die bereits durchgeführten Steps mit. Sobald die Anzahl an Steps
   * erreicht wird (also xyz_delays[<MotorIndex>]), wird kein weiterer Step durchgeführt.
   */
  float xyz_delays[3] = {1/(xyz_steps[0]/xyz_dis[0])/X_SPEED*1000000 , 1/(xyz_steps[1]/xyz_dis[1])/Y_SPEED*1000000 , 1/(xyz_steps[2]/xyz_dis[2])/Z_SPEED*1000000}; // durations for one step in µs
  unsigned int ind_MinDelay = indexOfMin(xyz_delays);
  float x_timeAdd = xyz_delays[ind_MinDelay]/xyz_delays[0];
  float y_timeAdd = xyz_delays[ind_MinDelay]/xyz_delays[1];
  float z_timeAdd = xyz_delays[ind_MinDelay]/xyz_delays[2];
  float x_timeCnt = 1;
  float y_timeCnt = 1;
  float z_timeCnt = 1;
  unsigned long int x_stepCnt = 0;
  unsigned long int y_stepCnt = 0;
  unsigned long int z_stepCnt = 0;
  /*
   * Hier passiert die eigentliche Bewegung:
   * Diese wird nur durchgefürt, solange der Druckprozess nicht gestoppt wurde und solange zumindest eine der Achsen
   * noch nicht die Anzahl an zu vollführenden Steps erreicht hat (durch xyz_steps gegeben).
   * Innerhalb der while Schleife wird zudem überprüft, ob die jeweilige Achse mit einem Step "dran" ist, also ob der
   * Zeit-Counter x_timeCnt, y_timeCnt bzw. z_timeCnt >= 1 ist. Dies gewährleistet, dass die jeweilige Achse auch wirklich
   * mit der vorgegeben Geschwindigkeit verfährt.
   * Anmerkung: Es geht bei der Geschwindigkeit um die Geschw. der Achsen, NICHT der Motoren. Sprich, beispielsweise
   * eins der Betten verfährt mit der Geschwindigkeit X_SPEED (oder Y_SPEED je nachdem), die Drehrate des Motors ist jedoch
   * durch die Step Size UND X_SPEED gegeben.
   */
  Serial.println("Moving steppers...");
  while(READY && (x_stepCnt<xyz_steps[0] || y_stepCnt<xyz_steps[1] || z_stepCnt<xyz_steps[2])){
    if(x_timeCnt>=1 && x_stepCnt<xyz_steps[0]){
      x_step();
      x_stepCnt++;
      X_POS += x_dir/X_STEP_SIZE;
      x_timeCnt -= 1;
    }
    if(y_timeCnt>=1 && y_stepCnt<xyz_steps[1]){
      y_step();
      y_stepCnt++;
      Y_POS += y_dir/Y_STEP_SIZE;
      y_timeCnt -= 1;
    }
    if(z_timeCnt>=1 && z_stepCnt<xyz_steps[2]){
      z_step();
      z_stepCnt++;
      Z_POS += z_dir/Z_STEP_SIZE;
      z_timeCnt -= 1;
    }
    delayMicroseconds(xyz_delays[ind_MinDelay]);
    x_timeCnt += x_timeAdd;
    y_timeCnt += y_timeAdd;
    z_timeCnt += z_timeAdd;
    if(isTriggered(X_MIN_PIN) || isTriggered(X_MAX_PIN) || isTriggered(Y_MIN_PIN) || isTriggered(Y_MAX_PIN) || isTriggered(Z_MIN_PIN) || isTriggered(Z_MAX_PIN)){
      Serial.println("Endstop triggered,  movement stopped");
      return;
    }
    /*
     * Siehe "functions":
     */
    check_interrupt(true);
  }
}

//***********************************************************************************************************

void x_step(){
  /* Führe einen Step am X-Motor aus
   */
  digitalWrite(X_STEP_PIN, HIGH);
  digitalWrite(X_STEP_PIN, LOW);
}

//***********************************************************************************************************

void y_step(){
  /* Führe einen Step am Y-Motor aus
   */
  digitalWrite(Y_STEP_PIN, HIGH);
  digitalWrite(Y_STEP_PIN, LOW);
}

//***********************************************************************************************************

void z_step(){
  /* Führe einen Step am Z-Motor aus
   */
  digitalWrite(Z_STEP_PIN, HIGH);
  digitalWrite(Z_STEP_PIN, LOW);
}

//***********************************************************************************************************

bool home_axis(char motor){
  /* Diese Funktion homed die Achse, die durch den char 'motor' übergeben wurde.
   *  In der switch-case Abfrage werden alle nötigen Pins für das Homing der entsprechenden Achse in Variablen
   *  hinterlegt.
   *  Das gesamte Homing besteht aus:
   *    - Fahre bis Endstop "MIN" gedrückt
   *    - Fahre eine gewisse Distanz zurück (gegeben durch globale Variable 'HOMING_REBUMP_DISTANCE')
   *    - Fahre erneut Richtung Endstop, jedoch mit verminderter Geschwindigkeit (gegeben durch globale Variablen
   *      'HOMING_SPEED' geteilt durch 'HOMING_SPEED_REBUMP_DIVISOR'
   *    - Wenn Endstop nach gewisser Strecke nicht erreicht wird -> Error und return false
   *    - Ansonsten -> Homing fertig
   *    - Setze entsprechende Motorposition ('X_POS', etc.) auf das gegebene Minimum ('X_MIN', etc.) und return
   *      true
   */
  if(motor=='x' || motor=='y' || motor=='z'){
    String msg1 = "Homing "+String(motor)+"-axis...";
    String msg2 = "Homing "+String(motor)+"-axis done";
    byte motor_enable_pin;
    byte motor_dir_pin;
    float motor_step_size;
    byte motor_ind;
    byte motor_min_pin;
    byte motor_max_pin;
    float motor_speed;
    /*
     * Definiere Variablen, welche weiter unten benutzt werden, dem Motor entsprechend:
     */
    switch(motor){
      case 'x':
        motor_enable_pin = X_ENABLE_PIN;
        motor_dir_pin = X_DIR_PIN;
        motor_step_size = X_STEP_SIZE;
        motor_ind = 0;
        motor_min_pin = X_MIN_PIN;
        motor_max_pin = X_MAX_PIN;
        break;
      case 'y':
        motor_enable_pin = Y_ENABLE_PIN;
        motor_dir_pin = Y_DIR_PIN;
        motor_step_size = Y_STEP_SIZE;
        motor_ind = 1;
        motor_min_pin = Y_MIN_PIN;
        motor_max_pin = Y_MAX_PIN;
        break;
      case 'z':
        motor_enable_pin = Z_ENABLE_PIN;
        motor_dir_pin = Z_DIR_PIN;
        motor_step_size = Z_STEP_SIZE;
        motor_ind = 2;
        motor_min_pin = Z_MIN_PIN;
        motor_max_pin = Z_MAX_PIN;
        break;
    }
    /* 
     * Starte das Homing...
     */
    Serial.println(msg1);
    digitalWrite(motor_enable_pin, LOW);  // aktiviere Motor
    digitalWrite(motor_dir_pin, LOW);     // stelle die Richtung des Motors ein: rückwärts
    float time_per_step = 1000000/motor_step_size/HOMING_SPEED[motor_ind];  // Wartezeit zwischen zwei Steps in µs
    while(!isTriggered(motor_min_pin) && !isTriggered(motor_max_pin) && READY){
      /*  
       * 1. Fahre bis Endstop gedrückt
       */
      switch(motor_ind){
        case 0:
          x_step();
          break;
        case 1:
          y_step();
          break;
        case 2:
          z_step();
          break;
      }
      delayMicroseconds(time_per_step);
      check_interrupt(true);
    }
    digitalWrite(motor_dir_pin,HIGH); // stelle die Richtung des Motors ein: vorwärts
    for(int i=0; i<motor_step_size*HOMING_REBUMP_DISTANCE[motor_ind] && READY;i++){
      /*  
       * 2. Nachdem Endstop gedrückt: Fahre um 'HOMING_REBUMP_DISTANCE' mm zurück
       */
      switch(motor_ind){
        case 0:
          x_step();
          break;
        case 1:
          y_step();
          break;
        case 2:
          z_step();
          break;
      }
      delayMicroseconds(time_per_step);
      check_interrupt(true);
    }
    digitalWrite(motor_dir_pin,LOW);  // stelle die Richtung des Motors ein: rückwärts
    for(int i=0; i<motor_step_size*1.05*HOMING_REBUMP_DISTANCE[motor_ind] && READY;i++){
      /*  
       * 3. Fahre erneut auf Endstop zu, diesmal aber langsamer
       */
      switch(motor_ind){
        case 0:
          x_step();
          break;
        case 1:
          y_step();
          break;
        case 2:
          z_step();
          break;
      }
      delayMicroseconds(time_per_step*HOMING_SPEED_REBUMP_DIVISOR[motor_ind]);
      if(isTriggered(motor_min_pin) || isTriggered(motor_max_pin)){
        break;
      }
      check_interrupt(true);
    }
    if(!isTriggered(motor_min_pin) && !isTriggered(motor_max_pin) && READY){
      error(16);
      return false;
    }else if(READY){
      digitalWrite(motor_dir_pin,HIGH); // stelle die Richtung des Motors ein: vorwärts
      while((isTriggered(motor_min_pin) || isTriggered(motor_max_pin)) && READY){
        /*  
         * 4. Wenn Endstop erneut gedrückt, fahre zurück bis Endstop nicht mehr gedrückt
         */
        switch(motor_ind){
          case 0:
            x_step();
            break;
          case 1:
            y_step();
            break;
          case 2:
            z_step();
            break;
        }
        delayMicroseconds(time_per_step*HOMING_SPEED_REBUMP_DIVISOR[motor_ind]);
        check_interrupt(true);
      }
      switch(motor_ind){
        /*  
         * 5. Setze die Position
         */
        case 0:
          X_POS=X_MIN;
          break;
        case 1:
          Y_POS=Y_MIN;
          break;
        case 2:
          Z_POS=Z_MIN;
          break;
      }
      Serial.println(msg2);
      return true;
    }
  }else{
    Serial.println("Can only home 'x', 'y' or 'z'-motor. Check Arduino code!");
  }
}
