/*
  // March 2014 - TMRh20 - Updated along with High Speed RF24 Library fork
  // Parts derived from examples by J. Coliz <maniacbug@ymail.com>
*/
/**
   Example for efficient call-response using ack-payloads

   This example continues to make use of all the normal functionality of the radios including
   the auto-ack and auto-retry features, but allows ack-payloads to be written optionally as well.
   This allows very fast call-response communication, with the responding radio never having to
   switch out of Primary Receiver mode to send back a payload, but having the option to if wanting
   to initiate communication instead of respond to a commmunication.
*/



#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.

typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

// A single byte to keep track of the data being sent back and forth
byte counter = 1;

// rgb - led pin configure
#define led_1_Pin A0
#define led_2_Pin A2
#define led_3_Pin A1

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval_ON = 200;           // interval at which to blink (milliseconds)
const long interval_OFF = 100;           // interval at which to blink (milliseconds)


void setup() {

  Serial.begin(115200);
  printf_begin();
  Serial.print(F("\n\rRF24/examples/pingpair_ack/\n\rROLE: "));
  Serial.println(role_friendly_name[role]);

  // Setup and configure rf radio

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0, 15);                // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(pipes[1]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1, pipes[0]);
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  role = role_pong_back;                // Become the primary receiver (pong back)
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  radio.startListening();

  pinMode(led_1_Pin, OUTPUT);
  pinMode(led_2_Pin, OUTPUT);
  pinMode(led_3_Pin, OUTPUT);

  analogWrite(led_1_Pin, 0);
  analogWrite(led_2_Pin, 0);
  analogWrite(led_3_Pin, 0);
}

void blink_without_delay(int red, int green, int blue, int count)
{
  int iterator = 0;

  while (iterator <= count) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval_ON) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
        analogWrite(led_1_Pin, red);
        analogWrite(led_2_Pin, green);
        analogWrite(led_3_Pin, blue);
        iterator++;
      } else {
        ledState = LOW;
        analogWrite(led_1_Pin, 0);
        analogWrite(led_2_Pin, 0);
        analogWrite(led_3_Pin, 0);
      }



      //Serial.println(count);


      if (iterator > count) {
        ledState = LOW;
        analogWrite(led_1_Pin, 0);
        analogWrite(led_2_Pin, 0);
        analogWrite(led_3_Pin, 0);
      }
      //Serial.println(iterator);
    }
  }
}

void loop(void) {
  // Pong back role.  Receive each packet, dump it out, and send it back

  if ( role == role_pong_back ) {
    byte pipeNo;
    byte gotByte;                                       // Dump the payloads until we've gotten everything
    while ( radio.available(&pipeNo)) {
      radio.read( &gotByte, 1 );
      switch (gotByte) {
        case 1:
          blink_without_delay(255, 0, 0, 1);
          break;
        case 3:
          blink_without_delay(0, 255, 0, 1);

          break;
        case 4:
          blink_without_delay(0, 0, 255, 1);

          break;
        case 5:
          blink_without_delay(0, 0, 255, 2);

          break;
        case 6:
          blink_without_delay(0, 0, 255, 3);

          break;
        case 7:
          blink_without_delay(255, 0, 255, 1);

          break;
        case 8:
          blink_without_delay(255, 0, 255, 2);

          break;
        case 9:
          blink_without_delay(255, 0, 255, 3);

          break;
        case 0:
          blink_without_delay(255, 0, 0, 5);
          break;

          radio.writeAckPayload(pipeNo, &gotByte, 1 );
      }
    }
  }
}
