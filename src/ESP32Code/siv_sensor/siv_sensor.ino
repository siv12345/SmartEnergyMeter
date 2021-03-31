#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include "InfluxDbClient.h"
#include "InfluxDbCloud.h"

// WiFi AP SSID
#define WIFI_SSID "WIFI_SSID_SIV"
// WiFi password
#define WIFI_PASSWORD "WIFI_PWD_SIV"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://us-central1-1.gcp.cloud2.influxdata.com"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "TOK_SME_SIV"
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG "ORG_ID"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "sme-siva"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("Phase1");

/*
  An Arduino Sketch for reading data from a PZEM-014 or PZEM-016, tested with ESP32 DEVKit 1, Arduino 1.8.5
  EvertDekker.com 2018, based on the example from http://solar4living.com/pzem-arduino-modbus.htm
  If you want to use slaveid function to change the slaveid on the fly, you need to modify the ModbusMaster library (Or get the copy from my website)
  In ModbusMaster.h add at line 78
    void slaveid(uint8_t);
  In ModbusMaster.cpp add at line 75
    void ModbusMaster::slaveid(uint8_t slave)
     {
      _u8MBSlave = slave;
     }
*/
/* If you are using other then uart0 on the ESP32, Comment out in esp32-hal-uart.c the follwing line:
  //uart->dev->conf0.txfifo_rst = 1;
  //uart->dev->conf0.txfifo_rst = 0;
  //uart->dev->conf0.rxfifo_rst = 1;
  //uart->dev->conf0.rxfifo_rst = 0;
  Source: https://github.com/4-20ma/ModbusMaster/issues/93
*/

#include <ModbusMaster.h>

HardwareSerial Pzemserial(2);

#define RXD2 16 //Gpio pins Serial2
#define TXD2 17

#define MAX485_DE      19  // We're using a MAX485-compatible RS485 Transceiver. The Data Enable and Receiver Enable pins are hooked up as follows:
#define MAX485_RE_NEG  18

ModbusMaster node;
static uint8_t pzemSlaveAddr = 0x01;

void setup() {
  client.setInsecure(true);
  client.setInsecure(true);
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Add tags
  //sensor.addTag("device", DEVICE);
  //sensor.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }


  Pzemserial.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  node.begin(pzemSlaveAddr, Pzemserial);  //Start the Modbusmaster

  pinMode(MAX485_RE_NEG, OUTPUT);  // Setting up the RS485 transceivers
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);  // Init in receive mode
  digitalWrite(MAX485_DE, 0);

  node.preTransmission(preTransmission);  // Callbacks allow us to configure the RS485 transceiver correctly
  node.postTransmission(postTransmission);

  //changeAddress(0x01, 0x02);
  //changeAddress(0xF8, 0x01);
  /* By Uncomment the function in the above line you can change the slave address from one of the nodes, only need to be done ones. Preverable do this only with 1 slave in the network.
     changeAddress(OldAddress, Newaddress)
     If you f*ck it up or don't know the new address anymore, you can use the broadcast address 0XF8 as OldAddress to change the slave address. Use this with one slave ONLY in the network.
  */

  //resetEnergy(0x01);
  /* By Uncomment the function in the above line you can reset the energy counter (Wh) back to zero from one of the slaves.
  */

  delay(1000);
}

/*
  RegAddr Description                 Resolution
  0x0000  Voltage value               1LSB correspond to 0.1V
  0x0001  Current value low 16 bits   1LSB correspond to 0.001A
  0x0002  Current value high 16 bits
  0x0003  Power value low 16 bits     1LSB correspond to 0.1W
  0x0004  Power value high 16 bits
  0x0005  Energy value low 16 bits    1LSB correspond to 1Wh
  0x0006  Energy value high 16 bits
  0x0007  Frequency value             1LSB correspond to 0.1Hz
  0x0008  Power factor value          1LSB correspond to 0.01
  0x0009  Alarm status  0xFFFF is alarm，0x0000is not alarm
*/

