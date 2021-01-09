
//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_WARN, {{"app", LOG_LEVEL_TRACE}});

int accel = 0;
int buttonPin = D5;
int ledPin = D4;
bool runBlink = false;
bool publishSampleData = true;
String data = "";
String STATION_A = "A";
String STATION_B = "B";

void setup()
{

  Log.info("Starting application setup.");
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  if (!Particle.connected())
  {
    Particle.connect();
  }

  if (Particle.connected())
  {
    Log.info("Connected to the Particle Cloud.");
  }
  else
  {
    Log.error("Could not connect to the Particle Cloud.");
  }
}

void loop()
{

  if (digitalRead(buttonPin) == LOW)
  {
    if (publishSampleData)
    {
      accel++;
      Log.trace("%d", accel);
      data = String::format(
          "{\"station_a\":\"A\", \"station_b\":\"B\", \"accel\":%d}", accel);
      Log.trace("Publishing Data");
      Log.trace(data);
      digitalWrite(ledPin, HIGH);
      Particle.publish("sampleData", data, PRIVATE);
      publishSampleData = false;
    }
  }
  else
  {
    digitalWrite(ledPin, LOW);
    Log.trace("Push Button (D5) to publish data");
    publishSampleData = true;
  }
  Log.trace("******");
  delay(1000);
}
