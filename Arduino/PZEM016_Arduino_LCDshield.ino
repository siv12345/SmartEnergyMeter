// PZEM-016 <-> RS485 to TTL (4 conductor out) -> Arduino Mega 2560 + LCD Shield
// Safety is very important when dealing with electricity. 
// Code monitors AC Voltage, current, Power, Energy, Frequency, and Power Factor.
// LCD Display is refreshed every second.
// Values are calculated internally by energy meter and function of Arduino is only to read the value and for further calculation. 
// Modbus Master library is required.
// Core of the code was from EvertDekker.com 2018 which based on the example from http://solar4living.com/pzem-arduino-modbus.htm

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/////////////*/


        /* 1- PZEM-014/016 AC Energy Meter */
        
        #include <ModbusMaster.h>                   // Load the (modified) library for modbus communication command codes. Kindly install at our website.
        #define MAX485_DE      2                    // Define DE Pin to Arduino pin. Connect DE Pin of Max485 converter module to Pin 2 (default) Arduino board
        #define MAX485_RE      3                    // Define RE Pin to Arduino pin. Connect RE Pin of Max485 converter module to Pin 3 (default) Arduino board
                                                    // These DE anr RE pins can be any other Digital Pins to be activated during transmission and reception process.
        static uint8_t pzemSlaveAddr = 0x01;        // Declare the address of device (meter) in term of 8 bits. You can change to 0x02 etc if you have more than 1 meter.
        ModbusMaster node;                          /* activate modbus master codes*/  
        float PZEMVoltage =0;                       /* Declare value for AC voltage */
        float PZEMCurrent =0;                       /* Declare value for AC current*/
        float PZEMPower =0;                         /* Declare value for AC Power */
        float PZEMEnergy=0;                         /* Declare value for energy */
        float PZEMHz =0;                            /* Declare value for frequency */
        float PZEMPf=0;                             /* Declare value for power factor */
        unsigned long startMillisPZEM;              /* start counting time for LCD Display */
        unsigned long currentMillisPZEM;            /* current counting time for LCD Display */
        const unsigned long periodPZEM = 1000;      // refresh every X seconds (in seconds) in LED Display. Default 1000 = 1 second 

        /* 2 - LCD Display  */

        #include<LiquidCrystal.h>                   /* Load the liquid Crystal Library (by default already built-it with arduino solftware)*/
        LiquidCrystal LCD(8,9,4,5,6,7);             /* Creating the LiquidCrystal object named LCD. The pin may be varies based on LCD module that you use*/
        unsigned long startMillisLCD;               /* start counting time for LCD Display */
        unsigned long currentMillisLCD;             /* current counting time for LCD Display */
        const unsigned long periodLCD = 1000;       /* refresh every X seconds (in seconds) in LED Display. Default 1000 = 1 second */
        int page = 1;                               /* LCD Display page*/
        int ResetEnergy = 0;                        /* reset energy function */
        unsigned long startMillisEnergy;            /* start counting time for LCD Display */
        unsigned long currentMillisEnergy;          /* current counting time for LCD Display */
        const unsigned long periodEnergy = 1000;    // refresh every X seconds (in seconds) in LED Display. Default 1000 = 1 second 

void setup() 
{
        /* General*/
        
        Serial.begin(9600);                         /* to display readings in Serial Monitor at 9600 baud rates */

        
        /* 1- PZEM-014/016 AC Energy Meter */
        
        startMillisPZEM = millis();                 /* Start counting time for run code */
        Serial3.begin(9600);                        // To assign communication port to communicate with meter. 
                                                    // By default communicate via Serial3 port: pin 14 (Tx) and pin 15 (Rx)
        node.begin(pzemSlaveAddr, Serial3);         /* Define and start the Modbus RTU communication. Communication to specific slave address and which Serial port */
        pinMode(MAX485_RE, OUTPUT);                 /* Define RE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
        pinMode(MAX485_DE, OUTPUT);                 /* Define DE Pin as Signal Output for RS485 converter. Output pin means Arduino command the pin signal to go high or low so that signal is received by the converter*/
        digitalWrite(MAX485_RE, 0);                 /* Arduino create output signal for pin RE as LOW (no output)*/
        digitalWrite(MAX485_DE, 0);                 /* Arduino create output signal for pin DE as LOW (no output)*/
                                                    // both pins no output means the converter is in communication signal receiving mode
        node.preTransmission(preTransmission);      // Callbacks allow us to configure the RS485 transceiver correctly
        node.postTransmission(postTransmission);
        changeAddress(0XF8, 0x01);                  // By delete the double slash symbol, the meter address will be set as 0x01. 
                                                    // By default I allow this code to run every program startup. Will not have effect if you only have 1 meter
        //  resetEnergy(0x01);                      // By delete the double slash symbol, the Energy value in the meter is reset. 
        delay(1000);                                /* after everything done, wait for 1 second */


        /* 2 - LCD Display  */

        LCD.begin(16,2);                            /* Tell Arduino that our LCD has 16 columns and 2 rows*/
        LCD.setCursor(0,0);                         /* Set LCD to start with upper left corner of display*/  
        startMillisLCD = millis();                  /* Start counting time for display refresh time*/

}

