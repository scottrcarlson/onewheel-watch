
/**************************
 * Factory Default Screen
 **************************/
void stateFactory()
{
  if (state_enter) {
      u8g2.clearDisplay();
      u8g2.setCursor(0,0);
      u8g2.printf("ow wrist %s", VERSION);
      u8g2.drawStr(0,10, " un-associated");
      u8g2.drawStr(0,45, " press to scan"); 
  }
   
   if ( buttonRose() ) { // Rising Edge
     pressed = true;
     buttonPressTimeStamp = millis();
   }
   else if ( buttonFell() ) {
     if (pressed) {
        Serial.printf("Press\n");
     }
     //u8g2.clearLine(7);
     pressed = false;
   }

  if (not pressed) buttonPressTimeStamp = millis();

  int t_delta = millis()-buttonPressTimeStamp;

  bling_press = t_delta / 62.5;
  float scale = 1280.0 / 100.0;
  int bar_width = bling_press * scale;
  u8g2.setDrawColor(0);// Black
  u8g2.drawBox(0,59,128,5);
  u8g2.setDrawColor(1);// Black
  u8g2.drawBox(0,59,bar_width,5);
  u8g2.sendBuffer();

    if (buttonLong()) {
    Serial.printf("Long Press\n");
    pressed = false;
    STATE = ST_SCAN;
   }
}
/**************************/


/**************************
 * OW CHARGING
 **************************/
void stateCharging() {
  if (state_enter) {
      Serial.println("Charging State");
      u8g2.clearDisplay();
    }    
    if (millis() - dash_last_update > dash_update_rate) {
      dash_last_update = millis();
      float scale = 128.0 / 100.0;

      u8g2.setDrawColor(0);// Black
      u8g2.drawBox(0,0,128,25);
      
      u8g2.setFont(u8g2_font_profont22_mf);
      u8g2.setDrawColor(2);// Xor
      u8g2.setCursor(17, 2);
      if (ow_batt_remaining == 100 & ow_batt_cell_avg == ow_batt_cell_min && ow_batt_cell_avg == ow_batt_cell_max) {
        u8g2.print("COMPLETE");
      }
      else if (ow_batt_remaining == 100) {
        u8g2.setCursor(14, 2);
        u8g2.print("BALANCING");
      }
      else {
        u8g2.print("CHARGING");
      }
      
      //u8g2.setDrawColor(1);// White
      
      u8g2.setDrawColor(2);// Xor
      u8g2.drawBox(0,0,ow_batt_remaining*scale,25);
      
      u8g2.setDrawColor(1);// White
      u8g2.setFont(u8g2_font_profont15_mf);
      u8g2.setCursor(47, 26);
      u8g2.printf("%d",ow_batt_remaining);
      u8g2.setCursor(72, 26);
      u8g2.print("%");

       u8g2.setFont(u8g2_font_profont12_mf);
      u8g2.setCursor(0, 49);
      //u8g2.printf("Avg:%0.2fv  ",ow_batt_cell_avg); 
      u8g2.printf("Rssi:%d  ",ow_rssi); 
      u8g2.setCursor(75, 44);
      u8g2.printf("Min:%0.2fv",ow_batt_cell_min); 
      u8g2.setCursor(75, 54);
      u8g2.printf("Max:%0.2fv",ow_batt_cell_max); 


      u8g2.sendBuffer();
     }


     if (!ow_charging) {
        Serial.println("Done Charging");
        STATE = ST_DASH_1;
     }
}
/**************************/



/**************************
 * BLE OW Scanning
 **************************/
void stateScanning()
{
  if (state_enter) {
    u8g2.clearDisplay();
    u8g2.setCursor(0,0);
    u8g2.printf("ow wrist %s", VERSION);
    u8g2.drawStr(0, 10, "locating board");
    u8g2.setCursor(0,20);
    u8g2.printf("boot count %d", boot_count);
    u8g2.sendBuffer();
  }
  row = 0;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  String countMsg = String(foundDevices.getCount());
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(1000);  
}
/**************************/



/**************************
 * Connect
 **************************/
