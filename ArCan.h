/****************************************************************************
 *
 * Proyecto ArCan
 *
 * Copyright (C) 2008-2009 Raúl Milla Pérez raulmp(at)arcan.es
 *          http://www.arcan.es
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
	void mode(byte mode);
	void Set_RXB0Filters(uint16_t Mask0, filter0, filter1);
	void Set_RXB1Filters(uint16_t Mask1, filter2, filter3, filter4, filter5);

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

