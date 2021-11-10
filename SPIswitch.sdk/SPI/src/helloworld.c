/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xspi.h"
#include "xspi_l.h"
#include <unistd.h>


#define SPI_DEVICE_ID XPAR_SPI_0_DEVICE_ID

#define BUFFER_SIZE 12
#define BANK0 0xC0
#define BANK1 0xC2
#define BANK2 0xC4
#define BANK3 0xC6
#define WCR 0x40
#define RCR 0x00
#define EUDASTL 0x16

/*
TABLE 4-1: SPI INSTRUCTION SET
WCR - 010a aaaa 0x40
RCR - 000a aaaa
BFS - 100a aaaa
BFC - 101a aaaa

COCON<3:0> (ECON2<11:8> BANK3), which controls the frequency output on CLKOUT.
1111 = 50 kHz nominal ((4 * FOSC)/2000)
1110 = 100 kHz nominal ((4 * FOSC)/1000) 0000 1110 0000 0000 => ECON2H = 0x0E


*/


static XSpi SpiInstance;

void bufferWriter(unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, int ByteCount);

u8 ReadBuffer[BUFFER_SIZE];
u8 WriteBuffer[BUFFER_SIZE];
u8 MAC[6];

static XSpi SpiInstance;   /* The instance of the SPI device */

int main()
{
	int Status;
    init_platform();

    print("START\n\r");



    XSpi_Initialize(&SpiInstance, SPI_DEVICE_ID);
	XSpi_Start(&SpiInstance);
	XSpi_IntrGlobalDisable(&SpiInstance);
    Status = XSpi_SetOptions(&SpiInstance, XSP_MASTER_OPTION);
    XSpi_SetSlaveSelect(&SpiInstance, 0x01);


    //Clean Buffer



	/////////////Read MAC from MAADDR//////////////////
	bufferWriter(BANK3, 0x00, 0x00, 0x00, 2);	//Select Bank3
	for(int i=0x00; i < 0x06; i++){
		bufferWriter(i, 0x00, 0x00, 0x00, 2);	//read MAADR from 0 to 5
		MAC[i]=ReadBuffer[1];
	}


	/////////////Initialization//////////////////
	//1. Write 1234h to EUDAST.
	bufferWriter(BANK0, 0x00, 0x00, 0x00, 2);	//Select Bank0
	bufferWriter(WCR + EUDASTL, 0x34, 0x12, 0x00, 3);	//Set EUDASTL=16 to 0x1234

	//2. Read EUDAST to see if it now equals 1234h.
    print("----------------------T\n\r");
    print("START\n\r");

	bufferWriter(0x16, 0x00, 0x00, 0x00, 4);	//verify bank 0 #16 EUDASTL, #17 EUDASTH

	//3. Poll CLKRDY (ESTAT<12>) and wait for it to become set. 0001 0000 0000 0000
	bufferWriter(0x1A, 0x00, 0x00, 0x00, 2);	//ESTATH = 0x10 ?
	bufferWriter(0x1B, 0x00, 0x00, 0x00, 2);	//ESTATL

	//4. Issue a System Reset command by setting ETHRST (ECON2<4>).
	bufferWriter(BANK3, 0x00, 0x00, 0x00, 2);		//Select Bank3
    bufferWriter(0x0E, 0x00, 0x00, 0x00, 3);		//RCR ECON2L optional
	bufferWriter(WCR+0x0E, 0x10, 0x00, 0x00, 2);	//WCR (0x40) + 0x0E = 0x4E  ECON2<4> = 1 = 0x10
	bufferWriter(0x0E, 0x00, 0x00, 0x00, 3);		//RCR ECON2 optional

	//5. In software, wait at least 25 us for the Reset to take place and the SPI/PSP interface to begin operating again.
	usleep(2500 * 1000);

	//6. Read EUDAST to confirm that the System Reset took place. EUDAST should have reverted back to its Reset default of 0000h.
	bufferWriter(BANK0, 0x00, 0x00, 0x00, 2);	//Select Bank0
	bufferWriter(0x10, 0x00, 0x00, 0x00, 4);	//verify bank 0 #16 EUDASTL, #17 EUDASTH

	//7. Wait at least 256 us for the PHY registers and PHY status bits to become available.
	usleep(2500 * 1000);


	//CLKOUT Frequency
	bufferWriter(BANK3, 0x00, 0x00, 0x00, 2);		//Select Bank3
	bufferWriter(WCR+0x0E, 0x10, 0x00, 0x00, 2);	//WCR (0x40) + 0x0E = 0x4E  ECON2<4> = 1 = 0x10



	xil_printf("\r\n\r\n");

    cleanup_platform();
    return 0;
}


void bufferWriter(unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, int ByteCount ){
    ReadBuffer[0] = 0x00;
	ReadBuffer[1] = 0x00;
	ReadBuffer[2] = 0x00;
	ReadBuffer[3] = 0x00;
	WriteBuffer[0] = v0;
	WriteBuffer[1] = v1;
	WriteBuffer[2] = v2;
	WriteBuffer[3] = v3;
	XSpi_Transfer(&SpiInstance, WriteBuffer, ReadBuffer, ByteCount);
	xil_printf("readBuffer: 0x%02x 0x%02x 0x%02x 0x%02x\r\n", ReadBuffer[0], ReadBuffer[1], ReadBuffer[2], ReadBuffer[3]);
	}

