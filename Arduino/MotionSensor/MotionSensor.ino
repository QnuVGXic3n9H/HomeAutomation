#define MY_RADIO_NRF24
#define MY_BAUD_RATE 38400
#define MY_DEBUG

#include <MyConfig.h>
#include <MySensors.h>

//sensor IDs
#define SENSOR_ID_PIR 0
#define SENSOR_ID_RELAY_PIR 1

// Arduino Digital I/O pin number
#define PIN_PIR 3
#define PIN_PIR_RELAY 4

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

#define SLEEP_TIME 15000     // Sleep time between reports (in milliseconds)
#define SHORT_SLEEP_TIME 200 // Short sleep time

MyMessage pirMsg(SENSOR_ID_PIR, V_TRIPPED);
MyMessage pirRelayMsg(SENSOR_ID_RELAY_PIR, V_STATUS);

enum RelayStatus
{
  RELAY_STATUS_UNKNOWN,
  RELAY_STATUS_OFF,
  RELAY_STATUS_ON
};

RelayStatus pirRelayStatus = RELAY_STATUS_UNKNOWN;

void controlPirRelay(RelayStatus state)
{
  if (pirRelayStatus != state)
  {
    Serial.print("PIR relay ");
    Serial.println(state == RELAY_STATUS_ON ? "ON" : "OFF");
    pirRelayStatus = state;

    //send updated PIR state
    if (pirRelayStatus != RELAY_STATUS_ON)
      send(pirMsg.set(false));

    digitalWrite(PIN_PIR_RELAY, pirRelayStatus == RELAY_STATUS_ON ? RELAY_ON : RELAY_OFF);
  }
}

// the setup function runs once when you press reset or power the board
void setup()
{
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_PIR_RELAY, OUTPUT);

  digitalWrite(PIN_PIR_RELAY, RELAY_OFF);

  Serial.println("Node is running");
}

void presentation()
{
  sendSketchInfo("Motion Sensor", "1.0");

  present(SENSOR_ID_PIR, S_MOTION, "PIR sensor");
  present(SENSOR_ID_RELAY_PIR, S_BINARY, "PIR relay", true);
}

void loop()
{
  //request initial values for all relays
  request(SENSOR_ID_RELAY_PIR, V_STATUS);
  smartSleep(SHORT_SLEEP_TIME);

  //don't act until all required params are known
  if (pirRelayStatus == RELAY_STATUS_UNKNOWN)
  {
    Serial.println("PIR relay status unknown, requesting it from controller");
    return;
  }

  if (pirRelayStatus == RELAY_STATUS_ON)
  {
    // Read digital motion value
    boolean tripped = digitalRead(PIN_PIR) == HIGH;

    Serial.print("PIR tripped: ");
    Serial.println(tripped);
    send(pirMsg.set(tripped)); // Send tripped value to gw

    // Sleep until interrupt comes in on motion sensor
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
      Serial.print("Incoming change for actuator:");
      Serial.print(message.sensor);
      Serial.print(", New status: ");
      Serial.println(message.getBool());

      controlPirRelay(message.getBool() ? RELAY_STATUS_ON : RELAY_STATUS_OFF);
    }
  }
  else
  {
    Serial.print("Unknown sensor ID: ");
    Serial.println(message.sensor);
  }
}

