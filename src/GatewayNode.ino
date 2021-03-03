#include "GatewayBLE.h"
#include "CircularBuffer.h"
#include "CommandHandler.h"
#include "GatewayCommands.h"
#include "Geolocator.h"
#include "PublishQueueAsyncRK.h"

SYSTEM_THREAD(ENABLED);

//#include "spark_wiring_ble.h"
//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_ALL, {{"app", LOG_LEVEL_ALL}});

#define GCP

uint8_t spi_buff[245]; //Hold 40 samples +1 byte id.+ 4 byte frame num

uint8_t publishQueueBuffer[2048];
PublishQueueAsync publishQueue(publishQueueBuffer, sizeof(publishQueueBuffer));

CommandHandler cmdHandler = CommandHandler(&publishQueue);
static char requestBuf[256];

//Setup the input and output pins.
int buttonPin = D5;
int ledPin = D4;

//Gloabl variables
bool runBlink = false;
GatewayBLE BleStack(&publishQueue);
#define NUM_BLE_NODES 1

#define MOSI D12
#define MISO D11
#define CS A5
#define SCK D13
#define HANDSHAKE A4
char buf[6] = "hello";
char spiSendBuf[32] = "SPI transmission - dummy data";

GoogleMapsDeviceLocator locator; // = GoogleMapsDeviceLocator();

int64_t t = 0;

volatile bool spiBusy = false;
void spiDoneHandler();
void connectLTE();
void handleDeviceLocator(const char *event, const char *data);
void testing(const char *event, const char *data)
{
  Serial.println("TESTING");
}

//Particle Functions
int setThreshold(String threshString);
int setMode(String modeString);
int setTxStdY(String txStdYString);

