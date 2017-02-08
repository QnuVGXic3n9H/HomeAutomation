//#define MY_NODE_ID 100
#define MY_RADIO_NRF24
#define MY_BAUD_RATE 38400
#define MY_DEBUG

//#include <SPI.h>
//#include <MyConfig.h>
#include <MySensors.h>

//sensor IDs
#define SENSOR_ID_PIR 0
#define SENSOR_ID_RELAY_PIR 1
#define SENSOR_ID_RELAY_SIREN 2

// Arduino Digital I/O pin number
#define PIN_PIR 3
#define PIN_PIR_RELAY 4
#define PIN_SIREN 5

#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

#define SLEEP_TIME 15000 // Sleep time between reports (in milliseconds)

MyMessage pirMsg(SENSOR_ID_PIR, V_TRIPPED);
MyMessage pirRelayMsg(SENSOR_ID_RELAY_PIR, V_STATUS);
MyMessage sirenMsg(SENSOR_ID_RELAY_PIR, V_VAR1);

enum RelayStatus
{
  RELAY_STATUS_UNKNOWN,
  RELAY_STATUS_OFF,
  RELAY_STATUS_ON
} ;

RelayStatus pirRelayStatus = RELAY_STATUS_UNKNOWN;
RelayStatus sirenRelayOn = RELAY_STATUS_OFF;

unsigned long sirenStopsAt = 0;

void controlPirRelay(RelayStatus state)
{
  if (pirRelayStatus != state)
  {
    Serial.print("PIR relay ");
    Serial.println(state == RELAY_STATUS_ON ? "ON" : "OFF");
    pirRelayStatus = state;
    digitalWrite(PIN_PIR_RELAY, state == RELAY_STATUS_ON ? RELAY_ON : RELAY_OFF);

    //send updated PIR state
    if (pirRelayStatus != RELAY_STATUS_ON)
      send(pirMsg.set(false));
  }
}

void controlSiren(unsigned long delay)
{
  if (sirenRelayOn != RELAY_STATUS_ON && delay > 0)
  {
    Serial.print("Siren ON for ");
    Serial.print(delay);
    sirenRelayOn = RELAY_STATUS_ON;
    digitalWrite(PIN_SIREN, RELAY_ON);
    sirenStopsAt = millis() + delay;
  }
  else if (sirenRelayOn == RELAY_STATUS_ON)
  {
    if (delay == 0)
    {
      Serial.print("Siren OFF");
      sirenRelayOn = RELAY_STATUS_OFF;
      digitalWrite(PIN_SIREN, RELAY_OFF);
      sirenStopsAt = 0;
    }
    else
    {
      Serial.print("Siren ON for ");
      Serial.print(delay);
      sirenStopsAt = millis() + delay;
    }
  }
}

void handleSirenTimeout()
{
  unsigned long now = millis();
  if (sirenRelayOn == RELAY_STATUS_ON && (now >= sirenStopsAt))
  {
    Serial.print("Siren timeout expired");
    controlSiren(0);
  }

  if (sirenRelayOn != RELAY_STATUS_ON)
    send(sirenMsg.set(0));
  else
    send(sirenMsg.set(sirenStopsAt - now));
}

// the setup function runs once when you press reset or power the board
void setup()
{
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_PIR_RELAY, OUTPUT);
  pinMode(PIN_SIREN, OUTPUT);

  Serial.println("Node is running");
}

void presentation()
{
  sendSketchInfo("Motion Sensor", "1.0");

  present(SENSOR_ID_PIR, S_MOTION, "PIR sensor");
  present(SENSOR_ID_RELAY_PIR, S_BINARY, "PIR relay", true);
  present(SENSOR_ID_RELAY_SIREN, S_CUSTOM, "Siren");
}

void loop()
{
  //requesting status explicitly since it's often missied during smartSleep
  request(SENSOR_ID_RELAY_PIR, V_STATUS);

  handleSirenTimeout();

  if (pirRelayStatus == RELAY_STATUS_ON)
  {
    // Read digital motion value
    boolean tripped = digitalRead(PIN_PIR) == HIGH;

    Serial.print("PIR tripped: ");
    Serial.println(tripped);
    send(pirMsg.set(tripped)); // Send tripped value to gw

    // Sleep until interrupt comes in on motion sensor. Send update every two minute.
    smartSleep(digitalPinToInterrupt(PIN_PIR), CHANGE, SLEEP_TIME);
  }
  else
    smartSleep(SLEEP_TIME);
}

void receive(const MyMessage &message)
{
  Serial.print("Msg received: ");
  Serial.println(message.type);

  if (message.sensor == SENSOR_ID_RELAY_PIR)
  {
    if (message.type == V_STATUS)
    {
      // Write some debug info
      Serial.print("Incoming change for sensor:");
      Serial.print(message.sensor);
      Serial.print(", New status: ");
      Serial.println(message.getBool());

      controlPirRelay(message.getBool() ? RELAY_STATUS_ON : RELAY_STATUS_OFF);
    }
  }
  else if (message.sensor == SENSOR_ID_RELAY_SIREN)
  {
    if (message.type == V_VAR1)
    {
      // Write some debug info
      Serial.print("Incoming change for sensor:");
      Serial.print(message.sensor);
      Serial.print(", New value: ");
      Serial.println(message.getULong());

      controlSiren(message.getULong());
    }
  }
  else
  {
    Serial.print("Unknown sensor ID: ");
    Serial.println(message.sensor);
  }
}