void loop() {
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();

  uint8_t result;

  for (pzemSlaveAddr = 1; pzemSlaveAddr < 2; pzemSlaveAddr++) {  // Loop all the Pzem sensors
    node.slaveid(pzemSlaveAddr);          //Switch to another slave address. NOTE: You can only use this function is you have modified the ModbusMaster library (Or get the copy from my website)
    Serial.print("Pzem Slave ");
    Serial.print(pzemSlaveAddr);
    Serial.print(": ");

    result = node.readInputRegisters(0x0000, 9); //read the 9 registers of the PZEM-014 / 016
    if (result == node.ku8MBSuccess)
    {
      uint32_t tempdouble = 0x00000000;
      float voltage = node.getResponseBuffer(0x0000) / 10.0;  //get the 16bit value for the voltage, divide it by 10 and cast in the float variable
      sensor.addField("voltage", voltage);

      tempdouble =  (node.getResponseBuffer(0x0002) << 16) + node.getResponseBuffer(0x0001);  // Get the 2 16bits registers and combine them to an unsigned 32bit
      float current = tempdouble / 1000.00;   // Divide the unsigned 32bit by 1000 and put in the current float variable
      sensor.addField("current", current);

      tempdouble =  (node.getResponseBuffer(0x0004) << 16) + node.getResponseBuffer(0x0003);
      float power = tempdouble / 10.0;
      sensor.addField("power", power);

      tempdouble =  (node.getResponseBuffer(0x0006) << 16) + node.getResponseBuffer(0x0005);
      float energy = tempdouble;
      sensor.addField("energy", energy);

      float hz = node.getResponseBuffer(0x0007) / 10.0;
      sensor.addField("frequency", hz);
      float pf = node.getResponseBuffer(0x0008) / 100.00;
      sensor.addField("pf", pf);

      // Print what are we exactly writing
      Serial.print("Writing: \n");
      Serial.println(sensor.toLineProtocol());

      // If no Wifi signal, try to reconnect it
      if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
        Serial.println("Wifi connection lost");
      }

      // Write point
      if (!client.writePoint(sensor)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
      }


      Serial.print(voltage, 1);  // Print Voltage with 1 decimal
      Serial.print("V   ");

      Serial.print(hz, 1);
      Serial.print("Hz   ");

      Serial.print(current, 3);
      Serial.print("A   ");

      Serial.print(power, 1);
      Serial.print("W  ");

      Serial.print(pf, 2);
      Serial.print("pf   ");

      Serial.print(energy, 0);
      Serial.print("Wh  ");
      Serial.println();
      if (pzemSlaveAddr==2){Serial.println(); }
    } else
    {
      Serial.println("Failed to read modbus");
    }
    delay(1000);
  }

}

void preTransmission()  // Put RS485 Transceiver in transmit mode
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
  delay(1);
}

void postTransmission()  // Put RS485 Transceiver back in receive mode (default mode)
{
  delay(3);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void resetEnergy(uint8_t slaveAddr)    //Reset the slave's energy counter
{
  uint16_t u16CRC = 0xFFFF;
  static uint8_t resetCommand = 0x42;
  u16CRC = crc16_update(u16CRC, slaveAddr);
  u16CRC = crc16_update(u16CRC, resetCommand);
  Serial.println("Resetting Energy");
  preTransmission();
  Pzemserial.write(slaveAddr);
  Pzemserial.write(resetCommand);
  Pzemserial.write(lowByte(u16CRC));
  Pzemserial.write(highByte(u16CRC));
  delay(10);
  postTransmission();
  delay(100);
  while (Pzemserial.available()) {         // Prints the response from the Pzem, do something with it if you like
    Serial.print(char(Pzemserial.read()), HEX);
    Serial.print(" ");
  }
}

void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)  //Change the slave address of a node
{
  static uint8_t SlaveParameter = 0x06;
  static uint16_t registerAddress = 0x0002; // Register address to be changed
  uint16_t u16CRC = 0xFFFF;
  u16CRC = crc16_update(u16CRC, OldslaveAddr);  // Calculate the crc16 over the 6bytes to be send
  u16CRC = crc16_update(u16CRC, SlaveParameter);
  u16CRC = crc16_update(u16CRC, highByte(registerAddress));
  u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
  u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
  u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));

  Serial.println("Change Slave Address");
  preTransmission();
  Pzemserial.write(OldslaveAddr);
  Pzemserial.write(SlaveParameter);
  Pzemserial.write(highByte(registerAddress));
  Pzemserial.write(lowByte(registerAddress));
  Pzemserial.write(highByte(NewslaveAddr));
  Pzemserial.write(lowByte(NewslaveAddr));
  Pzemserial.write(lowByte(u16CRC));
  Pzemserial.write(highByte(u16CRC));
  delay(10);
  postTransmission();
  delay(100);
  while (Pzemserial.available()) {   // Prints the response from the Pzem, do something with it if you like
    Serial.print(char(Pzemserial.read()), HEX);
    Serial.print(" ");
  }
}