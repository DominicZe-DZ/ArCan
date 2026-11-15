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
 
// include core Wiring API
//#include "WProgram.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/* DigitalWrite es excesivamente lento.
   Depender‡ del pin CS que usemos, en 
   en l’neas generales, ser‡ el 
   PortB.2 = Pin 10 de arduino */
#define ArCan_CS_LOW	PORTB &= 0xFB
// El ChipSelect debe tener tiempo m’nimo
// de 50ns el nop son 62,5ns
#define ArCan_CS_HIGH	PORTB |= 0x04; __asm__("nop\n\t")

// include this library's description file
#include "ArCan.h"
#include "ArCan_def.h"

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances
tArCan::tArCan(void)
{
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

//Escribir un Byte por SPI
byte tArCan::spi_putc( byte data )
{
	SPDR = data;
	
	// espera hasta que se ha enviado el dato
	while( !( SPSR & (1<<SPIF) ) )
	  __asm__("nop\n\t");
	
	return SPDR;
}

//***************************************************************************
//Escribir un registro del mcp2515
void tArCan::write_register( byte address, byte data )
{
	ArCan_CS_LOW;

	spi_putc(SPI_WRITE);
	spi_putc(address);
	spi_putc(data);
	
	ArCan_CS_HIGH;
}

//Leer un registros del mcp2515
byte tArCan::read_register(byte address)
{
	byte data;
	
	ArCan_CS_LOW;
	
	spi_putc(SPI_READ);
	spi_putc(address);
	
	data = spi_putc(0xff);	 //Envia un dummy
	
	ArCan_CS_HIGH;
	
	return data;
}

//***************************************************************************
//Modificar un bit del mcp2515
void tArCan::bit_modify(byte address, byte mask, byte data)
{
	ArCan_CS_LOW;
	
	spi_putc(SPI_BIT_MODIFY);
	spi_putc(address);
	spi_putc(mask);
	spi_putc(data);
	
	ArCan_CS_HIGH;
}
//***************************************************************************
//Consultar el estado del mcp2515 (2 tipos de consulta SPI_RX_STATUS o SPI_READ_STATUS
byte tArCan::read_status(byte type)
{
	byte data;
	
	ArCan_CS_LOW;
	
	spi_putc(type);
	data = spi_putc(0xff);  //Envia un dummy
	
	ArCan_CS_HIGH;
	
	return data;
}

//***************************************************************************
//Inicializa el mcp2515 con una velocidad por defecto, 125Kbps
boolean tArCan::begin(void)
{
	return begin(125);
}

//***************************************************************************
//Inicializa el mcp2515 (Reloj 16MHz)
boolean tArCan::begin(uint16_t speed)
{
	digitalWrite(ArCan_CS, HIGH);
	digitalWrite(P_MOSI, HIGH);
	digitalWrite(P_MISO, HIGH);
	digitalWrite(P_SCK, LOW);
	
	pinMode(ArCan_CS,OUTPUT);
	pinMode(P_SCK,OUTPUT);
	pinMode(P_MOSI,OUTPUT);
	pinMode(P_MISO,INPUT);
	
	pinMode(ArCan_INT,INPUT);
	
	// Activamos y configuramos el SPI como Master a velocidad de 8MHz
	SPCR = (1<<SPE)|(1<<MSTR) | (0<<SPR1)|(0<<SPR0);
	// activamos SPI2X (Doble SPI Speed Bit) para alcanzar los 8MHz
	SPSR = (1<<SPI2X);
	
	
	// Hacemos un reset por soft al mcp2515
	// Despues del reset entra en modo de configuracion
	ArCan_CS_LOW;
	spi_putc(SPI_RESET);
	ArCan_CS_HIGH;

	// Esperamos un poco para que se estabilize el mcp2515 despues del reset
	delay(20);
	
	// load CNF1..3 Register
	ArCan_CS_LOW;
	spi_putc(SPI_WRITE);
	spi_putc(CNF3);  // Usaremos una escritura secuencial empezando en la 
                     // dirección 0x28h(CNF3) 0x29h(CNF2) 0x2A(CNF1)
                     // Si no se intruduce un valor valido se configura en 125Kbps
	
    switch(speed){
        case 250:   spi_putc((1<<PHSEG22)|(1<<PHSEG20));                            // Escribe en el CNF3 = 0x05
                    spi_putc((1<<BTLMODE)|(1<<PHSEG12)|(1<<PHSEG11)|(1<<PHSEG10));  // Escribe en el CNF2 = 0xB8
                    spi_putc((1<<BRP0));                                            // Escribe en el CNF1 = 0x01
                    break;
        case 500:   spi_putc((1<<PHSEG22)|(1<<PHSEG20));                            // Escribe en el CNF3 = 0x05
                    spi_putc((1<<BTLMODE)|(1<<PHSEG12)|(1<<PHSEG11)|(1<<PHSEG10));  // Escribe en el CNF2 = 0xB8
                    spi_putc(0x00);                                                 // Escribe en el CNF1 = 0x00
                    break;
        case 1000:  spi_putc((1<<PHSEG21));                                         // Escribe en el CNF3
                    spi_putc((1<<BTLMODE)|(1<<PHSEG11));                            // Escribe en el CNF2
                    spi_putc(0x00);                                                 // Escribe en el CNF1
                    break;
        default:    spi_putc((1<<PHSEG22)|(1<<PHSEG20));                            // Escribe en el CNF3 = 0x05
                    spi_putc((1<<BTLMODE)|(1<<PHSEG12)|(1<<PHSEG11)|(1<<PHSEG10));  // Escribe en el CNF2 = 0xB8
                    spi_putc((1<<BRP1)|(1<<BRP0));                                  // Escribe en el CNF1 = 0x03
                    break;
    }
	
	// Activamos la interrupcion
	spi_putc((1<<RX1IE)|(1<<RX0IE));
	ArCan_CS_HIGH;
	
	// Testeamos que la escritura se ha producido correctamente para verificar que el mcp2515 es accesible
	// Podemos probar cualquier otro registro, en este caso verifico que se ha escrito bien el registro de CANINTE
	if (read_register(CANINTE) != ((1<<RX1IE)|(1<<RX0IE))) {
		return false;
	}
	
	// Para que D2 y D3 se activen cuando los buffers se llenen
	write_register(BFPCTRL, (1<<B0BFM)|(1<<B1BFM)|(1<<B0BFE)|(1<<B1BFE));
	
	// Configuramos TXnRTS como entradas digitales
	write_register(TXRTSCTRL, 0);
	
	// Desactivamos todos los filtros en la recepcion de mensajes
	write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0)|(1<<BUKT));
	write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));
	
	// Cambiamos al modo Normal
	SetMode(NORMAL);
    
    // Inicializamos el buffer
    buffer_rx_count = 0;
	
	return true;
}
//***************************************************************************
// Chequea si existe algun mensaje nuevo en espera devuelve 1 si existe o 0 si no hay
boolean tArCan::check_message(void)
{
	return (buffer_rx_count > 0);
}

