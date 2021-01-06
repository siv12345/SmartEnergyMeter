

// PZEM-014/016 AC Energy Meter Online Monitoring using Blynk App By Solarduino 

// Note Summary
// Note :  Safety is very important when dealing with electricity. We take no responsibilities while you do it at your own risk.
// Note :  This AC Energy Monitoring Code needs PZEM-014 or PZEM-016 AC Energy Meter to measure values and Node MCU for communication to Blynk Server.
// Note :  This Code monitors AC Voltage, current, Power, Energy, Frequency, and Power Factor.
// Note :  The values are calculated internally by energy meter and function of Node MCU is only to read the value and for further submission to Blynk Server for display. 
// Note :  You need to download and install (modified) Modbus Master library at our website (https://solarduino.com/pzem-014-or-016-ac-energy-meter-with-arduino/ )
// Note :  You need to download the Blynk App on your smart phone for display.
// Note :  The Core of the code was from EvertDekker.com 2018 which based on the example from http://solar4living.com/pzem-arduino-modbus.htm
// Note :  Solarduino only amend necessary code and integrate with Blynk Server.
// Note :  Blog page for this code : https://solarduino.com/pzem-016-ac-energy-meter-online-monitoring-with-blynk-app/ 

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/////////////*/

        
        /* Virtual Serial Port */

        //#include <SoftwareSerial.h>                           /* include virtual Serial Port coding */
        HardwareSerial PZEMSerial(2);                            // Move the PZEM DC Energy Meter communication pins from Rx to pin D1 = GPIO 5 & TX to pin D2 = GPIO 4

        
        /* 0- Blynk Server and Wifi Connection */

        #include <WiFi.h>                              // Enable the use of wifi module. Make sure you downloaded and installed the ESP8266 library
        #include <BlynkSimpleEsp32.h>                       /* Not sure what is this for */

        char auth[] = "0vKTZm16d0ic_wjoCRVuOyoejx1P1oIk";     // Put in the Auth Token for the project from Blynk. You should receive it in your email.
        char ssid[] = "133MahalayaMax";                  // Key in your wifi name. You can check with your smart phone for your wifi name
        char pass[] = "F14B3CDE48";                             // Key in your wifi password.


        /* 1- PZEM-014/016 AC Energy Meter */
        
        #include <ModbusMaster.h>                             // Load the (modified) library for modbus communication command codes. Kindly install at our website.
        #define MAX485_DE      19                             // Define DE Pin to Arduino pin. Connect DE Pin of Max485 converter module to Pin D0 (GPIO 16) Node MCU board
        #define MAX485_RE      18                             // Define RE Pin to Arduino pin. Connect RE Pin of Max485 converter module to Pin D1 (GPIO 5) Node MCU board
                                                              // These DE anr RE pins can be any other Digital Pins to be activated during transmission and reception process.
        static uint8_t pzemSlaveAddr = 0x01;                  // Declare the address of device (meter) in term of 8 bits. You can change to 0x02 etc if you have more than 1 energy meter.
        ModbusMaster node;                                    /* activate modbus master codes*/  
        float PZEMVoltage =0;                                 /* Declare value for AC voltage */
        float PZEMCurrent =0;                                 /* Declare value for AC current*/
        float PZEMPower =0;                                   /* Declare value for AC Power */
        float PZEMEnergy=0;                                   /* Declare value for energy */
        float PZEMHz =0;                                      /* Declare value for frequency */
        float PZEMPf=0;                                       /* Declare value for power factor */
        unsigned long startMillisPZEM;                        /* start counting time for data collection */
        unsigned long currentMillisPZEM;                      /* current counting time for data collection */
        const unsigned long periodPZEM = 1000;                // refresh data collection every X seconds (in seconds). Default 1000 = 1 second 

        /* 2 - Data submission to Blynk Server  */

        unsigned long startMillisReadData;                    /* start counting time for data collection*/
        unsigned long currentMillisReadData;                  /* current counting time for data collection*/
        const unsigned long periodReadData = 1000;            /* refresh every X seconds (in seconds) in LED Display. Default 1000 = 1 second */
        int ResetEnergy = 0;                                  /* reset energy function */
        int a = 1;
        unsigned long startMillis1;                           // to count time during initial start up (PZEM Software got some error so need to have initial pending time)