//particle varibale
int threshold = 1;
int prevThreshold = 1;
int mode = 1; 
int particleTxStdY = 0;
void setup()
{

  //UART
  Serial1.begin(115200, SERIAL_DATA_BITS_8 | SERIAL_STOP_BITS_1 | SERIAL_PARITY_NO);

#ifdef DEBUG
  delay(3000);
  Log.info("This is a debug build.");
#endif

  Log.info("Starting application setup.");

  Serial.println("Starting application setup.");
  Serial.begin(9600);
  waitFor(Serial.isConnected, 5000);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(A0, OUTPUT);

  //SPI
  pinMode(MOSI, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(HANDSHAKE, INPUT);
  SPI.setClockSpeed(8000000);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin(CS);
  t = millis();

#ifdef GCP
  Particle.function("setThreshold", setThreshold);
  Particle.function("setMode", setMode);
  Particle.function("setTxStdY", setTxStdY);
  Particle.variable("threshold", threshold);
  Particle.variable("mode", mode);
  Particle.variable("txStdY", particleTxStdY);
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
  while (!Particle.connected())
  {
  }

#endif

  Time.zone(-6); //CST, set to -8 for PST?
  Particle.syncTime();

  //Wait until were connected then log some info.
  if (Time.isValid())
  {
    Log.info("Sending uart command to update time.");

    char buf[255];
    sprintf(buf, "%d: %d/%d/ %dh %dm %ds\n", TIME_UPDATE, Time.month(), Time.day(), Time.hour(), Time.minute(), Time.second());

    Serial1.printf(buf);
  }

  CellularSignal sig = Cellular.RSSI();
  float quality = sig.getQuality(); //Percentage 0 to 100.

  char buf[255];
  sprintf(buf, "%d:%3.1f\n", LTE_RSSI_DATA, quality);
  Serial1.printf(buf);

  BleStack.startBLE();

  //Auto connect
  while (BleStack.numConnections < NUM_BLE_NODES)
  {
    digitalWrite(A0, HIGH);
    BleStack.scanBLE();
    uint8_t numCon = BleStack.connectBLE();
    delay(500);
    char buf[255];
    for (int i = 0; i < numCon; i++)
    {

      sprintf(buf, "%d: %d,%d\n", BLE_CONNECTION_EVENT, 1, i);
      Serial1.printf(buf);

      snprintf(buf, 255, "ble:%d,n:%d,t:%s", 1, i, Time.timeStr().c_str());
      publishQueue.publish("telemetry", buf, PRIVATE);
    }
    digitalWrite(A0, LOW);
  }
  //snprintf(requestBuf, sizeof(requestBuf), "hook-response/%s/%s", "tcm-arm-device-locator", System.deviceID().c_str());

  Particle.subscribe(requestBuf, handleDeviceLocator, MY_DEVICES);
  Particle.subscribe("hook-response/tcm-arm-device-locator", testing, MY_DEVICES);
}

void loop()
{
  if (threshold != prevThreshold)
  {
    Serial.printlnf("THRESHOLD CHANGED FROM %d TO %d", prevThreshold, threshold);
    prevThreshold= threshold;
  }
  // The core of your code will likely live here.
  // if(digitalRead(buttonPin) == LOW){

  //     BleStack.scanBLE();
  //     BleStack.connectBLE();
  // }

  //Try and reconnect BLE if lost.
  if (BleStack.numConnections < NUM_BLE_NODES)
  {
    digitalWrite(A0, HIGH);
    BleStack.scanBLE();
    uint8_t numCon = BleStack.connectBLE();
    for (int i = 0; i < numCon; i++)
    {
      char buf[255];
      sprintf(buf, "%d: %d,%d\n", BLE_CONNECTION_EVENT, 1, i);
      Serial1.printf(buf);
      snprintf(buf, 255, "ble:%d,n:%d,t:%s", 1, i, Time.timeStr().c_str());
      publishQueue.publish("telemetry", buf, PRIVATE);
      digitalWrite(A0, LOW);
    }
  }

  //Handle uart commands
  if (Serial1.available())
  {
    cmdHandler.getChar();
  }

  //Handle BLE data and send over SPI.
  uint8_t packets = 0;
  if ((packets = BleStack.dataAvailable(0)) && (digitalRead(HANDSHAKE)) && (!spiBusy))
  {

    digitalWrite(D6, HIGH); //For debugging

    //Log.info("ready to send 1 of %d over SPI for node A.",packets);

    uint8_t *location = BleStack.getReadPtr(0);
    memcpy(spi_buff, location, BLE_RX_DATA_SIZE);

    spi_buff[244] = 0; //Last byte indicate this data is from node 0.
    //Log.info("x: %d y: %d, z: %d", *((int16_t *)&spi_buff[0]), *((int16_t *)&spi_buff[2]), *((int16_t *)&spi_buff[4]));
    //delay?
    spiBusy = true;
    digitalWrite(CS, LOW);
    SPI.transfer(spi_buff, NULL, 245, spiDoneHandler); //This version of spi.transfer uses dma.
    digitalWrite(D6, LOW);
  }

  //It can take around 20 us for handshake  to go low after spi transaction,
  //so we need delay to wait at least that long otherwise we falsely read the handshake is high and send when the esp is not read.

  delayMicroseconds(100);
}


void spiDoneHandler()
{
  digitalWrite(CS, HIGH);
  spiBusy = false;
}

void handleDeviceLocator(const char *event, const char *data)
{
  cmdHandler.setGPS(data);
}

int setThreshold(String threshString) {
  Serial.println(threshString);
  threshold = threshString.toInt(); 
  char buf[255];
  sprintf(buf, "%d: %d\n", SET_THRESHOLD, threshold);
  Serial1.printf(buf);  
  return 1;
}

int setMode(String modeString) {
  Serial.println(modeString);
  mode = modeString.toInt(); 
  BleStack.sendCommand(&mode,1);
  char buf[255];
  sprintf(buf, "%d: %d\n", SET_MODE, mode);
  Serial1.printf(buf);  
  return 1;
}

int setTxStdY(String txStdYString){
  Serial.println(txStdYString);
  cmdHandler.txStdY = txStdYString.toInt(); 
  particleTxStdY = txStdYString.toInt(); 
  return 1;
}