void stateConnecting() {
  if (state_enter) {
    //u8g2.clearDisplay();
    retry_count = 0;
    u8g2.setDrawColor(0);// Black
    u8g2.drawBox(0,10,128,12); 
    u8g2.setDrawColor(1);// Black
    delay(250);
  }

    u8g2.setCursor(0,0);
    u8g2.printf("ow wrist %s", VERSION);
  u8g2.drawStr(0, 10, "board found");
  u8g2.sendBuffer();

  pClient  = BLEDevice::createClient();
  Serial.println("Connect Step 1");
  pClient->connect(*pServerAddress);
  Serial.println("Connect Step 2");
  pClient->setClientCallbacks(new MyClientCallback());
  connected = true;
  Serial.println("Connect Step 3");   
  /*int time_start = millis();
  while (!connected) {
    if (millis() - time_start > 10000) {
      Serial.println("Timeout, Retry after 5 sec.");
      delay(5000);
      retry_count += 1;
      break;
    }
  }*/

  if (connected) {
    delay(200);
    pRemoteService = pClient->getService(serviceUUID);
    Serial.println("Found Service");
    delay(200);
    pRemoteCharacteristicFirmware = pRemoteService->getCharacteristic(firmwareUUID);
    Serial.println("Registered Firmware Chara");
    if (unlocked) {
      registerOwCharacteristics();
      if (ow_charging) {
        STATE = ST_CHARGING;
      }
      else
      {
        STATE = ST_DASH_1;
      }
      
    }
    else STATE = ST_UNLOCK;
  }
  else {
    if (retry_count > 2) {
      STATE = ST_SCAN;
  }
 }
}
 /**************************/

 

 /**************************
 * BLE Unlock Onewheel
 **************************/
