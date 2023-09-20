#include <Arduino.h>
#include <SensirionI2cSf06Lf.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>

SensirionI2cSf06Lf sensor;
File logFile;
//SPIClass SPI(HSPI);

void setup_fuelflow();

static char errorMessage[128];
static int16_t error;
const int numReadings  = 1000;
float readings [numReadings];
int readIndex  = 0;
float total  = 0.0;
float average;
uint64_t start = esp_timer_get_time();
float m = 1;//14.378;
float c = 0;//-68.319;
float total_consumed = 0.0;
float total_consumed_counter_track = 0.0;
bool sdstatus;


void setup_fuelflow() {
    Wire.begin();

    sensor.begin(Wire, SF06_LF_I2C_ADDRESS);
    sensor.stopContinuousMeasurement();
    
    delay(100);

    uint32_t productIdentifier = 0;
    uint8_t serialNumber[8] = {0};
    

    error = sensor.startH2oContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print(
            "Error trying to execute startH2oContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}


void setup() {
    pinMode(D0, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    

    Serial.begin(115200);

    setup_fuelflow();
    
    SPI.begin(D8, D9, D10, D7);

    sdstatus = !SD.begin(D7, SPI);
    if (!sdstatus) {
        Serial.println("Card Mount Failed");
    }
    else {
        logFile = SD.open("ff.txt", FILE_WRITE);
        Serial.println("SD started");
    }
    
}

void loop() {
    while (esp_timer_get_time() - start < 5000) {
        NOP();
    };
    start = esp_timer_get_time();
    digitalWrite(D0, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    
    float aFlow = 0.0;
    float aTemperature = 0.0;
    uint16_t aSignalingFlags = 0u;


    error = sensor.readMeasurementData(INV_FLOW_SCALE_FACTORS_SLF3S_4000B, aFlow, aTemperature, aSignalingFlags);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurementData(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }


    total = total - readings[readIndex];
    readings[readIndex] = constrain(m*aFlow+c, 0, 1000);
    total = total + readings[readIndex];
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) { readIndex = 0; };
    average = total / numReadings;
    total_consumed = total_consumed + average * 0.005;


    if(total_consumed - total_consumed_counter_track > 1) {
        total_consumed_counter_track = total_consumed + constrain(total_consumed - 1 - total_consumed_counter_track, 0, 1);
        digitalWrite(D0, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println(total_consumed_counter_track);
    };

    Serial.println(aFlow);

    if(sdstatus){ 
        logFile.print(esp_timer_get_time());    logFile.print(",");
        logFile.print(aFlow);                   logFile.print(",");
        logFile.print(average);                 logFile.print(",");
        logFile.print(total_consumed);          logFile.print(",");
        logFile.print(aTemperature);            logFile.print(",");
        logFile.print(aSignalingFlags);         logFile.println();
        logFile.flush();
    }
}