void setup() 
{
        /* General*/
        
        startMillis1 = millis();
        
        Serial.begin(9600);                                   /* to display readings in Serial Monitor at 9600 baud rates */
        PZEMSerial.begin(9600,SERIAL_8N1,16,17);              // 4 = Rx/R0/ GPIO 4 (D2) & 0 = Tx/DI/ GPIO 0 (D3) on NodeMCU 
        Blynk.begin(auth, ssid, pass);                        /* You can also specify server: Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80); Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);*/

        /* 1- PZEM-014/016 AC Energy Meter */
        
        startMillisPZEM = millis();                           /* Start counting time for run code */            
        pinMode(MAX485_RE, OUTPUT);                           /* Define RE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
        pinMode(MAX485_DE, OUTPUT);                           /* Define DE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
        digitalWrite(MAX485_RE, 0);                           /* Arduino create output signal for pin RE as LOW (no output)*/
        digitalWrite(MAX485_DE, 0);                           /* Arduino create output signal for pin DE as LOW (no output)*/
                                                              /* both pins no output means the converter is in communication signal receiving mode */
        node.preTransmission(preTransmission);                /* Callbacks allow us to configure the RS485 transceiver correctly*/
        node.postTransmission(postTransmission);
        node.begin(pzemSlaveAddr,PZEMSerial);                 /* Define and start the Modbus RTU communication. Communication to specific slave address and which Serial port */
        delay(1000);                                          /* after everything done, wait for 1 second */

        /* 2 - Data submission to Blynk Server */

        startMillisReadData = millis();                       /* Start counting time for data submission to Blynk Server*/


}

void loop() 
{
       
        /* 0- General */
        
        Blynk.run();                                          /* allow the communication between Blynk server and Node MCU */
        if ((millis()- startMillis1 >= 10000) && (a ==1))
        {   
          changeAddress(0XF8, pzemSlaveAddr);                 // By delete the double slash symbol, the meter address will be set as 0x01. 
                                                              // By default I allow this code to run every program startup. Will not have effect if you only have 1 meter
          a = 0;
        }
        
        /* 1- PZEM-014/016 AC Energy Meter */

        currentMillisPZEM = millis();                                                                     /* count time for program run every second (by default)*/
        if (currentMillisPZEM - startMillisPZEM >= periodPZEM)                                            /* for every x seconds, run the codes below*/
        {    
          uint8_t result;                                                                                 /* Declare variable "result" as 8 bits */   
          result = node.readInputRegisters(0x0000, 9);                                                    /* read the 9 registers (information) of the PZEM-014 / 016 starting 0x0000 (voltage information) kindly refer to manual)*/
          if (result == node.ku8MBSuccess)                                                                /* If there is a response */
            {
              uint32_t tempdouble = 0x00000000;                                                           /* Declare variable "tempdouble" as 32 bits with initial value is 0 */ 
              PZEMVoltage = node.getResponseBuffer(0x0000) / 10.0;                                        /* get the 16bit value for the voltage value, divide it by 10 (as per manual) */
                                                                                                          // 0x0000 to 0x0008 are the register address of the measurement value
              tempdouble =  (node.getResponseBuffer(0x0002) << 16) + node.getResponseBuffer(0x0001);      /* get the currnet value. Current value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
              PZEMCurrent = tempdouble / 1000.00;                                                         /* Divide the value by 1000 to get actual current value (as per manual) */
              
              tempdouble =  (node.getResponseBuffer(0x0004) << 16) + node.getResponseBuffer(0x0003);      /* get the power value. Power value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
              PZEMPower = tempdouble / 10.0;                                                              /* Divide the value by 10 to get actual power value (as per manual) */
              
              tempdouble =  (node.getResponseBuffer(0x0006) << 16) + node.getResponseBuffer(0x0005);      /* get the energy value. Energy value is consists of 2 parts (2 digits of 16 bits in front and 2 digits of 16 bits at the back) and combine them to an unsigned 32bit */
              PZEMEnergy = tempdouble;                                                                    
  
              PZEMHz = node.getResponseBuffer(0x0007) / 10.0;                                             /* get the 16bit value for the frequency value, divide it by 10 (as per manual) */
              PZEMPf = node.getResponseBuffer(0x0008) / 100.00;                                           /* get the 16bit value for the power factor value, divide it by 100 (as per manual) */
              
              if (pzemSlaveAddr==2)                                                                       /* just for checking purpose to see whether can read modbus*/
                {
                }
            } 
              else
                {
                }
              startMillisPZEM = currentMillisPZEM ;                                                       /* Set the starting point again for next counting time */
        }


        /* 2 - Data submission to Blynk Server  */
        
        currentMillisReadData = millis();                                                                 /* Set counting time for data submission to server*/
        if (currentMillisReadData - startMillisReadData >= periodReadData)                                /* for every x seconds, run the codes below*/  
          {
            Serial.print("Vac : "); Serial.print(PZEMVoltage); Serial.println(" V ");
            Serial.print("Iac : "); Serial.print(PZEMCurrent); Serial.println(" A ");
            Serial.print("Power : "); Serial.print(PZEMPower); Serial.println(" W ");
            Serial.print("Energy : "); Serial.print(PZEMEnergy); Serial.println(" Wh ");
            Serial.print("Power Factor : "); Serial.print(PZEMPf); Serial.println(" pF ");
            Serial.print("Frequency : "); Serial.print(PZEMHz); Serial.println(" Hz ");
            Blynk.virtualWrite(V0,PZEMVoltage);                                                           // Send data to Blynk Server. Voltage value as virtual pin V0
            Blynk.virtualWrite(V1,PZEMCurrent);
            Blynk.virtualWrite(V2,PZEMPower);
            Blynk.virtualWrite(V3,PZEMEnergy);
            Blynk.virtualWrite(V4,PZEMPf);
            Blynk.virtualWrite(V5,PZEMHz);  
            startMillisReadData = millis();                                                /* Set the starting point again for next counting time */
          }
          
}