void stateUnlock() {
  if (state_enter) {
    u8g2.setDrawColor(0);// Black
    u8g2.drawBox(0,10,128,12); 
    u8g2.setDrawColor(1);// White
    pRemoteCharacteristicUart = pRemoteService->getCharacteristic(uartReadUUID);
    pRemoteCharacteristicUartWrite = pRemoteService->getCharacteristic(uartWriteUUID);
  }
  uint16_t firmware = pRemoteCharacteristicFirmware->readUInt16();
  Serial.printf("OW Firmware Version: %#08x\n", firmware);  
  
  u8g2.setCursor(0,0);
  u8g2.printf("ow wrist %s", VERSION);
  u8g2.setCursor(0, 10);
  u8g2.printf("board: %s", device_name); 
  u8g2.sendBuffer();

  bool gemini = false;
  u8g2.setCursor(0, 20);
  if (firmware == 0x2610) {
    gemini = true;
    u8g2.printf("OW+ XR FW: %02x", firmware);
  }
  else if  (firmware == 0x0fc2) u8g2.printf("OW+ FW: %02x", firmware);
  
  u8g2.sendBuffer();

  if (!gemini) {
    STATE = ST_DASH_1;
  }
  else {
    firmware_out[0]=firmware & 0xff;
    firmware_out[1]=(firmware >> 8);
    pRemoteCharacteristicUart->registerForNotify(notifyCallback);
    challenge_byte_count = 0;
    u8g2.drawStr(0, 30, "Initiate Unlock");
    u8g2.sendBuffer();
    pRemoteCharacteristicFirmware->writeValue(firmware_out, 2,true);
    int unlock_wait = millis();
    timerWrite(wdTimer, 0); //reset watchdog timer (feed the beast)
    while (millis() - unlock_wait < 3000 && challenge_byte_count < 20) {}
    timerWrite(wdTimer, 0); //reset watchdog timer (feed the beast)
    if (challenge_byte_count == 20) {
      u8g2.drawStr(0, 40, "Challenge Recvd");
      u8g2.sendBuffer();
      Serial.println("Challenge received.");
      for (int i=0; i< 20; i++) {
        Serial.printf("%02X", unlock_challenge[i]);
      }
      Serial.println("");
      for (int i=0; i< 3; i++) {
         unlock_resp[i] = unlock_challenge[i];
      }
       for (int i=3; i< 19; i++) {
         unlock_prehash[i-3] = unlock_challenge[i];
      }
       for(int i=0; i< 16; i++) {
        unlock_prehash[i+16] = unlock_password[i];
      }
      char* prehash_chars = (char*) unlock_prehash;
      prehash_chars[32] = '\0';
      unsigned char* hash=MD5::make_hash(prehash_chars);
      for (int i=0; i< 16; i++) {
        unlock_resp[i+3] = hash[i];
      }
      free(hash);
      byte checkByte = 0;
      int j = 19;
      int i = 0;
      while (i < j)
      {
          checkByte = ((byte)(unlock_resp[i] ^ checkByte));
          i += 1;
      }
      unlock_resp[19] = checkByte;
      pRemoteCharacteristicUart->registerForNotify(nullptr);
      pRemoteCharacteristicUartWrite->writeValue(unlock_resp, 20);
      u8g2.drawStr(0, 50, "Unlocking");
      u8g2.sendBuffer();
      unlocked = true;
      registerOwCharacteristics();
      
      uint16_t riding_mode = pRemoteCharacteristicRidingMode->readUInt16();
      delay(50);
      uint8_t ridingmode_val[2];
      ridingmode_val[0]=riding_mode & 0xff;
      ridingmode_val[1]=(riding_mode >> 8);
      Serial.printf("Riding Mode: %d\n", ridingmode_val[1]);
      ow_riding_mode = ridingmode_val[1];
    
      //Battery Remaining
      uint16_t batt = pRemoteCharacteristicBatteryLife->readUInt16();
      delay(50);
      uint8_t batt_val[2];
      batt_val[0]=batt & 0xff;
      batt_val[1]=(batt >> 8);
      Serial.printf("Battery Remaining: %d\n", batt_val[1]);
      ow_batt_remaining = batt_val[1];
    
      // Battery Voltage
      uint16_t battvolt = pRemoteCharacteristicBatteryVoltage->readUInt16();
      delay(50);
      uint8_t battvolt_val[2];
      battvolt_val[0]=battvolt & 0xff;
      battvolt_val[1]=(battvolt >> 8);
      //Serial.printf("BattVolt BYTE1 %d\n", battvolt_val[0]);
      //Serial.printf("BattVolt BYTE2 %d\n", battvolt_val[1]);
    
      ow_batt_voltage = battvolt_val[1] / 1.0;
      Serial.printf("Battery Voltage: %.2f\n", ow_batt_voltage);
    
      // Life Odometer
      uint16_t odometer = pRemoteCharacteristicOdometer->readUInt16();
      delay(50);
      uint8_t odometer_val[2];
      odometer_val[0]=odometer & 0xff;
      odometer_val[1]=(odometer >> 8);
      Serial.printf("Odometer: %d\n", odometer_val[1]);
      ow_odom_life =  odometer_val[1];
    
    
      // Status Code
      uint16_t status_error = pRemoteCharacteristicStatusError->readUInt16();
      delay(50);
      uint8_t status_error_val[2];
      status_error_val[0]=status_error & 0xff;
      status_error_val[1]=(status_error >> 8);
      Serial.printf("Status Code: %d / %d | %d\n", status_error_val[0], status_error_val[1], status_error);
      ow_status_code =  status_error_val[0];
  
      unpackStatusCode();
        
    
      // Last Error Code
      uint16_t last_error = pRemoteCharacteristicLastError->readUInt16();
      delay(50);
      uint8_t last_error_val[2];
      last_error_val[0]=last_error & 0xff;
      last_error_val[1]=(last_error >> 8);
      Serial.printf("Last Error Code: %d\n", last_error_val[1]);
      ow_last_error_code =  last_error_val[1];
  
      if (ow_status_code == 0 and ow_odom_life == 0 and ow_batt_voltage == 0) {
        //Bullshit Connection, Retry
        Serial.println("Bullshit connection, going to scan again");
        connected = false;
        STATE = ST_SCAN;
      }
      else {
        if (ow_charging) {
          STATE = ST_CHARGING;
        }
        else {
          STATE = ST_DASH_1;
        }
         
      }
    }
  }
 }
/**************************/



/**************************
 * DASH 1
 **************************/