//***************************************************************************
//Chequea si el buffer esta vacio
boolean tArCan::check_free_buffer(void)
{
	byte status = read_status(SPI_READ_STATUS);
	
	if ((status & 0x54) == 0x54) {
		// verifica todos los buffers
		return false;
	}
	
	return true;
}

//***************************************************************************
//Cogemos el mensaje CAN para meterlo en el buffer de recepción
byte tArCan::get_message_to_buffer_rx(void)
{
	// Se lee el estado para comprabar en que buffer se encuentra el mensaje
	byte status = read_status(SPI_RX_STATUS);
	byte addr;
	
	if (bit_is_set(status,6)) {
		// Si el mensaje esta en el Buffer 1
		addr = SPI_READ_RX;
	}
	else if (bit_is_set(status,7)) {
		// Si el mensaje esta en el Buffer 2
		addr = SPI_READ_RX | 0x04;
	}
	else {
		// Error, el mensaje no esta disponible
		return 0;
	}

	ArCan_CS_LOW;
	spi_putc(addr);
	
	// leemos la id 
	buffer_rx[buffer_rx_count].id  = (char) spi_putc(0xff) << 3;       //leemos la parte alta
	buffer_rx[buffer_rx_count].id |= spi_putc(0xff) >> 5;              //leemos la parte baja
	buffer_rx[buffer_rx_count].id &= 0X07FF;
	
	spi_putc(0xff);		//devolvera la parte alta id extendida
	spi_putc(0xff);		//devolvera la parte baja de la id extendida
	
	// Leemos el DLC
	byte length = spi_putc(0xff) & 0x0f;   //solo nos quedamos con la logitud
	
	buffer_rx[buffer_rx_count].header.length = length;
	buffer_rx[buffer_rx_count].header.rtr = (bit_is_set(status, 3)) ? 1 : 0;
	
	// A continuacion leemos el dato
	for (byte i=0;i<length;i++) {
		buffer_rx[buffer_rx_count].data[i] = spi_putc(0xff);
	}
	ArCan_CS_HIGH;
	
	// borramos los flag de las interrupciones
	if (bit_is_set(status, 6)) {
		bit_modify(CANINTF, (1<<RX0IF), 0);
	}
	else {
		bit_modify(CANINTF, (1<<RX1IF), 0);
	}
    
    //Activamos el Flag interno
    check_message_flag = true;
    
    //Incrementamos el buffer_rx_count
    buffer_rx_count++;
	
	return status;
}

