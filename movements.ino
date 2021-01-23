//========================================================================================
//==================================== Movements =========================================
//========================================================================================

void move_steppers(float xdis, float ydis, float zdis, bool xmove, bool ymove, bool zmove){
  /* Bewegt die Motoren um den jeweiligen float 'xdis, 'ydis' und 'zdis', sofern zugehöriger
   *  boolean 'xmove', etc. true ist.
   */
  /* Passe die Drehrichtungen entsprechend den Vorzeichen der Verfahrwege an:
   */
  float dir = 1;
  if(xdis<0){
    digitalWrite(X_DIR_PIN,LOW);
    dir = -1;
  }else{
    digitalWrite(X_DIR_PIN,HIGH);
  }
  if(ydis<0){
    digitalWrite(Y_DIR_PIN,LOW);
    dir = -1;
  }else{
    digitalWrite(Y_DIR_PIN,HIGH);
  }
  if(zdis<0){
    digitalWrite(Z_DIR_PIN,LOW);
    dir = -1;
  }else{
    digitalWrite(Z_DIR_PIN,HIGH);
  }
  /* Berechne die zu machenden Steps eines Motors, speichere sie in 'xyz_steps[3]' und merke
   *  dir den Motor (mittels des Indizes 'max_ind'), der am weitesten fahren muss. Das wird
   *  weiter unten wichtig für die Anzahl an Wiederholungen der for-Schleife. Diese wird benötigt
   *  um Motoren zu bewegen (google das).
   */
  float xyz_steps[3] = {0,0,0};
  float xyz_dis[3] = {abs(xdis),abs(ydis),abs(zdis)};
  byte max_ind = 0;
  if(xmove){
    digitalWrite(X_ENABLE_PIN,LOW);
    xyz_steps[0] = X_STEP_SIZE*xyz_dis[0];
  }
  if(ymove){
    digitalWrite(Y_ENABLE_PIN,LOW);
    xyz_steps[1] = Y_STEP_SIZE*xyz_dis[1];
    if(xyz_steps[1]>xyz_steps[max_ind]){
      max_ind = 1;
    }
  }
  if(zmove){
    digitalWrite(Z_ENABLE_PIN,LOW);
    xyz_steps[2] = Z_STEP_SIZE*xyz_dis[2];
    if(xyz_steps[2] > xyz_steps[max_ind]){
      max_ind = 2;
    }
  }
  if(xyz_steps[max_ind]==0){
    return;
  }else{
    /* Hier werden die entsprechenden Motoren bewegt. Dies passiert mittels for-loop, in der gleichzeitig auch
     * Abfragen gemacht werden, z.B. ob ein Stop-Signal gesendet wurde, ein Endstop berührt wurde oder wie sich
     * die Motorpositionen ändern.
     */
    Serial.println("Linear Move");
    /* Mit 'time_per_step' wird die Zeit zwischen zwei Steps berechnet, welche nötig ist, um den übergegenen
     *  maximalen (!) Verfahrweg mit der aktuellen Geschwindigkeit 'SPEED' (= globale Variable) zu fahren.
     *  Falls sich weitere Motoren mit geringerem Verfahrweg bewegen sollen, geschieht dies mit reduzierter
     *  Geschwindigkeit, sodass alle Motoren gleichzeitig stehen bleiben.
     */
    float time_per_step = 1/(xyz_steps[max_ind]/xyz_dis[max_ind])/SPEED * 1000000; // duration for one step in µs
    float x_factor = xyz_steps[0]/xyz_steps[max_ind];
    float x_do_step = 0;
    bool x_step_done = false;
    float y_factor = xyz_steps[1]/xyz_steps[max_ind];
    float y_do_step = 0;
    bool y_step_done = false;
    float z_factor = xyz_steps[2]/xyz_steps[max_ind];
    float z_do_step = 0;
    bool z_step_done = false;
    for(int i=1;i<=xyz_steps[max_ind] && READY;i++){
        x_do_step += x_factor;
        y_do_step += y_factor;
        z_do_step += z_factor;
        if(x_do_step>=1){
          x_step();
          x_do_step -= 1;
          x_step_done = true;
        }
        if(y_do_step>=1){
          y_step();
          y_do_step -= 1;
          y_step_done = true;
        }
        if(z_do_step>=1){
          z_step();
          z_do_step -= 1;
          z_step_done = true;
        }
        delayMicroseconds(time_per_step);
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //if(digitalRead(X_MIN_PIN)==HIGH || digitalRead(X_MAX_PIN)==HIGH || digitalRead(Y_MIN_PIN)==HIGH || digitalRead(Y_MAX_PIN)==HIGH || digitalRead(Z_MIN_PIN)==HIGH || digitalRead(Z_MAX_PIN)==HIGH){
        if(digitalRead(X_MIN_PIN)==LOW || digitalRead(X_MAX_PIN)==LOW || digitalRead(Y_MIN_PIN)==LOW || digitalRead(Y_MAX_PIN)==LOW || digitalRead(Z_MIN_PIN)==LOW || digitalRead(Z_MAX_PIN)==LOW){
          Serial.println("Endstop triggered,  movement stopped");
          return;
        }
        update_POS(x_step_done,y_step_done,z_step_done,1/X_STEP_SIZE*dir,1/Y_STEP_SIZE*dir,1/Z_STEP_SIZE*dir);
        x_step_done = false;
        y_step_done = false;
        z_step_done = false;
        check_interrupt();
      }
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
   *      'SPEED' geteilt durch 'HOMING_SPEED_REBUMP_DIVISOR'
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
    byte motor_step_size;
    byte motor_ind;
    byte motor_min_pin;
    byte motor_max_pin;
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
      case 'Z':
        motor_enable_pin = Z_ENABLE_PIN;
        motor_dir_pin = Z_DIR_PIN;
        motor_step_size = Z_STEP_SIZE;
        motor_ind = 2;
        motor_min_pin = Z_MIN_PIN;
        motor_max_pin = Z_MAX_PIN;
        break;
    }
    /* Starte das Homing...
     */
    Serial.println(msg1);
    digitalWrite(motor_enable_pin,LOW);
    digitalWrite(motor_dir_pin,LOW);
    float time_per_step = 1000000/motor_step_size/HOMING_SPEED[motor_ind]; // duration for one step in µs
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //while(digitalRead(motor_min_pin)==LOW && digitalRead(motor_max_pin)==LOW && READY){
    while(digitalRead(motor_min_pin)==HIGH && digitalRead(motor_max_pin)==HIGH && READY){
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
      check_interrupt();
    }
    digitalWrite(motor_dir_pin,HIGH);
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
      check_interrupt();
    }
    digitalWrite(motor_dir_pin,LOW);
    for(int i=0; i<motor_step_size*1.01*HOMING_REBUMP_DISTANCE[motor_ind] && READY;i++){
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
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //if(digitalRead(motor_min_pin)==HIGH || digitalRead(motor_max_pin)==HIGH){
      if(digitalRead(motor_min_pin)==LOW || digitalRead(motor_max_pin)==LOW){
        break;
      }
      check_interrupt();
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //if(digitalRead(motor_min_pin)==LOW && digitalRead(motor_max_pin)==LOW && READY){
    if(digitalRead(motor_min_pin)==HIGH && digitalRead(motor_max_pin)==HIGH && READY){
      error(16);
      return false;
    }else if(READY){
      digitalWrite(motor_dir_pin,HIGH);
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //while(digitalRead(motor_min_pin)==HIGH && digitalRead(motor_max_pin)==HIGH && READY){
      while(digitalRead(motor_min_pin)==LOW && digitalRead(motor_max_pin)==LOW && READY){
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
        check_interrupt();
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
