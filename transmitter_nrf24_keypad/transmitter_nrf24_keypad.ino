/*
  Dmytro's transmitter,  catch command from keyboard and send it as command to reciever
  based on RF24/pingpair_ack
  tested 01.03.2020
*/


#include <Keypad.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins
RF24 radio(9, 10);

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };              // Radio pipe addresses for the 2 nodes to communicate.

// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.

typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_ping_out;                                              // The role of the current running sketch

// A single byte to keep track of the data being sent back and forth
byte command = 0;

const byte ROWS = 4; // Four rows
const byte COLS = 3; // Four columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 2, 3, 4, 5 };
// Connect keypad COL0, COL1 and COL2, COL3 to these Arduino pins.
byte colPins[COLS] = { 6, 7, 8 };

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


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

  role = role_ping_out;                  // Become the primary transmitter (ping out)
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
}

void loop(void) {

  if (role == role_ping_out) {
    char key = kpd.waitForKey();

    switch (key) {
      case 49:
        command  = 1;
        break;
      case 51:
        command  = 3;
        break;
      case 52:
        command  = 4;
        break;
      case 53:
        command  = 5;
        break;
      case 54:
        command  = 6;
        break;
      case 55:
        command  = 7;
        break;
      case 56:
        command  = 8;
        break;
      case 57:
        command  = 9;
        break;
      default:
        command = 0;
        break;
    }

    radio.stopListening();                                  // First, stop listening so we can talk.

    printf("Now sending %d as payload. ", command);
    byte gotByte;
    unsigned long time = micros();                          // Take the time, and send it.  This will block until complete
    //Called when STANDBY-I mode is engaged (User is finished sending)
    if (!radio.write( &command, 1 )) {
      Serial.println(F("failed."));
    } else {

      if (!radio.available()) {
        Serial.println(F("Blank Payload Received."));
      } else {
        while (radio.available() ) {
          unsigned long tim = micros();
          radio.read( &gotByte, 1 );
          printf("Got response %d, round-trip delay: %lu microseconds\n\r", gotByte, tim - time);

        }
      }

    }
    // Try again later
    //delay(2000);
  }


}