//***************************************************************************
//Cogemos el mensaje CAN de nuestro buffer de recepción
byte tArCan::get_message(tCAN *msje){
    msje->id = buffer_rx[buffer_rx_count - 1].id;
    msje->header.length = buffer_rx[buffer_rx_count - 1].header.length;
    msje->header.rtr = buffer_rx[buffer_rx_count - 1].header.rtr;
    
    for (byte i=0;i<buffer_rx[buffer_rx_count - 1].header.length;i++) {
		msje->data[i] = buffer_rx[buffer_rx_count - 1].data[i];
	}
    
    if(buffer_rx_count == 0){
        check_message_flag = false;
    }
    
    buffer_rx_count--;
    
    return buffer_rx_count;
}
//***************************************************************************
//Enviamos el mensaje CAN
byte tArCan::send_message(tCAN *msje)
{
	byte status = read_status(SPI_READ_STATUS);
	
	/* Byte de Estado:
	 * 
	 * Bit	Funcion
	 *  2	TXB0CNTRL.TXREQ
	 *  4	TXB1CNTRL.TXREQ
	 *  6	TXB2CNTRL.TXREQ
	 *
	 *Con esto conseguimos saber que buffer esta libre para enviar el mensaje
	 */
	byte address;
	if (bit_is_clear(status, 2)) {
		address = 0x00;
	}
	else if (bit_is_clear(status, 4)) {
		address = 0x02;
	} 
	else if (bit_is_clear(status, 6)) {
		address = 0x04;
	}
	else {
		// Todos los buffer estan usados no se puede enviar mensajes 
		//devuelve un 0xFF si no se puede enviar
		return 0;
	}
	
	ArCan_CS_LOW;
	spi_putc(SPI_WRITE_TX | address);
	
	spi_putc(msje->id >> 3);		//Primero se envia la parte alta ID10....ID3
        spi_putc(msje->id << 5);	//Despues enviamos la parte baja ID2....ID0
	
	spi_putc(0);	//Como no vamos a usar la ID Extendida la ponemos a 0 (parte alta)
	spi_putc(0);	//(parte baja)
	
	byte length = msje->header.length & 0x0f;
	
	if (msje->header.rtr) {
		spi_putc((1<<RTR) | length);
	}
	else {
		// se envia la longitud del mensaje
		spi_putc(length);
		
		// ahora enviamos los datos
		for (byte i=0;i<length;i++) {
			spi_putc(msje->data[i]);
		}
	}
	ArCan_CS_HIGH;

	// send message
	ArCan_CS_LOW;
	address = (address == 0) ? 1 : address;
	spi_putc(SPI_RTS | address);
	ArCan_CS_HIGH;
	
	return address;
}
//***************************************************************************
//Configura las Mascaras y Filtros
void tArCan::SetRXB0Filters(uint16_t Mask0, uint16_t filter0, uint16_t filter1)
    {
    // Modo de Configuracion
	SetMode(CONFIGURATION);
    
    // Configuracion de las mascaras de los buffers
    write_register( RXM0SIDH, Mask0 >> 3);
    write_register( RXM0SIDL, Mask0 << 5);
    
	// Configuracion del filtro 0  
    write_register( RXF0SIDH, filter0 >> 3);
    write_register( RXF0SIDL, filter0 << 5);
	// Configuracion del filtro 1  
	write_register( RXF1SIDH, filter1 >> 3);
	write_register( RXF1SIDL, filter1 << 5);
   
    // Activamos los filtros para el buffer RXB0
    write_register(RXB0CTRL, (1<<RXM0)|(1<<BUKT));
		
	// Modo de Configuracion
	SetMode(NORMAL);
}