void stateDash1() {
  if (state_enter) {
      u8g2.clearDisplay();
    }    
    if (millis() - dash_last_update > dash_update_rate) {
      dash_last_update = millis();
      float scale = 128.0 / 100.0;

      u8g2.setDrawColor(0);// Black
      u8g2.drawBox(0,2,128,3);
      u8g2.setDrawColor(1);// White
      u8g2.drawBox(0,2,ow_batt_remaining*scale,3);
      
      u8g2.setDrawColor(1);// Xor
      u8g2.setCursor(5, 5);
      u8g2.printf("LIFE"); 
      u8g2.setCursor(90, 5);
      u8g2.printf("%d %%",ow_batt_remaining); 
 
      u8g2.setDrawColor(0);// Black
      u8g2.drawBox(0,58,128,3);
      u8g2.setDrawColor(1);// White
      u8g2.drawBox(0,58,ow_safety_headroom*scale,3);
      
      u8g2.setDrawColor(1);// Xor
      u8g2.setCursor(5, 47);
      u8g2.printf("HEAD ROOM");   
      u8g2.setCursor(90, 47);    
      u8g2.printf("%d %% ",ow_safety_headroom);   

      u8g2.setDrawColor(1);// White
      u8g2.setFont(u8g2_font_profont29_mf);
      if (SUB_STATE == 0) {
        u8g2.setCursor(20, 18);
        u8g2.printf("%.0f ",ow_speed_mph); 
        u8g2.setCursor(65, 18);
        u8g2.print("mph");
      }
      else {
        u8g2.setCursor(30, 18);
        u8g2.printf("%2.1f",ow_odom_trip);
        u8g2.setCursor(86, 18);
        u8g2.print("mi");
      }

      if (ow_detected_rider) {
        u8g2.setDrawColor(1);
      }
      else {
        u8g2.setDrawColor(0);
      }
      u8g2.drawBox(0,22,10,20);

      
      if (ow_detected_pad1) {
        u8g2.setDrawColor(1);
      }
      else {
        u8g2.setDrawColor(0);
      }
      if (config_ow_goofy) {
        u8g2.drawBox(118,35,10,10);
      }
      else {
        u8g2.drawBox(118,20,10,10);  
      }
      

      
      if (ow_detected_pad2) {
        u8g2.setDrawColor(1);
      }
      else {
        u8g2.setDrawColor(0);
      }
      if (config_ow_goofy) {
         u8g2.drawBox(118,20,10,10);  
      }
      else {
        u8g2.drawBox(118,35,10,10);
      }
      
      
      
      u8g2.sendBuffer();
     }

     buttonUpdate();
     if (buttonLong() ) {
        STATE = ST_DASH_2;
     }
     if (buttonRose() ) {
        if (SUB_STATE == 0) SUB_STATE = 1;
        else SUB_STATE = 0;
        u8g2.setDrawColor(0);// Black
        u8g2.drawBox(0,18,128,30); 
        u8g2.setDrawColor(1);// White
     }


}
/**************************/




/**************************
 * DASH 2
 **************************/
void stateDash5() {
  if (state_enter) {
    u8g2.clearDisplay();
    
  }    
  u8g2.setFont(u8g2_font_profont11_tr);
  if (millis() - dash_last_update > dash_update_rate) {
    dash_last_update = millis();
    u8g2.setCursor(0, 0);
    u8g2.printf(" %d mph | %d rpm  ",ow_speed_mph,ow_speed_rpm);  
    u8g2.setCursor(0, 8);
    u8g2.printf("Odom %0.2f | %d  ",ow_odom_trip,ow_odom_life);  
    u8g2.setCursor(0, 16);
    u8g2.printf("Life %d%%",ow_batt_remaining); 
    u8g2.setCursor(0, 24);
    u8g2.printf("Riding Mode: %d  ",ow_riding_mode); 
    u8g2.setCursor(0, 32);
    u8g2.printf("Headroom: %d  ",ow_safety_headroom);       
    u8g2.setCursor(0, 40);
    u8g2.printf("Status: %d  ",ow_status_code); 
    u8g2.setCursor(0, 48);
    u8g2.printf("Last Error: %d  ",ow_last_error_code); 
    u8g2.setCursor(0, 56);
    u8g2.printf("RSSI: %3d",ow_rssi); 
    u8g2.sendBuffer();
    //touchUpdate();
   }
   if (buttonLong() ) {
       STATE = ST_DASH_1;
   }
   if ( buttonRose() ) STATE = ST_DASH_2;

}
/**************************/