void loop() 
{

        /* 0- General */


        /* 0.1- Button Function */
        
        int buttonRead;
        buttonRead = analogRead (0);                                                                      // Read analog pin A0. Pin A0 automatically assigned for LCD Display Button function (cannot be changed)

        /*Right button is pressed */
        if (buttonRead < 60) 
          {   
             LCD.setCursor(0,0); LCD.print (" PRESS <SELECT> "); 
             LCD.setCursor(0,1); LCD.print ("TO RESET ENERGY ");
          }       
     
        /* Up button is pressed */
        else if (buttonRead < 200) 
          {   
            page = 1;
          }    
                 
        /* Down button is pressed */
        else if (buttonRead < 400)
          {   
            page = 2;
          }      
     
        /* Left button is pressed */
        else if (buttonRead < 600)
          {   
            LCD.setCursor(0,0); LCD.print (" PRESS <SELECT> "); 
            LCD.setCursor(0,1); LCD.print ("TO RESET ENERGY ");
            if(ResetEnergy ==1)                                                                           /* only to run reset energy if left button is pressed after select button*/
            {
            page = 3;
            }
          } 
     
        /* Select button is pressed */
        else if (buttonRead < 800)
          {   
              ResetEnergy = 1;                                                                            // to activate offset for power
              LCD.setCursor(0,0);                                                                         /* set display words starting at upper left corner*/
              LCD.print ("RESET ENERGY ?  ");
              LCD.setCursor(0,1);                                                                         /* set display words starting at lower left corner*/
              LCD.print ("PRESS < LEFT >  ");
              startMillisEnergy = millis();
          }
          
          /* After Select button is pressed */
          
          if(ResetEnergy ==1)
          {
            LCD.setCursor(0,0);                                                                           /* set display words starting at upper left corner*/
            LCD.print ("RESET ENERGY ?  ");
            LCD.setCursor(0,1);                                                                           /* set display words starting at lower left corner*/
            LCD.print ("PRESS < LEFT >  ");
            currentMillisEnergy = millis();
            if(( currentMillisEnergy - startMillisEnergy <= 5000) && (page==3))                           /* if within 5 seconds <left> button is pressed, do reset energy*/   
              {
                resetEnergy(0x01);
                page=1;                                                                                   /* go back to page 1 after reset*/
              }
            if(( currentMillisEnergy - startMillisEnergy > 5000) && (page!=3))                            /* if more than 5 seconds <Left> button does not pressed, go back to main page*/     
              {
                ResetEnergy=0;
              }
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

              Serial.print(PZEMVoltage, 1);                                                               /* Print Voltage value on Serial Monitor with 1 decimal*/
              Serial.print("V   ");
              Serial.print(PZEMHz, 1);
              Serial.print("Hz   ");
              Serial.print(PZEMCurrent, 3);
              Serial.print("A   ");
              Serial.print(PZEMPower, 1);
              Serial.print("W  ");
              Serial.print(PZEMPf, 2);
              Serial.print("pf   ");
              Serial.print(PZEMEnergy, 0);
              Serial.print("Wh  ");
              Serial.println();
              
              if (pzemSlaveAddr==2)                                                                       /* just for checking purpose to see whether can read modbus*/
                {
                  Serial.println(); 
                }
            } 
              else
                {
                  Serial.println("Failed to read modbus");
                }
              startMillisPZEM = currentMillisPZEM ;                                                       /* Set the starting point again for next counting time */
        }



        /* 2 - LCD Display  */
        
        currentMillisLCD = millis();                                                                      /* Set counting time for LCD Display*/
        if (currentMillisLCD - startMillisLCD >= periodLCD)                                               /* for every x seconds, run the codes below*/  
          {
            if( page ==1)
            {
            LCD.setCursor(0,0);                                                                           /* Set cursor to first colum 0 and second row 1  */
            LCD.print(PZEMVoltage, 1);                                                                    /* Display Voltage on LCD Display with 1 decimal*/
            LCD.print("V    ");
            LCD.setCursor(9,0);
            LCD.print(PZEMCurrent, 2);  
            LCD.print("A    "); 
            LCD.setCursor(0,1);
            LCD.print(PZEMPower, 1);
            LCD.print("W     ");
            LCD.setCursor(9,1);
            LCD.print(PZEMEnergy, 0);
            LCD.print("Wh    ");
            startMillisLCD = currentMillisLCD ;                                                           /* Set the starting point again for next counting time */
            }

            if( page ==2)
            {
            LCD.setCursor(0,0);                                                                           /* Set cursor to first colum 0 and second row 1  */
            LCD.print(PZEMPower, 1);
            LCD.print("W     ");
            LCD.setCursor(9,0);
            LCD.print(PZEMEnergy, 0);
            LCD.print("Wh    ");
            LCD.setCursor(0,1);
            LCD.print(PZEMPf, 2);  
            LCD.print("PF    ");
            LCD.setCursor(9,1); 
            LCD.print(PZEMHz, 1);  
            LCD.print("Hz   ");
            startMillisLCD = currentMillisLCD ;  
            }
          }
          
}

