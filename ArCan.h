/****************************************************************************
 *
 * ArCan - CAN-Bus shield and MCP2515 library for Arduino
 *
 * Copyright (c) 2008-2013 Raúl Milla Pérez (DominicZe)
 * https://dominicze.com
 *
 * SPDX-License-Identifier: MIT
 * Released under the MIT License. See the LICENSE file for details.
 *
 ****************************************************************************/
#ifndef ArCan_h
#define ArCan_h


//#include "WConstants.h"         // include types & constants of Wiring core API
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include "WConstants.h"
#endif
#include "ArCan_def.h"

// Descripcion del interfaz de la libreria CanBus
class tArCan
{
  // Parte pública
  public:
	typedef struct{
          int id;
        
          struct {
              byte rtr : 1;
              byte length : 4;
          }header;
        
          byte data[8];
	}tCAN;
		  
	tArCan(void);
	boolean begin(void);
	boolean begin(uint16_t speed);
	boolean check_message(void);
	boolean check_free_buffer(void);
	byte get_message(tCAN *msje);
    byte get_message_to_buffer_rx(void);
	byte send_message(tCAN *msje);
    void print_message(tCAN *msje);
	void SetMode(byte mode);
	void SetRXB0Filters(uint16_t Mask0, uint16_t filter0, uint16_t filter1);
	void SetRXB1Filters(uint16_t Mask1, uint16_t filter2, uint16_t filter3, uint16_t filter4, uint16_t filter5);

  // Parte privada
  private:
    tCAN buffer_rx[8];
    byte buffer_rx_count;
    boolean check_message_flag;
    byte spi_putc( byte data );
	void write_register( byte address, byte data );
	byte read_register(byte address);
	void bit_modify(byte address, byte mask, byte data);
	byte read_status(byte type);
};

#endif

