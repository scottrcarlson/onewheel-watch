#include "onewheel_watch.h"
#include "States.h"

/*
 * Deep Sleep Wakeup
 */
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

/*
 * Watchdog Timer Callback
 */
void IRAM_ATTR resetModule() {
  Serial.println("Watchdog RESET");
  esp_restart();
}

void wake_callback(){
  //placeholder for a callback
}

/***********************
 * BLE Advertisement Callback
 ***********************/
 class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        timerWrite(wdTimer, 0); //reset watchdog timer (feed the beast)
        //String adStr = advertisedDevice.toString().c_str();
        String advertDeviceName = advertisedDevice.getName().c_str(); //nameField.substring(nameValueIndex+1);

        // Scott ow076106
        // Eric  ow055999
        bool isOurDevice = advertDeviceName.startsWith("ow076106");
        Serial.print("Is our device?: ");
        Serial.println(isOurDevice ? "YES" : "NO");
        if (isOurDevice) {
            advertisedDevice.getScan()->stop();
            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            Serial.print("Found our device!  address: " + advertDeviceName); 
            advertDeviceName.toCharArray(device_name,9);
            //device_mac = advertisedDevice.getAddress().toString();
            STATE = ST_CONNECTING;
          }

        //u8g2.drawStr(0,row,advertisedDevice.getName().c_str());

        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
 
        row += 1;
        if (row > 5) {
          row = 0;
        }
    }
};


