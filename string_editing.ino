//========================================================================================
//================================== String editing ======================================
//========================================================================================

String remove_spaces(String msg){
  /*
   * entfernt alle Leerzeichen in einem String und gibt den resultierenden
   * String zurück
   */
  int l = msg.length();
  byte ind = 0;
  while(ind < l){ // delete all spaces
    if(isSpace(msg[ind])){
      msg.remove(ind,1);
      l--;
    }else{
      ind++;
    }
  }
  return msg;
}

//***********************************************************************************************************

bool xyz_ident(String msg, float xyz[3]){
  /*
   * Sucht nach den Variablen X, Y und Z in einem String (die Reihenfolge ist dabei egal)
   * und den jeweils darauf folgenden Werten. Beispiel: X213.21 Y543 (es müssen nicht
   * alle 3 Variablen vertreten sein). Die Werte werden in dem übergebenen float array "xyzf[3]"
   * gespeichert: Wert für X an 0. Stelle, für Y an 1. Stelle und für Z an 2. Stelle
   * Bei Erfolg wird true zurückgegeben, bei Misserfolg (zB wegen fehlerhaftem String) false
   */
  byte ind = 0;
  int l = msg.length();
  int x=xyz[0];
  int y=xyz[1];
  int z=xyz[2];
  while(ind<l){
    switch(msg[ind]){
      case 'x':
      case 'X':
        ind++;
        x = ind;
        while(msg[ind] != 'y' && msg[ind] != 'Y' && msg[ind] != 'z' && msg[ind] != 'Z' && ind<l){
          if(!isDigit(msg[ind]) && msg[ind] != '.' && msg[ind] != '-'){
            error(4);
            return false;
          }
          ind++;
        }
        if(x!=ind){
          xyz[0] = msg.substring(x,ind).toFloat();
        }else{
          error(4);
          return false;
        }
        break;
      case 'y':
      case 'Y':
        ind++;
        y = ind;
        while(msg[ind] != 'x' && msg[ind] != 'X' && msg[ind] != 'z' && msg[ind] != 'Z' && ind<l){
          if(!isDigit(msg[ind]) && msg[ind] != '.' && msg[ind] != '-'){
            error(5);
            return false;
          }
          ind++;
        }
        if(y!=ind){
          xyz[1] = msg.substring(y,ind).toFloat();
        }else{
          error(5);
          return false;
        }
        break;
      case 'z':
      case 'Z':
        ind++;
        z = ind;
        while(msg[ind] != 'x' && msg[ind] != 'X' && msg[ind] != 'y' && msg[ind] != 'Y' && ind<l){
          if(!isDigit(msg[ind]) && msg[ind] != '.' && msg[ind] != '-'){
            error(6);
            return false;
          }
          ind++;
        }
        if(z!=ind){
          xyz[2] = msg.substring(z,ind).toFloat();
        }else{
          error(6);
          return false;
        }
        break;
      default:
        ind++;
        break;
    }
  }
  return true;
}
