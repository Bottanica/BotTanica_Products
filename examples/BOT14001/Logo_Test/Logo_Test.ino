#include <Wire.h>
#define OLED_ADDR 0x3C

void setup() {
 Serial.begin(115200);
 Serial.println("Starting");
 
 Wire.begin();
 delay(100);
 Serial.println("Wire begun");

 Wire.beginTransmission(OLED_ADDR);
 byte error = Wire.endTransmission();
 Serial.print("I2C Check Result: ");
 Serial.println(error);

 if(error == 0) {
   Serial.println("OLED found, sending init commands...");
   
   writeCommand(0xAE); Serial.println("Display off"); delay(10);
   writeCommand(0x00); Serial.println("Set low col addr"); delay(10);
   writeCommand(0x10); Serial.println("Set high col addr"); delay(10);
   writeCommand(0x40); Serial.println("Set display start line"); delay(10);
   writeCommand(0xB0); Serial.println("Set page addr"); delay(10);
   writeCommand(0x81); Serial.println("Contract control"); delay(10);
   writeCommand(0x80); Serial.println("Set contrast 128"); delay(10);
   writeCommand(0x82); Serial.println("IREF Internal"); delay(10);
   writeCommand(0x00); Serial.println("Set value 0"); delay(10);
   writeCommand(0x23); Serial.println("Set value 23h"); delay(10);
   writeCommand(0x01); Serial.println("Set value 1"); delay(10);
   writeCommand(0xA0); Serial.println("Set segment remap"); delay(10);
   writeCommand(0xA2); Serial.println("Set bias 1/9"); delay(10);
   writeCommand(0xA4); Serial.println("Display all points normal"); delay(10);
   writeCommand(0xA6); Serial.println("Normal display"); delay(10);
   writeCommand(0xA8); Serial.println("Set multiplex ratio"); delay(10);
   writeCommand(0x2F); Serial.println("1/16 duty"); delay(10);
   writeCommand(0xC0); Serial.println("Set COM dir"); delay(10);
   writeCommand(0xD3); Serial.println("Set display offset"); delay(10);
   writeCommand(0x38); Serial.println("Set offset value"); delay(10);
   writeCommand(0xD5); Serial.println("Set osc div"); delay(10);
   writeCommand(0x50); Serial.println("Set div value"); delay(10);
   writeCommand(0xD9); Serial.println("Set precharge"); delay(10);
   writeCommand(0x22); Serial.println("Set precharge val"); delay(10);
   writeCommand(0xDB); Serial.println("Set vcomh"); delay(10);
   writeCommand(0x35); Serial.println("Set vcom value"); delay(10);
   writeCommand(0xAD); Serial.println("Set charge pump"); delay(10);
   writeCommand(0x8B); Serial.println("Enable charge pump"); delay(10);
   writeCommand(0x33); Serial.println("Set pump to 9V"); delay(10);
   writeCommand(0xAF); Serial.println("Display on"); delay(10);
   
   Serial.println("Init complete");
 }
}

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

void loop() {
  // Clear screen
  for(int page=0; page<6; page++) {
    writeCommand(0xB0 | page);
    writeCommand(0x00);
    writeCommand(0x10);
    for(int col=0; col<88; col++) {
      writeData(0x00);
    }
  }

  // Calculate center position
  int16_t x = (88 - LOGO_WIDTH) / 2;
  int16_t y = (48 - LOGO_HEIGHT) / 2;
  
  // Draw bitmap in columns, but reverse byte order
  for(int16_t i=0; i<LOGO_WIDTH; i++) {
    for(int16_t j=0; j<(LOGO_HEIGHT+7)/8; j++) {
      // Read byte but flip it vertically
      uint8_t byteCol = pgm_read_byte(&logo_bmp[i*2 + ((LOGO_HEIGHT/8)-1-j)]);
      
      // Position this 8-pixel column
      writeCommand(0xB0 | ((y/8) + j));
      writeCommand(0x00 | ((x+i) & 0x0F));
      writeCommand(0x10 | ((x+i) >> 4));
      
      writeData(byteCol);
    }
  }
  
  delay(2000);
}


void writeCommand(uint8_t cmd) {
 Wire.beginTransmission(OLED_ADDR);
 Wire.write(0x00);
 Wire.write(cmd);
 byte error = Wire.endTransmission();
 if(error != 0) {
   Serial.print("Cmd Error 0x");
   Serial.print(cmd, HEX);
   Serial.print(": ");
   Serial.println(error);
 }
}

void writeData(uint8_t data) {
 Wire.beginTransmission(OLED_ADDR);
 Wire.write(0x40);
 Wire.write(data);
 byte error = Wire.endTransmission();
 if(error != 0) {
   Serial.print("Data Error: ");
   Serial.println(error);
 }
}