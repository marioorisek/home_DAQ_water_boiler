// Include the libraries we need
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include "cfg.h"
#include "wifi_cfg.h"

// Global variables
float temp_in = 0.0;
float temp_out = 0.0;

WiFiClient client;

OneWire oneWireDS_in(DS_PIN_IN);
OneWire oneWireDS_out(DS_PIN_OUT);

DallasTemperature sens_DS_in(&oneWireDS_in);
DallasTemperature sens_DS_out(&oneWireDS_out);

float temps_in[AVERAGING];
float sum_in = 0.0;
float temps_out[AVERAGING];
float sum_out = 0.0;


void connectWiFi() {

  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin( ssid, password );
    Serial.println("Connecting to WiFi");
    delay(10000);
  }

  Serial.println();
  Serial.print("\nConnected to: " );
  Serial.print(ssid);
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin( client );
}

// Use this function if you want to write a single field.
int writeTSData( long TSChannel, unsigned int TSField, float data ) {
  int  writeSuccess = ThingSpeak.writeField( TSChannel, TSField, data, writeAPIKey ); // Write the data to the channel
  if ( writeSuccess ) {

    Serial.println( String(data) + " written to Thingspeak." );
  }

  return writeSuccess;
}

void writeMultipleTSData() {

  // write multiple fields to TS channel

 ThingSpeak.setField(2, temp_in);
 ThingSpeak.setField(3, temp_out);

 // write to the ThingSpeak channel
 int x = ThingSpeak.writeFields(channelID, writeAPIKey);
 if(x == 200){
   Serial.println("Channel update successful.");
 }
 else{
   Serial.println("Problem updating channel. HTTP error code " + String(x));
 }
}

void setup(void) {
  // power-up temperature sensors
  pinMode(SENSOR_PWR_1, OUTPUT);
  digitalWrite(SENSOR_PWR_1, HIGH);
  pinMode(SENSOR_PWR_2, OUTPUT);
  digitalWrite(SENSOR_PWR_2, HIGH);
  delay(2000);

  // start serial port
  Serial.begin(19200);

  // connect to WiFi
  connectWiFi();

  // init temp sensors DS1820
  sens_DS_in.begin();
  sens_DS_out.begin();
  delay(2000);
}

void loop(void)
{
  for (byte i = 0; i < AVERAGING; i++) {
    // request temperature readings
    sens_DS_in.requestTemperatures();
    sens_DS_out.requestTemperatures();

    // store readings
    temps_in[i] = sens_DS_in.getTempCByIndex(0);
    temps_out[i] = sens_DS_out.getTempCByIndex(0);

    Serial.print(i + 1);
    Serial.print(". Inlet temperature: ");
    Serial.print(temps_in[i], 1);
    Serial.print(" deg. C | Outlet temperature: ");
    Serial.print(temps_out[i], 1);
    Serial.println(" deg. C");
    delay(58000);
  }

  //  temperature averaging for ThingSpeak Cloud

  sum_in = 0.0;
  sum_out = 0.0;

  for (byte i = 0; i < AVERAGING; i++) {
    sum_in += temps_in[i];
    sum_out += temps_out[i];
  }

  temp_in = sum_in / float(AVERAGING);
  temp_out = sum_out / float(AVERAGING);

  Serial.print(AVERAGING);
  Serial.print(" min inlet average temperature: ");
  Serial.print(temp_in, 1);
  Serial.print(" deg. C | outlet average temperature: ");
  Serial.print(temp_out, 1);
  Serial.println(" deg. C");
  writeMultipleTSData();

}