void setup()
{
  delay(100);
  boot_count += 1;
  Serial.begin(115200);
  u8g2.setI2CAddress(0x3C * 2); //DSTIKE Watch
  u8g2.begin();
  
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A4, INPUT);

  debouncer.attach(BUTTON_PIN,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer.interval(10); // Use a debounce interval of 25 milliseconds
  
  BLEDevice::init("");
  BLEDevice::setPower(ESP_PWR_LVL_P9);    // Max TX Power

  pBLEScan = BLEDevice::getScan();        //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);          //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);                // less or equal setInterval value

  //Recover from Deep Sleep
  print_wakeup_reason();
  //touchAttachInterrupt(T2, wake_callback, Threshold);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2,0);
  //esp_sleep_enable_touchpad_wakeup();

  //Configure watchdog timer
  wdTimer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(wdTimer, &resetModule, true);  //attach callback
  timerAlarmWrite(wdTimer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(wdTimer);                          //enable interrupt

  //Setup Idle Deep Sleep Timer (If not connected, after n seconds, sleep)
  idle_last_update = millis();

  //Setup Cell Voltage Statistics Clear Timer
  cells_last_update = millis();
  
  //Connect with first OW Found
  STATE = ST_SCAN;
}

/*
 * Handle Notification Callback
 * 
 * Process Incoming Notifications, Scale, Update Memory
 */
static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  //Serial.print("Notification: ");
  if (uartReadUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    Serial.println("Uart Read (Challenge Bytes)");
    if (length + challenge_byte_count > 20) {
      memset(unlock_challenge, 0, sizeof(unlock_challenge));
      challenge_byte_count = 0;
      Serial.println("Onewheel Uart Read Flood");
    }
    for (int i=0; i<length; i++) {
      unlock_challenge[challenge_byte_count] = pData[i]; 
      challenge_byte_count += 1;       
    } 
  }
  else if (batteryLifeUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Battery Remaining (Size %d bytes)\n",length);
    ow_batt_remaining = pData[1];
  }
  else if (batteryVoltageUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    Serial.printf("Battery Voltage (Size %d bytes)\n",length);
    ow_batt_voltage = pData[1] / 1.0;
  }
  else if (batteryCellsUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Battery Cell Voltage (Size %d bytes)\n",length);  // 2 bytes
    if (pData[0] >= 0 && pData[0] < 16) {
      ow_batt_cells[pData[0]] = pData[1] / 50.0f;
      if (ow_charge_stats_reset) {
        ow_charge_stats_reset = false;
        ow_batt_cell_min = 0;
        ow_batt_cell_max = 0;
        Serial.println("Stats Reset ****");
      }
      if (ow_batt_cells[pData[0]] > 1.0) {
        if (ow_batt_cells[pData[0]] > ow_batt_cell_max) {
            ow_batt_cell_max = ow_batt_cells[pData[0]];
        }
        if (ow_batt_cell_min == 0) {
           ow_batt_cell_min = ow_batt_cells[pData[0]];
        }
        else if (ow_batt_cells[pData[0]] < ow_batt_cell_min) {
          ow_batt_cell_min = ow_batt_cells[pData[0]];
        }
      }
      float sum = 0.0;
      for (int i = 0; i < 15; i++) {
        sum += ow_batt_cells[i];
      }
      ow_batt_cell_avg = sum / 15.0;
      //Serial.printf("Cell %d volts %.03f\n",pData[0], ow_batt_cells[pData[0]]);
    }
  }
  else if (batteryTempUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) { 
    //Serial.printf("Battery Temperature (Size %d bytes)\n",length);
    Serial.printf("Battery Temperature Byte0:%d / Byte1:%d",pData[0],pData[1]);
  }
  else if (odometerUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    Serial.printf("Life Odometer (Size %d bytes)\n",length);
  }
  else if (tripUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {
    //Serial.printf("Trip Odometer (Size %d bytes)\n",length);
   ow_odom_trip = ((pData[0] << 8) | pData[1]) * 11.0 * 3.14159265 / 63360.0;
  }
  else if (speedUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Speed RPM: (Size %d bytes)\n",length);
    //ow_speed_rpm = (pData[0] << 8) | pData[1];
    //ow_speed_rpm = pData[0] * 256 + pData[1];
    ow_speed_rpm = word(pData[0], pData[1]);
    ow_speed_mph = ow_speed_rpm * 11.0 * 3.14159265 * 60.0 / 63360.0;
  }
  else if (customNameUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    Serial.printf("Customer Name: (Size %d bytes)\n",length);
  }
  else if (firmwareUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    Serial.printf("Firmware Version: (Size %d bytes)\n",length);
  }
  else if (ridingModeUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Riding Mode: (Size %d bytes)\n",length);
  }
  else if (safetyHeadroomUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
   // Serial.printf("Safety Headroom: (Size %d bytes)\n",length);
    //Serial.printf("Safety HeadRoom Byte0:%d / Byte1:%d\n",pData[0],pData[1]);
    ow_safety_headroom = pData[1];
  }
  else if (lastErrorCodeUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Last Error Code: (Size %d bytes)\n",length);
    Serial.printf("Last Error Byte0:%d / Byte1:%d",pData[0],pData[1]);
  }
  else if (statusCodeUUID.toString() == pBLERemoteCharacteristic->getUUID().toString().c_str()) {  
    //Serial.printf("Status: (Size %d bytes)\n",length);
    //Serial.printf("Status BYTES: %d / %d\n", pData[0], pData[1]);
    ow_status_code = pData[0];
    unpackStatusCode();
  }
}

void loop()
{
  timerWrite(wdTimer, 0); //reset watchdog timer (feed the beast)
  //touchUpdate();
  buttonUpdate();
  if (buttonSuperLong()) {     //Deep Sleep on Super Long Touch
    STATE = ST_DEEP_SLEEP;
  }

  if (connected)  {
    idle_last_update = millis();
  }

  if (millis() - idle_last_update > idle_rate) {
        Serial.println("System Idle");
        Serial.println("Going into deep sleep");
        u8g2.clearDisplay();
        u8g2.setFont(u8g2_font_profont22_tr);
        u8g2.drawStr(10,25, "snoozin");
        delay(2000);
        esp_deep_sleep_start();
   }
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  
  if (connected && unlocked) {
    if (millis() - cells_last_update > 300000) {// Reset Cell Voltage Stats every 5 min
      cells_last_update = millis();
      ow_charge_stats_reset = true;
    }
    if (millis() - unlock_last_update > 10000) {
      ow_rssi = pClient->getRssi();
      Serial.printf("RSSI %d\n",ow_rssi);
      Serial.println("Sent firmware to keep unlocked.");
      unlock_last_update = millis();
      pRemoteCharacteristicFirmware->writeValue(firmware_out, 2,true);
      unlocked = true;

      Serial.printf("Cell Voltage Min/Avg/Max: %.2f / %.2f / %.2f\n",ow_batt_cell_min,ow_batt_cell_avg,ow_batt_cell_max);

      // Last Error Code
      uint16_t last_error = pRemoteCharacteristicLastError->readUInt16();
      delay(50);
      uint8_t last_error_val[2];
      last_error_val[0]=last_error & 0xff;
      last_error_val[1]=(last_error >> 8);
      Serial.printf("Last Error Code: %d\n", last_error_val[1]);
      ow_last_error_code =  last_error_val[1];

      //Check Battery Level
      float batteryValue = analogRead(A4);
      Serial.printf("OWwatch Battery Value: %f\n",batteryValue);
    }
  }
  if (!connected && unlocked) {
     if (millis() - unlock_last_update > 25000) {
      Serial.println("Onewheel Locked");
      unlocked = false;
     }
  }
  if (STATE_PREV != STATE) {
    STATE_PREV = STATE;
    state_enter = true;
    Serial.printf("State: %d\n", STATE);
  }
  
  if (STATE == ST_FACTORY) {
    stateFactory();
    
  }
  else if (STATE == ST_CHARGING) {
    ow_rssi = pClient->getRssi();
    stateCharging();
  }
  else if (STATE == ST_SCAN) {
    //digitalWrite(LED_BUILTIN, LOW);;
    stateScanning();
  }

  else if (STATE == ST_CONNECTING) {
    stateConnecting();
  }
  else if (STATE == ST_UNLOCK) {
    //digitalWrite(LED_BUILTIN, HIGH);
    stateUnlock();
  }
  else if (STATE == ST_DASH_1) {
    stateDash1();
  }
  else if (STATE == ST_DASH_2) {
    ow_rssi = pClient->getRssi();
    stateDash2(); 
  }
  else if (STATE == ST_DASH_3) {
    stateDash3();
  }
  else if (STATE == ST_DASH_4) {
    stateDash4();
  }
    else if (STATE == ST_DASH_5) {
    stateDash5();
  }
  else if (STATE == ST_DEEP_SLEEP) {
    if (state_enter) {
     u8g2.clearDisplay();
     u8g2.setFont(u8g2_font_profont22_tr);
     u8g2.drawStr(5,25, "deep sleep");

     u8g2.setFont(u8g2_font_profont10_tr);
     u8g2.setCursor(0,52);
     u8g2.printf("%s",VERSION);
     
     u8g2.sendBuffer();
    }
    if (buttonFell()) {
      Serial.println("Going into Deep Sleep");
      esp_deep_sleep_start();
    }
  }
    
  state_enter = false;
  //battery_value = analogRead(A4);
  battery_value = 0;
  delay(25);
}
