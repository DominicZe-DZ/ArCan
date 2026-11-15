/*
 * ArCan basic example
 *
 * Receives CAN frames and prints them on the serial monitor,
 * and sends a test frame (id 0x123) every second.
 *
 * Board: Arduino Uno / Duemilanove / Nano (ATmega328) with the ArCan shield
 * (MCP2515 with a 16 MHz crystal, CS on pin 10, INT on pin 9).
 *
 * ArCan - Copyright (c) 2008-2013 Raúl Milla Pérez (DominicZe) - MIT License
 * https://dominicze.com/proyectos/arcan-can-bus-arduino/
 */
#include <ArCan.h>

tArCan can;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);

  // Bus speed in kbps: 125, 250, 500 or 1000
  if (!can.begin(125)) {
    Serial.println("MCP2515 not found, check the shield");
    while (true) {}
  }
  Serial.println("ArCan ready at 125 kbps");
}

void loop() {
  // The MCP2515 pulls INT low when a frame is waiting in its buffers
  if (digitalRead(ArCan_INT) == LOW) {
    can.get_message_to_buffer_rx();
  }

  // Print every frame stored in the library's receive buffer
  while (can.check_message()) {
    tArCan::tCAN msg;
    can.get_message(&msg);
    can.print_message(&msg);
  }

  // Send a test frame once per second
  if (millis() - lastSend >= 1000) {
    lastSend = millis();

    tArCan::tCAN msg;
    msg.id = 0x123;           // standard 11-bit identifier
    msg.header.rtr = 0;
    msg.header.length = 2;
    msg.data[0] = 0xAB;
    msg.data[1] = 0xCD;
    can.send_message(&msg);
  }
}