void preTransmission()                                                                                    /* transmission program when triggered*/
{
  
        /* 1- PZEM-014/016 AC Energy Meter */
        if(millis() - startMillis1 > 5000)                                                                // Wait for 5 seconds as ESP Serial cause start up code crash
        {
          digitalWrite(MAX485_RE, 1);                                                                     /* put RE Pin to high*/
          digitalWrite(MAX485_DE, 1);                                                                     /* put DE Pin to high*/
          delay(1);                                                                                       // When both RE and DE Pin are high, converter is allow to transmit communication
        }
}

void postTransmission()                                                                                   /* Reception program when triggered*/
{
        
        /* 1- PZEM-014/016 AC Energy Meter */
        if(millis() - startMillis1 > 5000)                                                                // Wait for 5 seconds as ESP Serial cause start up code crash
        {
          delay(3);                                                                                       // When both RE and DE Pin are low, converter is allow to receive communication
          digitalWrite(MAX485_RE, 0);                                                                     /* put RE Pin to low*/
          digitalWrite(MAX485_DE, 0);                                                                     /* put DE Pin to low*/
        }
}


BLYNK_WRITE(V6)                                       // Virtual push button is defined as V7 of Blynk App. When the button is pushed, it will activate the codes  
        {
          if(param.asInt()==1)
          { 
            uint16_t u16CRC = 0xFFFF;                         /* declare CRC check 16 bits*/
            static uint8_t resetCommand = 0x42;               /* reset command code*/
            uint8_t slaveAddr =pzemSlaveAddr;
            u16CRC = crc16_update(u16CRC, slaveAddr);
            u16CRC = crc16_update(u16CRC, resetCommand);
            preTransmission();                                /* trigger transmission mode*/                
            PZEMSerial.write(slaveAddr);                          /* send device address in 8 bit*/
            PZEMSerial.write(resetCommand);                       /* send reset command */
            PZEMSerial.write(lowByte(u16CRC));                    /* send CRC check code low byte  (1st part) */
            PZEMSerial.write(highByte(u16CRC));                   /* send CRC check code high byte (2nd part) */ 
            delay(10);
            postTransmission();                               /* trigger reception mode*/
            delay(100);
          }
        }


void changeAddress(uint8_t OldslaveAddr, uint8_t NewslaveAddr)                                            //Change the slave address of a node
{

        /* 1- PZEM-014/016 AC Energy Meter */
        
        static uint8_t SlaveParameter = 0x06;                                                             /* Write command code to PZEM */
        static uint16_t registerAddress = 0x0002;                                                         /* Modbus RTU device address command code */
        uint16_t u16CRC = 0xFFFF;                                                                         /* declare CRC check 16 bits*/
        u16CRC = crc16_update(u16CRC, OldslaveAddr);                                                      // Calculate the crc16 over the 6bytes to be send
        u16CRC = crc16_update(u16CRC, SlaveParameter);
        u16CRC = crc16_update(u16CRC, highByte(registerAddress));
        u16CRC = crc16_update(u16CRC, lowByte(registerAddress));
        u16CRC = crc16_update(u16CRC, highByte(NewslaveAddr));
        u16CRC = crc16_update(u16CRC, lowByte(NewslaveAddr));

        preTransmission();                                                                                /* trigger transmission mode*/
      
        PZEMSerial.write(OldslaveAddr);                                                                       /* these whole process code sequence refer to manual*/
        PZEMSerial.write(SlaveParameter);
        PZEMSerial.write(highByte(registerAddress));
        PZEMSerial.write(lowByte(registerAddress));
        PZEMSerial.write(highByte(NewslaveAddr));
        PZEMSerial.write(lowByte(NewslaveAddr));
        PZEMSerial.write(lowByte(u16CRC));
        PZEMSerial.write(highByte(u16CRC));
        delay(10);
        postTransmission();                                                                               /* trigger reception mode*/
        delay(100);
}
