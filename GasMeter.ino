#include "Setup.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <DFRobot_QMC5883.h>
#include <movingAvg.h>                  // https://github.com/JChristensen/movingAvg

DFRobot_QMC5883 compass(&Wire, /*I2C addr*/QMC5883_ADDRESS);
WiFiClient espClient;
PubSubClient client(espClient);

movingAvg avgFieldStrength(40);


String ip;
char charBuf[40];

void setup()
{
  Serial.begin(SERIAL_SPEED);
  ensureWifiConnected();
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
bool isHigh = false;
unsigned long timer = millis();

void loop()
{
  loopMqtt();

  sVector_t mag = compass.readRaw();

  // calculate field strength & store for average calculation
  fieldStrength = sqrt(mag.XAxis ^ 2 + mag.YAxis ^ 2 + mag.ZAxis ^ 2);
  avg = avgFieldStrength.reading(fieldStrength > MAX_FIELD_STRENGTH ? MAX_FIELD_STRENGTH : fieldStrength);

  Serial.print("Arrow: "); Serial.println(fieldStrength);

  // every 10 seconds: evaluate if signall falls from high (magnet detected) to low (no magnet)
  if ((unsigned long)(millis() - timer) > 10000) {
    Serial.print("Avg: "); Serial.println(avg);
    Serial.print("isHigh: "); Serial.println(isHigh);

    sprintf(charBuf, "avg: %i - high: %d", avg, isHigh);
    client.publish("mqtt.0.gas_meter.debug", charBuf);

    if (avg < TRIGGER_LOW and isHigh) {
      isHigh = false;
      client.publish("mqtt.0.gas_meter.tick", itoa(millis(), charBuf, 10));
      avgFieldStrength.reset();
    }

    if (avg > TRIGGER_HIGH) {
      isHigh = true;
    }


    timer = millis();
  }


  loopWebUpdater();
  delay(250);
}