void tArCan::SetRXB1Filters(uint16_t Mask1, uint16_t filter2, uint16_t filter3, uint16_t filter4, uint16_t filter5)
{
    // Modo de Configuracion
	SetMode(CONFIGURATION);
    
    // Configuracion de las mascaras de los buffers
    write_register( RXM1SIDH, Mask1 >> 3);
    write_register( RXM1SIDL, Mask1 << 5);
    
    // Configuracion del filtro 2  
    write_register( RXF2SIDH, filter2 >> 3);
    write_register( RXF2SIDL, filter2 << 5);
	// Configuracion del filtro 3  
    write_register( RXF3SIDH, filter3 >> 3);
    write_register( RXF3SIDL, filter3 << 5);
	// Configuracion del filtro 4  
    write_register( RXF4SIDH, filter4 >> 3);
    write_register( RXF4SIDL, filter4 << 5);
	// Configuracion del filtro 5  
    write_register( RXF5SIDH, filter5 >> 3);
    write_register( RXF5SIDL, filter5 << 5);
	
    // Activamos todos los filtros para el buffer RXB1
    write_register(RXB1CTRL, (1<<RXM0));
	
	// Modo de Configuracion
	SetMode(NORMAL);
}

//***************************************************************************
//Imprime por el puerto serie el mensaje CAN
void tArCan::print_message(tCAN *msje)
{
	Serial.print("id: ");
	Serial.println(msje->id,HEX);
	Serial.print("Longitud: ");
	Serial.println(msje->header.length,DEC);
	Serial.print("rtr: ");
	Serial.println(msje->header.rtr,BIN);
    
    
	if (!msje->header.rtr) {
		Serial.print("Datos: ");
		
		for (byte i = 0; i < msje->header.length; i++) {
			Serial.print(msje->data[i],DEC);
		}
		Serial.println();
	}
}

//***************************************************************************
//Establece el modo
void tArCan::SetMode(byte mode)
{
	switch (mode) {
		case CONFIGURATION:
			bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), (1<<REQOP2));
			delayMicroseconds(4); // Parece que hay que esperar despuŽs de entrar
			break;				  // en modo de configuraci—n (No viene en DSheet)
		case NORMAL:
			bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
			break;
		case SLEEP:
			bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), (1<<REQOP0));
			break;
		case LISTEN:
			bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), (1<<REQOP1)|(1<<REQOP0));
			break;
		case LOOPBACK:
			bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), (1<<REQOP1));
			break;
	}
	
}
// Private Methods /////////////////////////////////////////////////////////////
// Functions only available to other functions in this library



