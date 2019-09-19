

// Actual Onewheel Advertisement
// ow076106, Address: 38:81:d7:28:71:22, manufacturer data: 0103, serviceUUID: e659f300-ea98-11e3-ac10-0800200c9a66 
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("Connected to Onewheel!!!!!!!!!!!!!!!!!!!");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected from Onewheel");
    STATE = ST_SCAN;
 
  }
};

void registerOwCharacteristics() {
   delay(1000);
      Serial.println("Registering the rest of the characteristics");
      // Register Characteristics
      pRemoteCharacteristicBatteryLife = pRemoteService->getCharacteristic(batteryLifeUUID);
      pRemoteCharacteristicSpeedRPM = pRemoteService->getCharacteristic(speedUUID);
      pRemoteCharacteristicOdometer = pRemoteService->getCharacteristic(odometerUUID);
      pRemoteCharacteristicTrip= pRemoteService->getCharacteristic(tripUUID);
      pRemoteCharacteristicLastError= pRemoteService->getCharacteristic(lastErrorCodeUUID);
      pRemoteCharacteristicStatusError= pRemoteService->getCharacteristic(statusCodeUUID);
      pRemoteCharacteristicRidingMode= pRemoteService->getCharacteristic(ridingModeUUID);
      pRemoteCharacteristicCustomName= pRemoteService->getCharacteristic(customNameUUID);
      pRemoteCharacteristicSafetyHeadroom= pRemoteService->getCharacteristic(safetyHeadroomUUID);
      pRemoteCharacteristicBatteryCells= pRemoteService->getCharacteristic(batteryCellsUUID);
      pRemoteCharacteristicBatteryTemp= pRemoteService->getCharacteristic(batteryTempUUID);
      pRemoteCharacteristicBatteryVoltage= pRemoteService->getCharacteristic(batteryVoltageUUID);
      delay(2000);

      // Register Notifications for Data of Interest
      Serial.println("Enabling Notifications");
      pRemoteCharacteristicBatteryLife->registerForNotify(notifyCallback); delay(100);
      pRemoteCharacteristicSpeedRPM->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicOdometer->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicTrip->registerForNotify(notifyCallback);delay(100);
      //pRemoteCharacteristicLastError->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicStatusError->registerForNotify(notifyCallback);delay(100);
      
      //pRemoteCharacteristicCustomName->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicSafetyHeadroom->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicBatteryCells->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicBatteryTemp->registerForNotify(notifyCallback);delay(100);
      pRemoteCharacteristicBatteryVoltage->registerForNotify(notifyCallback);delay(100);

      Serial.println("Notifcations Setup!");
      delay(100);
}