void preTransmission()                                                                                    /* transmission program when triggered*/
{
  
        /* 1- PZEM-014/016 AC Energy Meter */
        
        digitalWrite(MAX485_RE, 1);                                                                       /* put RE Pin to high*/
        digitalWrite(MAX485_DE, 1);                                                                       /* put DE Pin to high*/
        delay(1);                                                                                         // When both RE and DE Pin are high, converter is allow to transmit communication
}

void postTransmission()                                                                                   /* Reception program when triggered*/
{
        
        /* 1- PZEM-014/016 AC Energy Meter */
        
        delay(3);                                                                                         // When both RE and DE Pin are low, converter is allow to receive communication
        digitalWrite(MAX485_RE, 0);                                                                       /* put RE Pin to low*/
        digitalWrite(MAX485_DE, 0);                                                                       /* put DE Pin to low*/
}

void resetEnergy(uint8_t slaveAddr)                                                                       //Reset the slave's energy counter
{
        
        /* 1- PZEM-014/016 AC Energy Meter */
  
        uint16_t u16CRC = 0xFFFF;                                                                         /* declare CRC check 16 bits*/
        static uint8_t resetCommand = 0x42;                                                               /* reset command code*/
        u16CRC = crc16_update(u16CRC, slaveAddr);
        u16CRC = crc16_update(u16CRC, resetCommand);
        Serial.println("Resetting Energy");
        preTransmission();                                                                                /* trigger transmission mode*/
       
        Serial3.write(slaveAddr);                                                                         /* send device address in 8 bit*/
        Serial3.write(resetCommand);                                                                      /* send reset command */
        Serial3.write(lowByte(u16CRC));                                                                   /* send CRC check code low byte  (1st part) */
        Serial3.write(highByte(u16CRC));                                                                  /* send CRC check code high byte (2nd part) */
      
        delay(10);
        postTransmission();                                                                               /* trigger reception mode*/
        delay(100);
        
        while (Serial3.available())                                                                       /* while receiving signal from Serial3 from meter and converter */ 
          {                                                                      
            Serial.print(char(Serial3.read()), HEX);                                                      /* Prints the response and display on Serial Monitor (Serial)*/
            Serial.print(" ");
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
      
        Serial.println("Change Slave Address");
        preTransmission();                                                                                 /* trigger transmission mode*/
      
        Serial3.write(OldslaveAddr);                                                                       /* these whole process code sequence refer to manual*/
        Serial3.write(SlaveParameter);
        Serial3.write(highByte(registerAddress));
        Serial3.write(lowByte(registerAddress));
        Serial3.write(highByte(NewslaveAddr));
        Serial3.write(lowByte(NewslaveAddr));
        Serial3.write(lowByte(u16CRC));
        Serial3.write(highByte(u16CRC));
        delay(10);
        postTransmission();                                                                                /* trigger reception mode*/
        delay(100);
        while (Serial3.available())                                                                        /* while receiving signal from Serial3 from meter and converter */
          {   
            Serial.print(char(Serial3.read()), HEX);                                                       /* Prints the response and display on Serial Monitor (Serial)*/
            Serial.print(" ");
          }
}