/**************************
 * DASH 3
 **************************/
void stateDash4() {
  if (state_enter) {
      u8g2.clearDisplay(); 
  }
  if (millis() - dash_last_update_slow > dash_update_rate_slow || state_enter) {
    dash_last_update_slow = millis(); 
    u8g2.setFont(u8g2_font_profont10_tr);
  
    u8g2.setCursor(0, 0);
    u8g2.printf("Life:%d%%  ",ow_batt_remaining); 
    u8g2.setCursor(0, 8);
    u8g2.printf("VBatt:%0.2f  ",ow_batt_voltage);  
    u8g2.setCursor(0, 16);
    u8g2.printf("VCell Min:%0.2f  ",ow_batt_cell_min); 
    u8g2.setCursor(0, 24);
    u8g2.printf("VCell Avg:%0.2f  ",ow_batt_cell_avg); 
    u8g2.setCursor(0, 32);
    u8g2.printf("VCell Max:%0.2f  ",ow_batt_cell_max); 
    u8g2.setCursor(0, 40);
    u8g2.printf("OW Watch Bat:%d  ",battery_value); 
    u8g2.sendBuffer();
  }

  if (buttonLong() ) {
    STATE = ST_DASH_1;
  }
  if ( buttonRose() ) STATE = ST_DASH_5;

}



/**************************
 * DASH 4
 **************************/
void stateDash3() {
  if (state_enter) {
      u8g2.clearDisplay(); 
  }
  if (millis() - dash_last_update_slow > dash_update_rate_slow || state_enter) {
    dash_last_update_slow = millis(); 

    u8g2.setCursor(0, 0);
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.printf("%0.2fv max ",ow_batt_cell_max); 
    u8g2.setCursor(0, 20);
    u8g2.setFont(u8g2_font_profont29_tr);
    u8g2.printf("%0.2fv",ow_batt_cell_avg); 
    u8g2.setCursor(0, 50);
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.printf("%0.2fv min ",ow_batt_cell_min); 
    u8g2.sendBuffer();
  }

  if (buttonLong() ) {
    STATE = ST_DASH_1;
  }
  if ( buttonRose() ) STATE = ST_DASH_4;

}


/**************************
 * DASH 5
 **************************/
void stateDash2() {
  if (state_enter) {
      u8g2.clearDisplay(); 
  }
  if (millis() - dash_last_update_slow > dash_update_rate_slow || state_enter) {
    dash_last_update_slow = millis(); 

    u8g2.setCursor(0, 0);
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.printf("Odometer"); 
    u8g2.setCursor(0, 20);
    u8g2.setFont(u8g2_font_profont29_tr);
    u8g2.printf("%0.2fmi",ow_odom_trip); 
    u8g2.setCursor(0, 50);
    u8g2.setFont(u8g2_font_profont17_tr);
    u8g2.printf("%d miles",ow_odom_life); 
    u8g2.sendBuffer();
  }

  if (buttonLong() ) {
    STATE = ST_DASH_1;
  }
  if ( buttonRose() ) STATE = ST_DASH_3;

}
/* Helper Function
 *  Breakout Bits from Status Flag
 */
void unpackStatusCode() {
  if (bitRead(ow_status_code,0)) {
      //Serial.println("Rider Detected");
      ow_detected_rider = true;
    }
    
    else ow_detected_rider = false;
    if (bitRead(ow_status_code,1)) {
      //Serial.println("Pad 1 Detected");
      ow_detected_pad1 = true;
    }
    else ow_detected_pad1 = false;
   
    if (bitRead(ow_status_code,2)) {
      //Serial.println("Pad 2 Detected");
      ow_detected_pad2 = true;
    }
    else ow_detected_pad2 = false;

    if (bitRead(ow_status_code,5)) {
      Serial.println("Charging");
      ow_charging = true;
      STATE = ST_CHARGING;
    }
    else ow_charging = false;
}
/**************************/
