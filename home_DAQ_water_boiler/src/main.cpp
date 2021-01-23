// Include the libraries we need
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

#define AVERAGING 10

const char* ssid = "MATRIXHOME";
const char* password = "9876543210";

// ThingSpeak information
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 1257117;
char* writeAPIKey = "YJ0ZBKDHD135BTSG";

// Global variables
float temp_in = 0.0;
float temp_out = 0.0;

WiFiClient client;

// Data wire is plugged into port 2 on the Arduino
// DS1820 init (out)
#define DS_PIN_IN  D10
#define DS_PIN_OUT D13
OneWire oneWireDS_in(DS_PIN_IN);
DallasTemperature sens_DS_in(&oneWireDS_in);
OneWire oneWireDS_out(DS_PIN_OUT);
DallasTemperature sens_DS_out(&oneWireDS_out);

float temps_in[AVERAGING];
float sum_in = 0.0;
float temps_out[AVERAGING];
float sum_out = 0.0;


int connectWiFi() {

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

  // set the fields with the values

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



/*
   Setup function. Here we do the basics
*/
void setup(void) {
  // power-up temperature sensor
  pinMode(D11, OUTPUT);
  digitalWrite(D11, HIGH);
  pinMode(D12, OUTPUT);
  digitalWrite(D12, HIGH);
  delay(2000);

  // start serial port
  Serial.begin(19200);

  // connect to WiFi
  connectWiFi();

  // init temp sensor DS1820
  sens_DS_in.begin();
  sens_DS_out.begin();
  delay(2000);
}

void loop(void)
{
  for (byte i = 0; i < AVERAGING; i++) {
    sens_DS_in.requestTemperatures();
    sens_DS_out.requestTemperatures();
    temps_in[i] = sens_DS_in.getTempCByIndex(0);
    temps_out[i] = sens_DS_out.getTempCByIndex(0);

    Serial.print(i + 1);
    Serial.print(". Inlet temperature: ");
    Serial.print(temps_in[i], 1);
    Serial.print(" deg. C | Outlet temperature: ");
    Serial.print(temps_out[i], 1);
    Serial.println(" deg. C");
    delay(55000);
  }

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
