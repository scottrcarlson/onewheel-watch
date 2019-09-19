// Integrate code from button_debounce_example
// When not registered to device
// scan ble and list mac addresses with arrow point to one in a list
// press progress arrow to the next mac address
// long press will attempt to connect and pair with one wheel
// ?? use mac address and name?  just the name? just the addr?

// Once paired with one wheel
// single press cycles through dashboards
// super long touch (5 seconds) will unregister/unpair/disconnect unassociate with onewheel

//On Power up, if previously registered with device, will attempt to connect to the device
// until either a successful pairing or an error message with ability to press to try
// again? or automatically
// super long press will unregister device

// slide switch for power, single momentary switch for actions

// the OLED used

/*
 * Current Draw Measurements
 *  140ma @ 5.14v to ESP 3.3v (During BLE Scanning)
 *  85ma avg. normal operation
 *  8.6ma deep sleep
 */

/* "Extneral Antenna on Heltek Wifi Kit, Soldered 1.18 inch stranded 26awg wire to the end of pcb antenna
 *  10-15dbm RSSI improvement, make device usable when strapped to wrist. Would like to see 10 more if possible?
 */

/* Discussion regarding BLE interaction with Gemini firmware
 *  https://github.com/ponewheel/android-ponewheel/issues/86
 */

 /* Android BLE Logging
  *  http://www.fte.com/WebHelp/BPA600/Content/Documentation/WhitePapers/BPA600/Encryption/GettingAndroidLinkKey/RetrievingHCIlog.htm
  */
/* BLE Gatt Characteristics
 *  OnewheelServiceUUID = "e659f300-ea98-11e3-ac10-0800200c9a66"
    OnewheelConfigUUID = "00002902-0000-1000-8000-00805f9b34fb"
    SerialNumber = "e659F301-ea98-11e3-ac10-0800200c9a66"  # 2085
    RidingMode = "e659f302-ea98-11e3-ac10-0800200c9a66"
    BatteryRemaining = "e659f303-ea98-11e3-ac10-0800200c9a66"
    BatteryLow5 = "e659f304-ea98-11e3-ac10-0800200c9a66"
    BatteryLow20 = "e659f305-ea98-11e3-ac10-0800200c9a66"
    BatterySerial = "e659f306-ea98-11e3-ac10-0800200c9a66"  # 22136
    TiltAnglePitch = "e659f307-ea98-11e3-ac10-0800200c9a66"
    TiltAngleRoll = "e659f308-ea98-11e3-ac10-0800200c9a66"
    TiltAngleYaw = "e659f309-ea98-11e3-ac10-0800200c9a66"
    Temperature = "e659f310-ea98-11e3-ac10-0800200c9a66"
    StatusError = "e659f30f-ea98-11e3-ac10-0800200c9a66"
    BatteryCells = "e659f31b-ea98-11e3-ac10-0800200c9a66"
    BatteryTemp = "e659f315-ea98-11e3-ac10-0800200c9a66"
    BatteryVoltage = "e659f316-ea98-11e3-ac10-0800200c9a66"
    CurrentAmps = "e659f312-ea98-11e3-ac10-0800200c9a66"
    CustomName = "e659f3fd-ea98-11e3-ac10-0800200c9a66"
    FirmwareRevision = "e659f311-ea98-11e3-ac10-0800200c9a66"  # 3034
    HardwareRevision = "e659f318-ea98-11e3-ac10-0800200c9a66"  # 2206
    LastErrorCode = "e659f31c-ea98-11e3-ac10-0800200c9a66"
    LifetimeAmpHours = "e659f31a-ea98-11e3-ac10-0800200c9a66"
    LifetimeOdometer = "e659f319-ea98-11e3-ac10-0800200c9a66"
    LightingMode = "e659f30c-ea98-11e3-ac10-0800200c9a66"
    LightsBack = "e659f30e-ea98-11e3-ac10-0800200c9a66"
    LightsFront = "e659f30d-ea98-11e3-ac10-0800200c9a66"
    Odometer = "e659f30a-ea98-11e3-ac10-0800200c9a66"
    SafetyHeadroom = "e659f317-ea98-11e3-ac10-0800200c9a66"
    SpeedRpm = "e659f30b-ea98-11e3-ac10-0800200c9a66"
    TripRegenAmpHours = "e659f314-ea98-11e3-ac10-0800200c9a66"
    TripTotalAmpHours = "e659f313-ea98-11e3-ac10-0800200c9a66"
    UartSerialRead = "e659f3fe-ea98-11e3-ac10-0800200c9a66"
    UartSerialWrite = "e659f3ff-ea98-11e3-ac10-0800200c9a66"
    e659f31d-ea98-11e3-ac10-0800200c9a66 labeled Data29
    e659f31e-ea98-11e3-ac10-0800200c9a66 labeled Data30
    e659f31f-ea98-11e3-ac10-0800200c9a66 labeled Data31
    e659f320-ea98-11e3-ac10-0800200c9a66 labeled Data32
    
 */


 /*
  * Error Codes from ponewheel
  *     ERROR_CODE_MAP.append(1, "ErrorBMSLowBattery");
        ERROR_CODE_MAP.append(2, "ErrorVoltageLow");
        ERROR_CODE_MAP.append(3, "ErrorVoltageHigh");
        ERROR_CODE_MAP.append(4, "ErrorFallDetected");
        ERROR_CODE_MAP.append(5, "ErrorPickupDetected");
        ERROR_CODE_MAP.append(6, "ErrorOverCurrentDetected");
        ERROR_CODE_MAP.append(7, "ErrorOverTemperature");
        ERROR_CODE_MAP.append(8, "ErrorBadGyro");
        ERROR_CODE_MAP.append(9, "ErrorBadAccelerometer");
        ERROR_CODE_MAP.append(10, "ErrorBadCurrentSensor");
        ERROR_CODE_MAP.append(11, "ErrorBadHallSensors");
        ERROR_CODE_MAP.append(12, "ErrorBadMotor");
        ERROR_CODE_MAP.append(13, "ErrorOvercurrent13");
        ERROR_CODE_MAP.append(14, "ErrorOvercurrent14");
        ERROR_CODE_MAP.append(15, "ErrorRiderDetectZone");

        BitSet(8);
        bitSet.set(0,riderDetected);
        bitSet.set(1,riderDetectPad1);
        bitSet.set(2,riderDetectPad2);
        bitSet.set(3,icsuFault);
        bitSet.set(4,icsvFault);
        bitSet.set(5,charging);
        bitSet.set(6,bmsCtrlComms);
        bitSet.set(7,brokenCapacitor);
  */
