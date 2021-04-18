

/*
   Copyright GI1MIC (2021)

   *** For non-commercial use only ***
   
   Creative Commons NonCommercial license
 
    RC-28 Emulator
    
    Check the readme for important info on the USB VID/PID
    and USB descriptors.

*/

#define RAWHID_TX_SIZE          64      // transmit packet size
#define RAWHID_TX_INTERVAL      2       // max # of ms between transmit packets
#define RAWHID_RX_SIZE          64      // receive packet size
#define RAWHID_RX_INTERVAL      2       // max # of ms between receive packets

#include <RotaryEncoder.h>
#include "HID-Project.h"

// This is a work-in-progress
// Responses always consist of 6 bytes terminated with ASCII "210"
// Byte 0 is the response type

// Response 0x01
// Byte 0 0x01
// Byte 1 An encoder "accelerator". Can be set to 0x01, 0x02, 0x03, 0x04
// Byte 2 0x00 // Delimiter
// Byte 3 0x00 No movement, toggle between 0x01 & 0x02 to move dial
// Byte 4 0x00 // Delimiter
// Byte 5 Button/status update
// Byte 6 ASCII '2' // Would guess the developer filled the buffer with 7,6,5,4,3,2,1,0 to aid debugging
// Byte 7 ASCII '1'
// Byte 8 ASCII '0'
// everything else 0x00

// Response 0x02
// Byte 0 0x02
// Byte 1 0x31  // 1
// Byte 2 0x30  // 0
// Byte 3 0x32  // 2
// Byte 4 0x20  // Space
// Byte 5 0x33  // 3 // // Would guess the developer filled the buffer with 7,6,5,4,3,2,1,0 to aid debugging
// Byte 6 0x32  // 2
// Byte 7 0x31  // 1
// Byte 8 0x30  // 0
// everything else 0x00

#define RESPONSE_SW_VERSION   "\x02\x31\x30\x32\x20\x33\x32\x31\x30\x00"    // Version 1.02
#define RESPONSE_DEFAULT      "\x01\x01\x00\x00\x00\x00\x32\x31\x30\x00"    // Used to fill the send buffer

#define F1_MOMENTARY  0b01111101  // Latch versions also seem to exist
#define F2_MOMENTARY  0b00000011
#define TX_MOMENTARY  0b00000110

const int pinLed = LED_BUILTIN;
const int pinButton = 2;

unsigned int counter = 0;

#define F1_BUTTON     8
#define F1_LED        9
#define F2_BUTTON     4
#define F2_LED        5
#define TX_BUTTON     6
#define TX_LED        7
#define ENCODER_PIN1  2
#define ENCODER_PIN1_INT  1
#define ENCODER_PIN2  3
#define ENCODER_PIN2_INT  0

unsigned long  encoder_max_rpm = 0;

RotaryEncoder encoder(ENCODER_PIN1, ENCODER_PIN2, RotaryEncoder::LatchMode::TWO03);

uint8_t Raw_HID_Buffer[80];
uint8_t HIDsendBuffer[64];
char HIDReceive[64];
uint8_t HIDReceivePtr = 0;

volatile long encoder_pos, new_encoder_pos = 0;

//------------------------------------------------------------------------------
void checkPosition()
{
  encoder.tick(); // just call tick() to check the state.
}

//------------------------------------------------------------------------------
void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);

  pinMode(F1_LED, OUTPUT);
  pinMode(F1_BUTTON, INPUT_PULLUP);

  pinMode(F2_LED, OUTPUT);
  pinMode(F2_BUTTON, INPUT_PULLUP);

  pinMode(TX_LED, OUTPUT);
  pinMode(TX_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  RawHID.begin(Raw_HID_Buffer, sizeof(Raw_HID_Buffer));

  attachInterrupt(ENCODER_PIN1, checkPosition, CHANGE);
  attachInterrupt(ENCODER_PIN2, checkPosition, CHANGE);

  HIDsendBuffer[0] = 0x00;
}

//------------------------------------------------------------------------------
void loop() {

  // Send button updates
  while (!digitalRead(F1_BUTTON)) {
    HIDsendBuffer[5] = F1_MOMENTARY;
    RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
  }
  while (!digitalRead(F2_BUTTON)) {
    HIDsendBuffer[5] = F2_MOMENTARY;
    RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
  }
  while (!digitalRead(TX_BUTTON)) {
    HIDsendBuffer[5] = TX_MOMENTARY;

    RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
  }

  // Send encoder updates
  encoder.tick();
  new_encoder_pos = encoder.getPosition();
  if (encoder_pos != new_encoder_pos) {
    //    unsigned long  t = encoder.getRPM();
    //    if (encoder_max_rpm < t) // Figure out the max RPM
    //      encoder_max_rpm = t;
    //    t = encoder_max_rpm / (t * 4);
    //    t++;                     // Need a value between 1 and 4
    //    if (t > 5) t = 4;
    //    HIDsendBuffer[1] = t;    // Set the speed byte
    HIDsendBuffer[1] = 1;    // Set the speed byte
    encoder_pos = new_encoder_pos;
    if ( (int)encoder.getDirection() == -1 ) {
      HIDsendBuffer[3] = 0x01;  // Down
    } else {
      HIDsendBuffer[3] = 0x02; // Up
    }
    HIDsendBuffer[5] = 0b0000111;
    RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
  } else {
    HIDsendBuffer[3] = 0x00; // Up
    HIDsendBuffer[5] = 0b0000111;
    RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
  }


  // Debug - dump the 8 bytes of control data to the serial port
  //    for (int i = 0; i < 6; i++) {
  //      Serial.print(HIDsendBuffer[i], HEX);
  //      Serial.print(" ");
  //    }
  //    Serial.println();

  // Process received requests
  auto bytesAvailable = RawHID.available();
  if (bytesAvailable)
  {
    while (bytesAvailable--) {
      HIDReceive[HIDReceivePtr++] = RawHID.read();
    }
    HIDReceivePtr = 0;

    switch (HIDReceive[0]) {
      case 0x01: // Host command (Set leds and ?)
        digitalWrite(TX_LED, !bitRead(HIDReceive[1], 0));
        digitalWrite(F1_LED, !bitRead(HIDReceive[1], 1));
        digitalWrite(F2_LED, !bitRead(HIDReceive[1], 2));
        break;
      case 0x02: // Host requesting firmware version
        Serial.print("Version request: ");
        strcpy(HIDsendBuffer, RESPONSE_SW_VERSION);
        RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
        memcpy(HIDsendBuffer, RESPONSE_DEFAULT, sizeof(RESPONSE_DEFAULT));
        break;
      case 0x06: // Host wants to send firmware update (Not sure how to respond to this - thing versions after 1.01 use PIC AES encryption)
        Serial.println("Firmware update request");
        strcpy(HIDsendBuffer, "\x02               ");
        RawHID.write(HIDsendBuffer, sizeof(HIDsendBuffer));
        break;
      default: // Unknown host request
        Serial.println("Unknown request");
        for (int i = 0; i < sizeof(HIDReceive); i++) {
          Serial.print(HIDReceive[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
        break;
    }
  }
}
