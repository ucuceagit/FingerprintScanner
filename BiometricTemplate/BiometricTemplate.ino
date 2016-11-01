#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
SoftwareSerial debug(2,3);

#define FPS_RESPONSE_SIZE     (48)
#define FPS_PACKET_SIZE        (24)
#define COMMAND_PACKET        (0xAA55)
#define RESPONSE_PACKET       (0x55AA)
#define COMMAND_DATA_PACKET   (0xA55A)
#define RESPONSE_DATA_PACKET  (0x5AA5)
 
#define CMD_IDENTIFY      (0x0102)
#define CMD_ENROLL_ONE    (0x0104)
#define CMD_CLEAR_ADDR    (0x0105)
#define CMD_CLEAR_ALL     (0x0106)
#define CMD_ENROLL_COUNT  (0x0128)
#define CMD_CANCEL        (0x0130)
#define CMD_LED_CONTROL   (0x0124)

uint8_t FPS_RESPONSE_PACKET[FPS_RESPONSE_SIZE];
uint8_t FPSPacket[FPS_PACKET_SIZE] = {0};   
boolean replied = false;


void setup() {
 Serial.begin(115200);
 debug.begin(9600);
 lcd.begin(20,4);
 debug.println("Debug Mode On!");
 pinMode(8,INPUT);
 pinMode(7,OUTPUT);
 lcd.clear();
 lcd.backlight();

          
 FPSPacket[0] = 1; 
 sendCommand(COMMAND_PACKET, CMD_LED_CONTROL, 2, FPSPacket);
 delay(1000);
 FPSPacket[0] = 0; 
 sendCommand(COMMAND_PACKET, CMD_LED_CONTROL, 2, FPSPacket);

 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("Fingerprint Test");
 lcd.setCursor(0,1);
 lcd.print("Waiting....         ");
 
}

void loop() 
{
   if(!digitalRead(8)){
      delay(300);
      lcd.setCursor(0,1);
      lcd.print("Lagay mo Daliri mo     ");
      delay(1000);

      sendCommand(COMMAND_PACKET,  CMD_IDENTIFY , 0, FPSPacket);
      getReply();
     
      if((FPS_RESPONSE_PACKET[24] != 0xAA) && (FPS_RESPONSE_PACKET[25] != 0x55)){
         lcd.setCursor(0,1);
         lcd.print("Ulit po!           ");
      }else if((FPS_RESPONSE_PACKET[30] == 1)&& FPS_RESPONSE_PACKET[32] == 0x12){
          lcd.setCursor(0,1);
          lcd.print("Who you po?           ");
          delay(2000);
          lcd.setCursor(0,1);
          lcd.print("Waiting....         ");
      }else{
          lcd.setCursor(0,1);
          lcd.print("Hello user ");
          lcd.print(FPS_RESPONSE_PACKET[32]);
          lcd.print("              ");
          digitalWrite(7,HIGH);
          delay(2000);
          digitalWrite(7,LOW);
          lcd.setCursor(0,1);
          lcd.print("Waiting....         ");
      }
   }
}

void getReply()
{
 uint8_t rx;
 uint8_t i=0;
 
 replied = false;
 memset(FPS_RESPONSE_PACKET,0,48);
 
 while(replied == false)
 {
  
   if(Serial.available() > 0){
     rx = Serial.read();
     if(rx == 0xAA){
        for(i=1; i<48; i++){
          FPS_RESPONSE_PACKET[i] = Serial.read();
        }
         FPS_RESPONSE_PACKET[0] = 0xAA;
         replied = true;

     }    
   }
  }

            for(i=0; i<48; i++){
           debug.print(FPS_RESPONSE_PACKET[i],HEX);
           debug.print(" ");
         }
         debug.println();
}

void sendCommand(uint16_t Prefix, uint16_t Command, uint16_t DataSize, uint8_t Data[])
{  
  uint8_t DataBuffer[FPS_PACKET_SIZE] = {0};
  uint8_t i = 0;
 
  DataBuffer[0] = (uint8_t)(Prefix & 0xFF);
  DataBuffer[1] = (uint8_t)((Prefix>>8) & 0xFF);
  DataBuffer[2] = (uint8_t)(Command & 0xFF);
  DataBuffer[3] = (uint8_t)((Command>>8) & 0xFF);
  DataBuffer[4] = (uint8_t)(DataSize & 0xFF);
  DataBuffer[5] = (uint8_t)((DataSize>>8) & 0xFF);
 
  for(i = 6; i <= 21; i++){
    DataBuffer[i] += Data[i-6];
  }
 
  //Compute Checksum
  uint16_t Checksum = 0;
  for(i = 0; i <= 21; i++){
    Checksum += DataBuffer[i];
  }
 
  DataBuffer[22] = (uint8_t)(Checksum & 0xFF);
  DataBuffer[23] = (uint8_t)((Checksum>>8) & 0xFF);
 
  //Clear the buffer first
Serial.flush();
 
  //Send
 Serial.write(DataBuffer, sizeof(DataBuffer));
  
  delay(1500); 
}

