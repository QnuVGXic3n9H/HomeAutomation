#include <SPI.h>

// Arduino Digital I/O pin number
#define PIN_PIR 3
#define PIN_SIREN 5

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define BOOT_BEEP_DURATION 100
#define ARMED_BEEP_DURATION 500
#define WARM_UP_TIME 30000             // Delay until armed (in milliseconds)
#define SIREN_DURATION (3 * 60 * 1000) // Time siren will be ON after been triggered by PIR

enum SensorState
{
  STATE_BOOTING,
  STATE_WARMING_UP,
  STATE_ARMED
};

unsigned long trippedSince = 0;
boolean lastSirenState = false;
SensorState currentState = STATE_BOOTING;

void controlSiren(boolean state)
{
  digitalWrite(PIN_SIREN, state ? RELAY_ON : RELAY_OFF);
  if (lastSirenState != state)
  {
    Serial.print("Siren ");
    Serial.println(state ? "ON" : "OFF");
    lastSirenState = state;
  }
}

void handlePIR()
{
  unsigned long now = millis();
  boolean tripped = digitalRead(PIN_PIR) == HIGH;
  if (tripped)
  {
    if (!trippedSince)
    {
      trippedSince = now + WARM_UP_TIME; //start siren with delay
      Serial.print("Siren will be ON in ");
      Serial.print(WARM_UP_TIME);
      Serial.println("ms");
    }
    else if (now > trippedSince)
      trippedSince = now; //extend siren time
  }
  else
  {
    if (trippedSince)
      if (now - trippedSince > SIREN_DURATION) //siren time expired?
      {
        trippedSince = 0;
        Serial.println("Siren time expired");
      }
  }

  boolean sirenState = trippedSince && (now >= trippedSince);

  controlSiren(sirenState);
}

void toggleSiren(unsigned long duration)
{
  controlSiren(true);
  delay(duration);
  controlSiren(false);
}

// the setup function runs once when you press reset or power the board
void setup()
{
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_SIREN, OUTPUT);

  digitalWrite(PIN_SIREN, RELAY_OFF);

  Serial.println("Started");
}

void loop()
{
  switch (currentState)
  {
  case STATE_BOOTING:
    toggleSiren(BOOT_BEEP_DURATION);
    currentState = STATE_WARMING_UP;
    Serial.println("Warming Up");
    break;

  case STATE_WARMING_UP:
    if (millis() >= WARM_UP_TIME)
    {
      toggleSiren(ARMED_BEEP_DURATION);
      currentState = STATE_ARMED;
      Serial.println("Armed");
    }
    break;

  case STATE_ARMED:
    handlePIR();
  }
}
