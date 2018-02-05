#include <Arduino.h>
#include <Homie.h>
#include <LightDependentResistor.h>

// Adjust these two based on your HW
#define PHOTOCELL_TYPE LightDependentResistor::GL5528
#define PHOTOCELL_RESISTOR 15000 /*ohm*/
//#define PHOTOCELL_RESISTOR 21700 /*ohm*/

#define LED_PIN LED_BUILTIN
#define PIR_PIN D5
#define PHOTOCELL_PIN A0

#define PHOTOCELL_TOLERANCE 15 /* % */
#define PHOTOCELL_READING_INTERVAL 100 /* ms */


int brightness = PWMRANGE;
bool led_on = false;
unsigned long photocell_report_interval = 60000; //ms
unsigned long photocell_report_interval_start = 0;
unsigned long photocell_reading_interval_start = 0;
float photocell_last_value = 0;
bool motion_detected = false;

LightDependentResistor photocell(PHOTOCELL_PIN, PHOTOCELL_RESISTOR, PHOTOCELL_TYPE );

HomieNode ledNode("led", "switch");
HomieNode pirNode("pir", "sensor");
HomieNode photocellNode("photocell", "sensor");

void updateLed() {
  if (brightness == 0) {
    led_on = false;
  }

  if (led_on) {
    analogWrite(LED_PIN, PWMRANGE-brightness);
  } else {
    analogWrite(LED_PIN, PWMRANGE);
  }

  ledNode.setProperty("brightness").send(String(brightness));
  ledNode.setProperty("status").send(led_on ? "ON" : "OFF");
}


bool ledBrightnessHandler(const HomieRange& range, const String& value) {
  brightness = value.toInt();
  if (brightness > PWMRANGE) { brightness = PWMRANGE; }
  if (brightness < 0) { brightness = 0; }

  updateLed();
  return true;
}

bool ledStatusHandler(const HomieRange& range, const String& value) {
  if ((value != "ON") && (value != "OFF")) return false;

  if (value == "ON") {
    led_on = true;
  } else {
    led_on = false;
  }

  updateLed();
  return true;
}

bool photocellIntervalHandler(const HomieRange& range, const String& value) {
   int v = value.toInt();
   if (v != 0) {
     photocell_report_interval = v * 1000;
     photocellNode.setProperty("interval").send(String(v));
   }
  return true;
}

void photocellLoopHandler() {
  unsigned long now = millis();

  if ((now - photocell_reading_interval_start) >= PHOTOCELL_READING_INTERVAL) {
      float lux = photocell.getCurrentLux();
      int raw = analogRead(PHOTOCELL_PIN);

      float luxdiff = (lux - photocell_last_value);
      float pdiff = 100 * (luxdiff / photocell_last_value);
      bool quick_update = false;
      if ((abs(luxdiff) > 1.0) and (abs(pdiff) > PHOTOCELL_TOLERANCE)) {
        quick_update = true;
      }

      /*
      Serial.print("Lux: ");
      Serial.print(lux);
      Serial.print(" (");
      Serial.print(pdiff);
      Serial.println(" %)");
      */

      if (quick_update) {
          photocellNode.setProperty("lux").send(String(photocell_last_value));
      }

      if (((now - photocell_report_interval_start) >= photocell_report_interval) or quick_update) {
          photocellNode.setProperty("lux").send(String(lux));
          photocellNode.setProperty("raw").send(String(raw));
          photocell_last_value = lux;
          photocell_report_interval_start = now;
      }

      photocell_reading_interval_start = now;
  }
}

void pirLoopHandler() {
  if (motion_detected != (digitalRead(PIR_PIN))) {
       motion_detected = digitalRead(PIR_PIN);
       pirNode.setProperty("motion").send(
         motion_detected ? "YES" : "NO"
       );
  }
}

void loopHandler() {
  photocellLoopHandler();
  pirLoopHandler();
}

void setupHandler() {
  ledNode.setProperty("brightness").send(String(brightness));
  ledNode.setProperty("status").send("OFF");
  pirNode.setProperty("motion").send("NO");

  photocell_report_interval_start = millis();
  photocell_reading_interval_start = millis();
  photocell_last_value = photocell.getCurrentLux();

  photocellNode.setProperty("interval").send(String(photocell_report_interval/1000));
  photocellNode.setProperty("lux").send(String(photocell_last_value));
  photocellNode.setProperty("raw").send(String(analogRead(PHOTOCELL_PIN)));

}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl << "[PIR_sensor]" << endl;
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  photocell.setPhotocellPositionOnGround(false);

  Homie.setLedPin(LED_PIN, LOW);
  Homie_setFirmware("pir_sensor", "0.1.0");
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);

  ledNode.advertise("brightness").settable(ledBrightnessHandler);
  ledNode.advertise("status").settable(ledStatusHandler);

  pirNode.advertise("motion");
  //photocellNode.advertise("lux");
  photocellNode.advertise("raw");
  photocellNode.advertise("interval").settable(photocellIntervalHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
}
