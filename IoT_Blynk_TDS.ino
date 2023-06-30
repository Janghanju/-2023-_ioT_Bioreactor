
// Original source code: https://wiki.keyestudio.com/KS0429_keyestudio_TDS_Meter_V1.0#Test_Code
// Project details: https://RandomNerdTutorials.com/arduino-tds-water-quality-sensor/
// TDS
#define TdsSensorPin 39
#define VREF 3.3   // analog reference voltage(Volt) of the ADC
#define SCOUNT 30  // sum of sample point

// IoT Controlled LED Using ESP32 with Blynk App : https://linuxhint.com/esp32-blynk-app-led-control-iot/
// WIFI + 블링크
#define BLYNK_PRINT Serial  /*  include Blynk Serial */
#include <WiFi.h>    /*ESP32 WiFi Library*/
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Enter device Authentication Token
char auth[] ="QN6sKlapOZrg1w1Ivz5A204B5DdxiicP";
#define BLYNK_TEMPLATE_ID "TMPL6YIHA6kVv"
#define BLYNK_TEMPLATE_NAME "2023 Bioreactor project"

// Vpin 설정
#define VPIN_TDS V0 // menentukan nama pin dalam blynk ( V0 dpilih semasa menentukan button di app blynk) 
#define VPIN_EC V1

//Enter your WIFI SSID and password
char ssid[] = "ASUS-SERVER_2.4GHz";
char pass[] = "Jace_Wing159159";


int analogBuffer[SCOUNT];  // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float ecValue = 0;
float temperature = 28.9;  // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup() {
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

void loop() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {  //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);  //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      //temperature compensation
      float compensationVoltage = averageVoltage / compensationCoefficient;

      //convert voltage value to tds value
      tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;

      ecValue = tdsValue * 0.64;

      Serial.print("voltage:");
      Serial.print(averageVoltage,2);
      Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue, 0);
      Serial.print("ppm ");
      Serial.print("EC Value:");
      Serial.print(ecValue, 0);
      Serial.println("μS/cm");

    }
  }
  Blynk.run();
  Blynk.virtualWrite(VPIN_TDS, tdsValue);
  Blynk.virtualWrite(VPIN_EC, ecValue);

}