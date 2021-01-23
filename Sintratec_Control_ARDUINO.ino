#include "definitions.h"

/***************************************************************************************************************************************
*    Dieses Skript macht es dem Arduino möglich, ausgewählte G-Codes (siehe String "supported_M_codes[]" und "supported_G_codes[]"     *
*    per serieller Schnittstelle zu empfangen (für Baudrate siehe "definitions.h"), auszuführen und eine entsprechende Antwort per     *
*    serieller Schnittstelle zurückzugeben. Der Arduino akzeptiert nur dann neue Eingaben, wenn die vorherige Eingabe durchgeführt     *
*    wurde. Bei etwaigen fehlerhaften Eingaben gibt er eine entsprechende Fehlermeldung zurück (siehe "functions.ino"). Für den Um-    *
*    fang der behandelbaren Fehler siehe Funktion "error(nmr)" aus "functions.ino".                                                    *
*    Nach einer erfolgreichen Ausführung folgt die Ausgabe 'ok' mit den aktuellen Positionen und Step Sizes der Motoren sowie dem      *
*    Speed.                                                                                                                            *
***************************************************************************************************************************************/


//*************************************************************************************************************************************************
//      DEFINIERE GLOBALE VARIABLEN                                                                                                               *
//                                                                                                                                                *
String macro[NUMBER_OF_MACROS];//                                                                                                                 *
//                                                                                                                                                *
//supported "M" G-codes:                                                                                                                          *
String supported_M_codes[] = {"M17 (enable steppers)", "M18 (disable steppers)", "M92 X<stepsize> Y<stepsize> Z<stepsize> (set step sizes)",//    *
      "M114 (get motor information)", "M119 (check endstops)", "M810-M819 <str> (macros)"};//                                                     *
//                                                                                                                                                *
//supported "G" G-codes;                                                                                                                          *
String supported_G_codes[] = {"G0 X<pos> Y<pos> Z<pos> F<rate> (linear move)", "G28 <X> <Y> <Z> (homing)", "G90 (absolute positioning)",//        *
      "G91 (relative positioning)", "G92 X<x_pos> Y<y_pos> Z<z_pos> (set axis position)"};//                                                      *
//                                                                                                                                                *
/* Veränderbar per G-Code:                                                                                                                        *
 * X_POS, Y_POS, Z_POS, X_STEP_SIZE, Y_STEP_SIZE, Z_STEP_SIZE, SPEED, ABSOLUTE_POS, macro                                                         *
 *///                                                                                                                                             *
float X_POS = 0;//   [mm]                                                                                                                         *
float Y_POS = 0;//   [mm]                                                                                                                         *
float Z_POS = 0;//   [mm]                                                                                                                         *
float X_MIN = 0;//   [mm]                                                                                                                         *
float X_MAX = 130;// [mm]                                                                                                                         *
float Y_MIN = 0;//   [mm]                                                                                                                         *
float Y_MAX = 130;// [mm]                                                                                                                         *
float Z_MIN = 0;//   [mm]                                                                                                                         *
float Z_MAX = 280;// [mm]                                                                                                                         *
float X_MAX_MOVE = X_MAX-X_MIN; // [mm]                                                                                                           *
float Y_MAX_MOVE = Y_MAX-Y_MIN; // [mm]                                                                                                           *
float Z_MAX_MOVE = Z_MAX-Z_MIN; // [mm]                                                                                                           *
float SPEED = 100;              // [mm/s]                                                                                                         *
float HOMING_SPEED[3] = {100,100,50};          // [mm/s]                                                                                          *
byte HOMING_SPEED_REBUMP_DIVISOR[3] = {2,2,1}; // [] (Divisor für Geschw., mit der sich der Motor dem entspr. Endstop beim 2. Approach nähert)    *                                                                                     *
byte HOMING_REBUMP_DISTANCE[3] = {50,50,50};   // [mm]                                                                                            *
bool ABSOLUTE_POS = true;//                                                                                                                       *
double X_STEP_SIZE = 32; // step size for 1mm travel distance                                                                                     *
double Y_STEP_SIZE = 32; // step size for 1mm travel distance                                                                                     *
double Z_STEP_SIZE = 32; // step size for 1mm travel distance                                                                                     *
//                                                                                                                                                *
// "READY" beschreibt ob Druckprozess gestoppt wurde oder nicht. Falls ja wird keine Eingabe mehr angenommen, bis die Verbindung                  *
//  resetet wurde.                                                                                                                                *
bool READY = true;//                                                                                                                              *
//                                                                                                                                                *
// Die Variable "macro_ok" wird genutzt, um mit der Ausgabe 'ok' und der Übermittlung der Motoreigenschaften nach der Ausführung eines G-Codes    *
// zu warten, bis das komplette Makro fertig ausgeführt wurde (Fehlermeldungen werden weiterhin auch zwischen den G-Codes eines Makros            *
// ausgegeben).                                                                                                                                   *
bool macro_ok = true;//                                                                                                                           *
//*************************************************************************************************************************************************


