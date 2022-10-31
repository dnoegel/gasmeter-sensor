# What does this do?
This is an arduino sketch to track a gas meter with an _ESP32_ and _QMC5883_ sensor. The sensor is attached to the gas meter and will detect rotations of the counter and report them via Mqtt.

# Requirements
* Gas meter with a magnet on one register/counter (eg. G4 RF1 style gas meters)
* ESP32 microcontroller
* QMC5883 chip (HMC5883 is also supported by the used library, you will need to adjust the sketch correspondingly)
* A mqtt server (if you want to use that - you can still edit the sketch to do something else with that information)

# How does it work
Some gas meters have a magnet on the counter for the second decimal place. In my case it is nearby the "6". If you now place a sensor on the top of the counter and the 6 passes by, you can detect the magnet with an increase in the magnet field strength (in case of a compass sensor) or by a contact closing (in case of a reed sensor). 
This again means: Whenever you detect that magnet, the counter for the second decimal place did a full rotation - which again means that the first decimal place increased by one. Which in my cases indicates a gas consumption of 0.1m³ - or 1.0kWh.

# Why not another sensor?
- I tried with a reed sensor in an aqara door- and window contact: it didn't work out as it was not sensitive enough
- I was unsure which hall sensor to use
- I found [this nice blog post on kompf.de](https://www.kompf.de/tech/gascountmag.html) and liked the idea to use a (quite sensitive) compass sensor


# Setup

Rename `Setup.h.orig` to  `Setup.h` and adjust the settings. The sketch will publish the MQTT debug event `mqtt.0.gas_meter.debug` - from this you can tell how to configure `TRIGGER_HIGH` and `TRIGGER_LOW` best. Be aware that in some situations the sensor will be measuring high values for hours (e.g. when the digit with the magnet is parked right under the sensor). From my experience `TRIGGER_LOW` should therefore be rather low, so that it is not in the range of the standard error of the sensor. 



# 3D model for a holder
A matching holder for the sensor can be found here: https://www.thingiverse.com/thing:5581975
