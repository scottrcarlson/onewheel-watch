#ifndef _OWWHEADER_H    // Put these two lines at the top of your file.
#define _OWWHEADER_H    // (Use a suitable name, usually based on the file name.)

#include <U8g2lib.h>
//#include <Bounce2.h>
#include <MD5.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BUTTON_PIN 22
//#define BUTTON_PIN 35

#include <Bounce2.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


char VERSION[] = "v0.3.0";

char device_name[] = "";
char device_mac[] = "";

/* Deep Sleep Wakeup Touch Sensitivity     */
/* Greater the value, more the sensitivity */
#define Threshold 15 

BLEScan* pBLEScan;
int scanTime = 7; //BLE Scantime In seconds
int row = 0; // OW BLE Name List Row Index

static BLEUUID serviceUUID("e659f300-ea98-11e3-ac10-0800200c9a66");
static BLEUUID firmwareUUID("e659f311-ea98-11e3-ac10-0800200c9a66");
static BLEUUID uartReadUUID("e659f3fe-ea98-11e3-ac10-0800200c9a66");
static BLEUUID uartWriteUUID("e659f3ff-ea98-11e3-ac10-0800200c9a66");
static BLEUUID batteryLifeUUID("e659f303-ea98-11e3-ac10-0800200c9a66");
static BLEUUID speedUUID("e659f30b-ea98-11e3-ac10-0800200c9a66");
static BLEUUID tripUUID("e659f30a-ea98-11e3-ac10-0800200c9a66");
static BLEUUID odometerUUID("e659f319-ea98-11e3-ac10-0800200c9a66");
static BLEUUID lastErrorCodeUUID("e659f31c-ea98-11e3-ac10-0800200c9a66");
static BLEUUID statusCodeUUID("e659f30f-ea98-11e3-ac10-0800200c9a66");
static BLEUUID ridingModeUUID("e659f302-ea98-11e3-ac10-0800200c9a66");
static BLEUUID customNameUUID("e659f3fd-ea98-11e3-ac10-0800200c9a66");
static BLEUUID safetyHeadroomUUID("e659f317-ea98-11e3-ac10-0800200c9a66");
static BLEUUID batteryCellsUUID("e659f31b-ea98-11e3-ac10-0800200c9a66");
static BLEUUID batteryTempUUID("e659f315-ea98-11e3-ac10-0800200c9a66");
static BLEUUID batteryVoltageUUID("e659f316-ea98-11e3-ac10-0800200c9a66");

static BLEClient*  pClient;
static BLEAddress *pServerAddress;
static BLERemoteService* pRemoteService;
static BLERemoteCharacteristic* pRemoteCharacteristicFirmware;
static BLERemoteCharacteristic* pRemoteCharacteristicUart;
static BLERemoteCharacteristic* pRemoteCharacteristicUartWrite;
static BLERemoteCharacteristic* pRemoteCharacteristicBatteryLife;
static BLERemoteCharacteristic* pRemoteCharacteristicSpeedRPM;
static BLERemoteCharacteristic* pRemoteCharacteristicOdometer;
static BLERemoteCharacteristic* pRemoteCharacteristicTrip;
static BLERemoteCharacteristic* pRemoteCharacteristicLastError;
static BLERemoteCharacteristic* pRemoteCharacteristicStatusError;
static BLERemoteCharacteristic* pRemoteCharacteristicRidingMode;
static BLERemoteCharacteristic* pRemoteCharacteristicCustomName;
static BLERemoteCharacteristic* pRemoteCharacteristicSafetyHeadroom;
static BLERemoteCharacteristic* pRemoteCharacteristicBatteryCells;
static BLERemoteCharacteristic* pRemoteCharacteristicBatteryTemp;
static BLERemoteCharacteristic* pRemoteCharacteristicBatteryVoltage;

/*
 * State Engine Definition
 */
int STATE = -1;
int STATE_PREV = -1;
int ST_FACTORY = 0;
int ST_SCAN = 1;
int ST_REGISTER = 2;
int ST_UNREGISTER = 3;
int ST_CHARGING = 5;
int ST_CONNECTING = 10;
int ST_UNLOCK = 15;
int ST_DASH_BLE = 20;
int ST_DASH_1 = 31;
int ST_DASH_2 = 32;
int ST_DASH_3 = 33;
int ST_DASH_4 = 34;
int ST_DASH_5 = 35;
int ST_DEEP_SLEEP = 100;

bool state_enter = false;  // State Transition Flag (First Scan)
int SUB_STATE = 0; // This is garbage, fix it

//0.96" OLED Display
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// 1.3" OLED Display from M5Stack "M5 Stick"
//U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R1, /* cs=*/ 14, /* dc=*/ 27, /* reset=*/ 33);

// 1.3" SH1106 (SDA-17,SCL-16)
// DSTIKE OLED Watch
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);


Bounce debouncer = Bounce(); // Instantiate a Bounce object
unsigned long buttonPressTimeStamp;
bool pressed = false;
int bling_press = 5;

/*
 * Update Timers
 */
int dash_update_rate = 100;
int dash_update_rate_slow = 1000;
int dash_last_update = 0;
int dash_last_update_slow = 0;
int unlock_last_update = 0;

uint8_t unlock_challenge[20];
uint8_t unlock_prehash[33];
uint8_t unlock_resp[20];
uint8_t unlock_password[16] = {0xD9,0x25,0x5F,0x0F,0x23,0x35,0x4E,0x19,0xBA,0x73,0x9C,0xCD,0xC4,0xA9,0x17,0x65};

static int ow_rssi = -200;
static int ow_status_code = -1;
static int ow_last_error_code = -1;
static int ow_riding_mode = -1;
static int ow_batt_remaining = 0;
static float ow_odom_trip = 0;
static int ow_odom_life = 0;
static int ow_speed_rpm = 0;
static float ow_speed_mph = 0;
static float ow_batt_voltage = 0.0;
static float ow_batt_cells[16] = {0.0};
static float ow_batt_cell_min = 0.0;
static float ow_batt_cell_avg = 0.0;
static float ow_batt_cell_max = 0.0;
static int ow_safety_headroom = 100.0;

static bool ow_detected_pad1 = false;
static bool ow_detected_pad2 = false;
static bool ow_detected_rider = false;
static bool ow_charging = false;
static bool ow_charge_stats_reset = false;
static boolean connected = false;
static boolean unlocked = false;
static int challenge_byte_count = 0;
static uint8_t firmware_out[2];

int retry_count = 0;

int battery_value = 0;
bool config_ow_goofy = true;

const int wdtTimeout = 8000;  //time in ms to trigger the watchdog
hw_timer_t *wdTimer = NULL;

int idle_rate = 60000;
int idle_last_update = 0;

int cells_last_update = 0;
#endif // _OWWHEADER_H    // Put this line at the end of your file.

RTC_DATA_ATTR int boot_count = 0;
