#include "Setup.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <DFRobot_QMC5883.h>
#include <movingAvg.h>                  // https://github.com/JChristensen/movingAvg

DFRobot_QMC5883 compass(&Wire, /*I2C addr*/QMC5883_ADDRESS);
WiFiClient espClient;
PubSubClient client(espClient);

movingAvg avgFieldStrength(20);


String ip;
char charBuf[15];

void setup()
{
  Serial.begin(SERIAL_SPEED);
  setupWifi();
  setupWebUpdater();
  
  while (!compass.begin())
  {
    Serial.println("Could not find a valid 5883 sensor, check wiring!");
    delay(500);
  }

  avgFieldStrength.begin();
  compass.setRange(QMC5883_RANGE_8GA);


  setupMqtt();

}


int fieldStrength;
int avg;
int lastAvg;
unsigned long timer = millis();

void loop()
{
  loopMqtt();


  sVector_t mag = compass.readRaw();

  // calculate field strength & store for average calculation
  fieldStrength = sqrt(mag.XAxis ^ 2 + mag.YAxis ^ 2 + mag.ZAxis ^ 2);
  avg = avgFieldStrength.reading(fieldStrength > 4000 ? 4000 : fieldStrength);
  
  Serial.print("Arrow: "); Serial.println(fieldStrength);  
  // optional: send raw data via MQTT for e.g. debugging
  // client.publish("mqtt.0.gas_meter.raw", itoa(fieldStrength, charBuf, 10));

  // every 10 seconds: evaluate if signall falls from high (magnet detected) to low (no magnet)
  if ((unsigned long)(millis() - timer) > 10000) {
    Serial.print("Avg: "); Serial.println(avg);
    Serial.print("Last Avg: "); Serial.println(lastAvg);

    client.publish("mqtt.0.gas_meter.avg", itoa(avg, charBuf, 10));
    client.publish("mqtt.0.gas_meter.lastAvg", itoa(lastAvg, charBuf, 10));

    // if signal falls from over 2000 to under 2000 in the average of the last 10 seconds: 
    // trigger "tick" via MQTT => 0.1m3 of gas have been consumed
    if (avg < 2000 and lastAvg >= 2000) {
      client.publish("mqtt.0.gas_meter.tick", itoa(millis(), charBuf, 10));
    }
    timer = millis();
    lastAvg = avg;
  }


  loopWebUpdater();
  delay(500);
}
