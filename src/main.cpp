#include <Arduino.h>
#include <SensirionI2cSf06Lf.h>
#include <Wire.h>

SensirionI2cSf06Lf sensor;

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


void print_byte_array(uint8_t* array, uint16_t len) {
    uint16_t i = 0;
    Serial.print("0x");
    for (; i < len; i++) {
        Serial.print(array[i], HEX);
    }
}

void setup() {
    pinMode(D0, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    

    Serial.begin(115200);
    Wire.begin();
    sensor.begin(Wire, SF06_LF_I2C_ADDRESS);

    sensor.stopContinuousMeasurement();
    delay(100);
    uint32_t productIdentifier = 0;
    uint8_t serialNumber[8] = {0};
    error = sensor.readProductIdentifier(productIdentifier, serialNumber, 8);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readProductIdentifier(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("productIdentifier: ");
    Serial.print(productIdentifier);
    Serial.print("\t");
    Serial.print("serialNumber: ");
    print_byte_array(serialNumber, 8);
    Serial.println();
    error = sensor.startH2oContinuousMeasurement();
    if (error != NO_ERROR) {
        Serial.print(
            "Error trying to execute startH2oContinuousMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
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


    // subtract the last reading:
    total = total - readings[readIndex];
    // read the sensor:
    readings[readIndex] = constrain(m*aFlow+c, 0, 1000);
    // add value to total:
    total = total + readings[readIndex];
    // handle index
    readIndex = readIndex + 1;
    if (readIndex >= numReadings) {
        readIndex = 0;
    }
    // calculate the average:
    average = total / numReadings;

    total_consumed = total_consumed + average * 0.005;

    if(total_consumed - total_consumed_counter_track > 1) {
        total_consumed_counter_track = total_consumed + constrain(total_consumed - 1 - total_consumed_counter_track, 0, 1);
        digitalWrite(D0, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println(total_consumed_counter_track);
    };

    Serial.print(esp_timer_get_time());
    Serial.print("\t");
    Serial.print("aFlow: ");
    Serial.print(aFlow);
    Serial.print("\t");
    Serial.print("calFlow: ");
    Serial.print(average);
    Serial.print("\t");
    Serial.print("total: ");
    Serial.print(total_consumed);
    Serial.print("\t");
    Serial.print("aTemperature: ");
    Serial.print(aTemperature);
    Serial.print("\t");
    Serial.print("aSignalingFlags: ");
    Serial.print(aSignalingFlags);
    Serial.println();



}