//********************************************************************
//      SETUP DES ARDUINO                                            *
//                                                                   *
void setup(){//                                                      *
  pinMode(X_STEP_PIN , OUTPUT);//                                    *
  pinMode(X_DIR_PIN , OUTPUT);//                                     *
  pinMode(X_ENABLE_PIN , OUTPUT);//                                  *
  digitalWrite(X_MIN_PIN,HIGH);   // applying pull-up                *
  digitalWrite(X_MAX_PIN,HIGH);   // applying pull-up                *
  //                                                                 *
  pinMode(Y_STEP_PIN , OUTPUT);//                                    *
  pinMode(Y_DIR_PIN , OUTPUT);//                                     *
  pinMode(Y_ENABLE_PIN , OUTPUT);//                                  *
  digitalWrite(Y_MIN_PIN,HIGH);   // applying pull-up                *
  digitalWrite(Y_MAX_PIN,HIGH);   // applying pull-up                * 
  //                                                                 *
  pinMode(Z_STEP_PIN , OUTPUT);//                                    *
  pinMode(Z_DIR_PIN , OUTPUT);//                                     *
  pinMode(Z_ENABLE_PIN , OUTPUT);//                                  *
  digitalWrite(Z_MIN_PIN,HIGH);   // applying pull-up                *
  digitalWrite(Z_MAX_PIN,HIGH);   // applying pull-up                *
  //                                                                 *
  digitalWrite(X_ENABLE_PIN , HIGH);//                               *
  digitalWrite(Y_ENABLE_PIN , HIGH);//                               *
  digitalWrite(Z_ENABLE_PIN , HIGH);//                               *
  //                                                                 *
  /* Beginne serielle Kommunikation:                                 *
   *  (ACHTUNG: beim Gegenüber muss nach Aufbau der Verbidnung       *
   *  kurz gewartet werden)                                          *
   *///                                                              *
  Serial.begin(BAUDRATE);//                                          *
  Serial.flush();//                                                  *
}//                                                                  *
//********************************************************************


//*********************************************************************************************************************************
//      LOOP                                                                                                                           * 
//                                                                                                                                *
void loop() {//                                                                                                                   *
  /* Das einzige was er macht, ist auf Eingaben der seriellen Schnittstelle zu warten.                                            *
   */ //                                                                                                                          *
  if(Serial.available() > 0){//                                                                                                   *
    /* speichere Eingabe in "msg" -> dies löscht automatisch den Buffer der ser. Schnittstelle                                    *
     *///                                                                                                                         *
    String msg = Serial.readStringUntil('\n');//                                                                                  *
    if(READY == true){//                                                                                                          *
        /* Überprüfe insbesondere, ob der Druckprozess pausiert oder gar gestoppt                                                 *
         * wurde. Wurde er pausiert, ist lediglich die Abfrage der Motor-Stati mittels                                            * 
         * M114 möglich, bis die Pause aufgehoben wird.                                                                           *
         * .....                                                                                                                  *
         *///                                                                                                                     *
        if(msg.equals("stop")){//                                                                                                 *
            error(100); /* ist genau genommen kein Error, wurde aber aus strukturellen Gründen in error-Funktion verschoben*///   *
            return;//                                                                                                             *
        }else if(msg.equals("pause")){//                                                                                          *
            error(101); /* ist genau genommen kein Error, wurde aber aus strukturellen Gründen in error-Funktion verschoben*///   *
            msg = " ";//                                                                                                          *
            while(!msg.equals("pause")){//                                                                                        *
              delay(0.01);//                                                                                                      *
              msg = Serial.readStringUntil('\n');//                                                                               *
              if(msg.equals("M114") || msg.equals("m114")){//                                                                     *
                M114();//                                                                                                         *
              }//                                                                                                                 *
            }//                                                                                                                   *
            Serial.println("Continue process");//                                                                                 *
        }else{//                                                                                                                  *
            /* .... Andernfalls führt er die Eingabe aus (siehe "functions.ino"):                                                 *
             *///                                                                                                                 *
            run_input(msg);//                                                                                                     *
        }//                                                                                                                       *
    }else{//                                                                                                                      *
      Serial.println("Printing process was stopped. For a restart, restart the console.");//                                      *
    }//                                                                                                                           *
  }//                                                                                                                             *
}//                                                                                                                               *
//*********************************************************************************************************************************
