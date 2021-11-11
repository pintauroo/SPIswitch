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

/*
TABLE 4-1: SPI INSTRUCTION SET
WCR - 010a aaaa 0x40
RCR - 000a aaaa
BFS - 100a aaaa
BFC - 101a aaaa
*/
#define RCR 0x00
#define WCR 0x40
#define BFS 0x80
#define BFC 0xA0


//-------------------------
#define BANK0 0xC0

#define ERXSTL 0x04
#define ERXSTH 0x05
#define ERXTAILL 0x06
#define ERXTAILH 0x07
#define ERXHEADL 0x08
#define ERXHEADH 0x09
#define ESTATL 0x1A
#define ESTATH 0x1B
#define ECON1L 0x1E
#define ECON1F 0x1F

//-------------------------
#define BANK1 0xC2
//-------------------------
#define BANK2 0xC4
#define MACON2L 0x02
#define MACON2H 0x03



#define MAMXFLL 0x0A
#define MAMXFLH 0x0B

//-------------------------
#define BANK3 0xC6

#define MADDR3L 0x01
#define MADDR3H 0x02
#define MADDR2L 0x03
#define MADDR2H 0x04
#define MADDR1L 0x05
#define MADDR1H 0x06
#define ECON2L 0x0E
#define ECON2H 0x0F
#define EUDASTL 0x16
#define EUDASTH 0x17
//-------------------------





/*

COCON<3:0> (ECON2<11:8> BANK3), which controls the frequency output on CLKOUT.
1111 = 50 kHz nominal ((4 * FOSC)/2000)
1110 = 100 kHz nominal ((4 * FOSC)/1000) 0000 1110 0000 0000 => ECON2H = 0x0E


*/


static XSpi SpiInstance;

//void bufferWriter(unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, int ByteCount);
void bufferWriter(unsigned char *outData, int ByteCount );

u8 ReadBuffer[BUFFER_SIZE];
u8 WriteBuffer[BUFFER_SIZE];
u8 MAC[6];
u8 rcvBufAdd[2];
u8 trsBufAdd[2];

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


    //READ MAC
	bufferWriter((unsigned char []){BANK3}, 1);	//Select Bank3
	bufferWriter((unsigned char []){MADDR3L}, 7);	//read MAADR from 0 to 5
	for(int i=0x00; i < 0x06; i++){
		MAC[i]=ReadBuffer[i+1]; //the first value is always 0
	}


	//1. Write 1234h to EUDAST.
	bufferWriter((unsigned char []){BANK0}, 1);	//Select Bank0
	bufferWriter((unsigned char []){WCR + EUDASTL, 0x34, 0x12}, 3);

	//2. Read EUDAST to see if it now equals 1234h.
	bufferWriter((unsigned char []){EUDASTL}, 2);
	bufferWriter((unsigned char []){EUDASTH}, 2);


	//3. Poll CLKRDY (ESTAT<12>) and wait for it to become set. 0001 0000 0000 0000 = 0x10
	bufferWriter((unsigned char []){ESTATH}, 2);

	while((ReadBuffer[1]&(0x10))!=(0x10)){
		bufferWriter((unsigned char []){ESTATH}, 2);
	}

	//4. Issue a System Reset command by setting ETHRST (ECON2<4>).
	bufferWriter((unsigned char []){BANK3}, 1);
	bufferWriter((unsigned char []){BFS+ECON2L, 0x10}, 2);

	//5. In software, wait at least 25 us for the Reset to take place and the SPI/PSP interface to begin operating again.
	usleep(25 * 1000);

	//6. Read EUDAST to confirm that the System Reset took place. EUDAST should have reverted back to its Reset default of 0000h.
	bufferWriter((unsigned char []){BANK0}, 1);
	bufferWriter((unsigned char []){EUDASTL}, 3);

	if((ReadBuffer[1]&(0x00))!=(0x00) && (ReadBuffer[2]&(0x00))!=(0x00)){
		xil_printf("HOW TO RISE AN ERROR? \r\n\r\n");
	}

	//7. Wait at least 256 us for the PHY registers and PHY status bits to become available.
	usleep(250 * 1000);

	//CLKOUT Frequency
	//bufferWriter(BANK3, 0x00, 0x00, 0x00, 2);			//Select Bank3
	//bufferWriter(WCR+ECON2L, 0x10, 0x00, 0x00, 2);	//WCR (0x40) + 0x0E = 0x4E  ECON2<4> = 1 = 0x10



	//MAC Settings
	xil_printf("\r\n\r\n");
	xil_printf("\r\n\r\n");
	xil_printf("\r\n\r\n");
	xil_printf("\r\n\r\n");

	//set HFRMEN (MACON2<2>) to accept any size frame.
	bufferWriter((unsigned char []){BANK2}, 1);
	bufferWriter((unsigned char []){MACON2L}, 3);
	bufferWriter((unsigned char []){BFS+MACON2L, 0x04}, 2);
	bufferWriter((unsigned char []){BFS+MACON2H, 0xff}, 2);
	bufferWriter((unsigned char []){MACON2L}, 3);




    cleanup_platform();
    return 0;
}


//void bufferWriter(unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, unsinged char *outData, int ByteCount ){
void bufferWriter(unsigned char *outData, int ByteCount ){
    if(ByteCount <= BUFFER_SIZE) {
    	for(int i=0; i<ByteCount; i++) {
    		ReadBuffer[i] = 0x00;
    	}
    }

	XSpi_Transfer(&SpiInstance, outData, ReadBuffer, ByteCount);

	xil_printf("writeBuffer: ");
	for(int i=0; i<sizeof(outData); i++) {
		xil_printf("0x%02x ", outData[i]);
	}

	xil_printf("readBuffer: ");
	for(int i=0; i<ByteCount; i++) {
		xil_printf("0x%02x ", ReadBuffer[i]);
	}
	xil_printf("\r\n\r\n");
	}

