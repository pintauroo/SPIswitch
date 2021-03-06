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




/*
 * Layer 2 switches (bridges) have a MAC address table that contains a MAC address and physical port number. Switches follow this simple algorithm for forwarding frames:

When a frame is received, the switch compares the SOURCE MAC address to the MAC address table. If the SOURCE is unknown, the switch adds it to the table along with the physical port number the frame was received on. In this way, the switch learns the MAC address and physical connection port of every transmitting device.

The switch then compares the DESTINATION MAC address with the table. If there is an entry, the switch forwards the frame out the associated physical port. If there is no entry, the switch sends the frame out all its physical ports, except the physical port that the frame was received on (Flooding). If the DESTINATION is on the same port as the SOURCE (if they're both on the same segment), the switch will not forward the frame.)

Note that the switch does not learn the destination MAC until it receives a frame from that device.
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xspi.h"
#include "xspi_l.h"
#include <unistd.h>


#define SPI_DEVICE_ID0 XPAR_SPI_0_DEVICE_ID
#define SPI_DEVICE_ID1 XPAR_SPI_1_DEVICE_ID

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

#define ETXSTL 0x00
#define ETXSTH 0x01
#define ETXLENL 0x02
#define ETXLENH 0x03
#define ERXSTL 0x04
#define ERXSTH 0x05
#define ERXTAILL 0x06
#define ERXTAILH 0x07
#define ERXHEADL 0x08
#define ERXHEADH 0x09
#define ETXSTATL 0x12
#define ETXSTATH 0x13
#define ETXWIREL 0x14
#define ETXWIREH 0x15
#define EUDASTL 0x16
#define EUDASTH 0x17
#define ESTATL 0x1A
#define ESTATH 0x1B
#define ECON1L 0x1E
#define ECON1H 0x1F

//-------------------------
#define BANK1 0xC2
//-------------------------
#define BANK2 0xC4
#define MACON2L 0x02
#define MACON2H 0x03
#define MAMXFLL 0x0A
#define MAMXFLH 0x0B
#define MIREGADRL 0x14


//-------------------------
#define BANK3 0xC6

#define MADDR3L 0x00
#define MADDR3H 0x01
#define MADDR2L 0x02
#define MADDR2H 0x03
#define MADDR1L 0x04
#define MADDR1H 0x05
#define MIWRL 0x06
#define MISTATL 0x0A
#define ECON2L 0x0E
#define ECON2H 0x0F

//-------------------------
// PHY SFR
#define PHANA 0x04



// General Purpose Pointers - Wraps from 0x5FFF to 0x0000
#define RGPDATA 0x28 //Read EGPDATA
#define WGPDATA 0x2A //Read ERXDATA
#define RGPRDPT 0x62 //Read EGPRDPT
#define WGPRDPT 0x60 //Write EGPRDPT
#define RGPWRPT 0x6E //Read EGPWRPT
#define WGPWRPT 0x6C //Write EGPWRPT

// RX Pointers  - Wraps from 0x5FFF to RXSTART
#define RRXDATA 0x2C //Read ERXDATA
#define WRXDATA 0x2E //Write ERXDATA
#define RRXRDPT 0x66 //Read ERXRDPT
#define WRXRDPT 0x64 //Write ERXRDPT
#define RRXWRPT 0x72 //Read ERXWRPT
#define WRXWDPT 0x70 //Write ERXWRPT


#define WUDARDPT 0x68 //Read EUDARDPT
#define RUDARDPT 0x6A //Read EUDARDPT

#define WUDAWRPT 0x74 //Read EUDAWRPT
#define RUDAWRPT 0x76 //Read EUDAWRPT

#define RUDADATA 0x30 //Read EUDADATA
#define WUDADATA 0x32 //Read EUDADATA




#define SETPKTDEC 0xCC




/*

COCON<3:0> (ECON2<11:8> BANK3), which controls the frequency output on CLKOUT.
1111 = 50 kHz nominal ((4 * FOSC)/2000)
1110 = 100 kHz nominal ((4 * FOSC)/1000) 0000 1110 0000 0000 => ECON2H = 0x0E


*/


//void bufferWriter(unsigned char v0, unsigned char v1, unsigned char v2, unsigned char v3, int ByteCount);
void bankSelect(XSpi *SpiInstancePtr,unsigned char add);
void net100Writer(XSpi *SpiInstancePtr,unsigned char *outData, int ByteCount);
unsigned int net100ReadByte(XSpi *SpiInstancePtr,unsigned char add);
unsigned int net100ReadWord(XSpi *SpiInstancePtr,unsigned char add);
void net100WriteWord(XSpi *SpiInstancePtr,unsigned char add, unsigned int word);
unsigned int receivePkt(XSpi *SpiInstancePtr,unsigned int add);
unsigned int sendPkt(XSpi *SpiInstancePtr,unsigned char *dataAddr, int numbyte);
void net100Init(XSpi *SpiInstancePtr, int SPI_ID);
void net100Poll(XSpi *SpiInstancePtr);

u8 MAC[6];


static XSpi SpiInstance0;   /* The instance of the SPI device */
static XSpi SpiInstance1;   /* The instance of the SPI device */
static XSpi SpiInstance2;   /* The instance of the SPI device */
unsigned int *reg0 = XPAR_MYNETWORKFILTER_0_S00_AXI_BASEADDR;


int main()
{
    print("---SPISwitch---\n\r");




    init_platform();

/*
    for(int i=0; i<16; i++) {
    	*(reg0+i) = 0;
    }
    while(1) {
    	for(int i=0; i<16; i++) {
			xil_printf("reg%d: %u\r\n", i, *(reg0+i));
			*(reg0+i) = *(reg0+i) + 1;
    	}
    	xil_printf("\r\n");
    }
*/
    net100Init(&SpiInstance0, SPI_DEVICE_ID0);
    net100Init(&SpiInstance1, SPI_DEVICE_ID1);
    net100Init(&SpiInstance2, SPI_DEVICE_ID2);

    while(1){
        net100Poll(&SpiInstance0);
    }


    cleanup_platform();

    return 0;
}

void net100Poll(XSpi *SpiInstancePtr){
	int numPkts = 0;
	unsigned int rxTail;
	unsigned int val;


	// Poll PKTCNT (ESTAT<7:0>) to determine how many pending packets
	int x = 0;
	do{
		numPkts=net100ReadByte(SpiInstancePtr, ESTATL);
		x++;
		if(x%100 == 0) {
			xil_printf(".");
		};
	}while(numPkts == 0);

	//1. Verify that a packet is waiting by ensuring that the PKTCNT<7:0>
	if(numPkts > 0) {
		xil_printf("\r\n");
		bankSelect(SpiInstancePtr, BANK0);
		//ERXHEAD, indicating the next location to be written, and automatically wraps back to ERXST when it reaches the end of memory
		val = net100ReadWord(SpiInstancePtr, ERXHEADL);
		//ERXTAIL, is maintained by software
		rxTail = net100ReadWord(SpiInstancePtr, ERXTAILL);
		xil_printf("Packets: %d - head: 0x%04x - tail: 0x%04x\r\n", numPkts, val, rxTail);
		NxtPkt = receivePkt(SpiInstancePtr, rxTail);
	}
	usleep(1000); // 0.001 Sec



}
void net100Init(XSpi *SpiInstancePtr, int SPI_ID){

	unsigned int val;
	int Status;

	XSpi_Initialize(SpiInstancePtr, SPI_ID);
	XSpi_Start(SpiInstancePtr);
	XSpi_IntrGlobalDisable(SpiInstancePtr);
	Status = XSpi_SetOptions(SpiInstancePtr, XSP_MASTER_OPTION);
	XSpi_SetSlaveSelect(SpiInstancePtr, 0x01);

	//READ MAC ADDRESS
	//XSpi_Transfer(SpiInstancePtr, (unsigned char []){BANK3}, NULL, 2);
	bankSelect(SpiInstancePtr,BANK3);
	val = net100ReadWord(SpiInstancePtr, MADDR1L);
	xil_printf("MAC1: 0x%04x ", val);
	val = net100ReadWord(SpiInstancePtr, MADDR2L);
	xil_printf("MAC2: 0x%04x ", val);
	val = net100ReadWord(SpiInstancePtr, MADDR3L);
	xil_printf("MAC3: 0x%04x\r\n", val);

	//1. Write 1234h to EUDAST.
	bankSelect(SpiInstancePtr, BANK0);

	net100Writer(SpiInstancePtr, (unsigned char []){WCR + EUDASTL, 0x34, 0x12}, 3);

	//2. Read EUDAST to see if it now equals 1234h.
	if( (net100ReadByte(SpiInstancePtr, EUDASTL)&(0x34)) != (0x34) && (net100ReadByte(SpiInstancePtr, EUDASTH)&(0x12)) != (0x12)){
		xil_printf("ERROR\r\n\r\n");
	}

	//3. Poll CLKRDY (ESTAT<12>) and wait for it to become set. 0001 0000 0000 0000 = 0x10
	do{val = net100ReadByte(SpiInstancePtr, ESTATH);
	}while( (val & (0x10)) != (0x10) );

	//4. Issue a System Reset command by setting ETHRST (ECON2<4>).
	bankSelect(SpiInstancePtr, BANK3);
	net100Writer(SpiInstancePtr,(unsigned char []){BFS+ECON2L, 0x10}, 2);

	//5. In software, wait at least 25 us for the Reset to take place and the SPI/PSP interface to begin operating again.
	usleep(25);

	//6. Read EUDAST to confirm that the System Reset took place. EUDAST should have reverted back to its Reset default of 0000h.
	bankSelect(SpiInstancePtr, BANK3);
	if( (net100ReadByte(SpiInstancePtr, EUDASTL)&(0x00)) != (0x00) && (net100ReadByte(SpiInstancePtr, EUDASTL)&(0x00)) != (0x00)){
		xil_printf("HOW TO RISE AN ERROR? \r\n\r\n");
	}

	//7. Wait at least 256 us for the PHY registers and PHY status bits to become available.
	usleep(500);

	// Configure receive buffer size
	bankSelect(SpiInstancePtr, BANK0);
	xil_printf("ERXST: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, ERXSTL));
	xil_printf("head: 0x%04x - tail: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, ERXHEADL), net100ReadWord(SpiInstancePtr, ERXTAILL));




	//MAC Initialization

	// Configure MAMXFL (1518 bytes or less by default)
	bankSelect(SpiInstancePtr, BANK2);
	xil_printf("MAMXFL: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, MAMXFLL));

	//Set the RXEN bit (ECON1<0>) to enable packet reception by the MAC.
	bankSelect(SpiInstancePtr, BANK2);
	net100Writer(SpiInstancePtr,(unsigned char []){BFS+ECON1L, 0x01}, 2);




	//PHY Initialization

	//Set PHY Auto Negoation write 0x05E1 to PHANA
    net100PHYWrite(SpiInstancePtr, PHANA, 0x05E1);

    //The current link status can be read from PHYLNK (ESTAT<8>).
    xil_printf("Link Not UP\r\n");
    do{
    	val = net100ReadWord(SpiInstancePtr, ESTATL);
    }while((val&0x0100) == 0);
	xil_printf("Link UP\r\n");


	if((val&0x0400) == 0) {
		xil_printf("PHY Half Deplex\r\n");
	}
	else {
		xil_printf("PHY Full Deplex\r\n");
		bankSelect(SpiInstancePtr, BANK2);
		net100WriteWord(SpiInstancePtr, BFS + MACON2L, 0x0001);
	}

	//When FULDPX (MACON2<0>) = 1:
	bankSelect(SpiInstancePtr, BANK2);
	val = net100ReadWord(SpiInstancePtr, MACON2L);
	if((val & 0x0001) == 0) {
		xil_printf("MAC Half Deplex\r\n");
	}else {
		xil_printf("MAC Full Deplex\r\n");
	}


}


void bankSelect(XSpi *SpiInstancePtr, unsigned char add){
	XSpi_Transfer(SpiInstancePtr, (unsigned char []){add}, NULL, 2);
}

void net100Writer(XSpi *SpiInstancePtr, unsigned char *outData, int ByteCount ){
	XSpi_Transfer(SpiInstancePtr, outData, NULL, ByteCount);
}

unsigned int net100ReadWord(XSpi *SpiInstancePtr, unsigned char add){
     unsigned int ret;
     unsigned char ReadBuffer[2];
     for(int i=0; i<3; i++) {
     		ReadBuffer[i] = 0x00;
     }
     XSpi_Transfer(SpiInstancePtr, (unsigned char []){add}, ReadBuffer, 3);
     ret = ReadBuffer[2]*256 + ReadBuffer[1]; // convert to word 0hXXXX
     return ret;
}

unsigned int net100ReadByte(XSpi *SpiInstancePtr, unsigned char add){
	unsigned char ReadBuffer[1];
	for(int i=0; i<2; i++) {
		ReadBuffer[i] = 0x00;
	}
    XSpi_Transfer(SpiInstancePtr, (unsigned char []){add}, ReadBuffer, 2);
	return ReadBuffer[1];
}

void net100WriteWord(XSpi *SpiInstancePtr, unsigned char add, unsigned int word) {
	 XSpi_Transfer(SpiInstancePtr, (unsigned char []){add, word&0xFF, word>>8}, NULL, 3);
	}

//WRITING PHY REGISTERS
void net100PHYWrite(XSpi *SpiInstancePtr, unsigned char add, unsigned int val) {
    // Write PHY Add to MIREGADR
	bankSelect(SpiInstancePtr, BANK2);
	net100WriteWord(SpiInstancePtr, WCR+MIREGADRL, add);
    // Write 16 bit val to MIWR - Low then High
	bankSelect(SpiInstancePtr, BANK3);
    net100WriteWord(SpiInstancePtr, WCR+MIWRL, val);
    // Wait 25.6 uSec
    usleep(26);
    // Check BUSY (MISTAT<0>)
    val = net100ReadWord(SpiInstancePtr, MISTATL);
    while((val & 0x0001) != 0) {
        val = net100ReadWord(SpiInstancePtr, MISTATL);
    }
}



unsigned int receivePkt(XSpi *SpiInstancePtr, unsigned int add) { // Add is expected to contain the PktTail
	unsigned int valW;
	unsigned char RSV[6];
	unsigned char eSRC[6];
	unsigned char eDST[6];
	unsigned char RdBuf[BUFFER_SIZE];
	unsigned int numBytes;
	unsigned char ipProto;
	unsigned int tcpLen;
	unsigned int udpLen;

	//2. Begin reading at address pointed to by the application variable, NextPacketPointer
	// Set ERXRDPT to desired location (Receive Buffer Read Pointer)
	do {
		net100WriteWord(SpiInstancePtr, WRXRDPT, add);
        valW = net100ReadWord(SpiInstancePtr, RRXRDPT);
        xil_printf("ERXRDPT: 0x%04x\r\n", valW);
	} while(valW != add);


	/*
	As with the general purpose pointers, operations with
	ERXDATA normally cause the ERXRDPT or
	ERXWRPT Pointer to automatically increment by one
	byte address. However, if the end of the receive buffer
	area (5FFFh) is reached, the pointer will increment to
	the start of the receive FIFO buffer area instead, as
	defined by ERXST (Figure 3-7).
	 */

	//3. Read the first two bytes of the packet, which are 	the address of the next packet and write to NextPacketPointer.

	//first read is a dummy word
	net100ReadWord(SpiInstancePtr, RRXDATA);

    //next packet start address
	int newTail = net100ReadWord(SpiInstancePtr, RRXDATA) - 2; //subtract the dummy byte?

	//4. Read the next six bytes, which are the Receive Status Vector (RSV).
	XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 7);
	memcpy(RSV, &RdBuf[1], 6);
	xil_printf("RSV: 0x%02x%02x%02x%02x%02x%02x\r\n", RSV[0], RSV[1], RSV[2], RSV[3], RSV[4], RSV[5]);

	numBytes = RSV[1]*256 + RSV[0]; // This includes the destination address, source address, type/length, data, padding and CRC fields.


	// if numBytes is odd add one
	if((numBytes & 0x01) == 1) {
		numBytes += 1;
	}
    xil_printf("RSV numBytes: 0x%04x\r\n",numBytes);


	//5. Read the Ethernet frame. The number of bytes to be read is indicated by the received byte count in the RSV read during step 4.
	xil_printf(" *** Ether *** \r\n");
	XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 7); //dest addr.
	memcpy(eDST, &RdBuf[1], 6);


	XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 7); //src addr.
	memcpy(eSRC, &RdBuf[1], 6);
	numBytes -= 12;
	xil_printf("SRC: 0x%02x%02x%02x%02x%02x%02x  -  DST: 0x%02x%02x%02x%02x%02x%02x  -  ", eSRC[0], eSRC[1], eSRC[2], eSRC[3], eSRC[4], eSRC[5], eDST[0], eDST[1], eDST[2], eDST[3], eDST[4], eDST[5]);

	/*6. As the frame is read and processed, incremental
	amounts of memory buffer can be freed up by
	updating the ERXTAIL Pointer value to the point
	where the packet has been processed, taking
	care to wrap back at the end of the received
	memory buffer. Once the whole frame has been
	processed, the final value of ERXTAIL should be
	equal to (NextPacketPointer ? 2).*/
	bankSelect(SpiInstancePtr, BANK0);
	net100WriteWord(SpiInstancePtr, WCR+ERXTAILL, newTail);
    xil_printf("ERXTAIL: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, ERXTAILL));

	/*7. Set PKTDEC (ECON1<8>) to decrement the
	PKTCNT bits. PKTDEC is automatically cleared
	by hardware if PKTCNT decrements to zero.*/
    net100ReadByte(SpiInstancePtr, SETPKTDEC);


    //Read type/lenght field
    int type= net100ReadByte(SpiInstancePtr, RRXDATA)*256 + net100ReadByte(SpiInstancePtr, RRXDATA);

    typedef union myType_t {
    	unsigned char c[4];
    	unsigned long l;
    } myType;

    myType val;
    //////////////////////////////////////////////////
    //copy dest to reg0; type to reg1
	val.c[0] = eDST[0]; val.c[1] = eDST[1]; val.c[2] = eDST[2]; val.c[3] = eDST[3];
	*(reg0) = val.l;
	xil_printf("reg0: %u\r\n", *(reg0));
	*(reg0+1) = eSRC;
	xil_printf("reg0: %u\r\n", *(reg0));
	*(reg0+2) = type;
	xil_printf("reg1: %u\r\n", *(reg0));
	*(reg0+3) = 1;

	unsigned int dataAddr = net100ReadWord(SpiInstancePtr, RRXRDPT);
	if(*(reg0+15) == 100){// broadcast arp pkt

		sendPkt(&SpiInstance0, dataAddr, 28+(6+6+2));
		sendPkt(&SpiInstance1, dataAddr, 28+(6+6+2));
		sendPkt(&SpiInstance2, dataAddr, 28+(6+6+2));
	}
	if(*(reg0+15) == 200){// forward to interface 1

		sendPkt(&SpiInstance1, dataAddr, 28+(6+6+2));
	}
	if(*(reg0+15) == 300){// forward to interface 2

		sendPkt(&SpiInstance2, dataAddr, 28+(6+6+2));
	}


	numBytes -= 2;

	if(type < 0x600) { // this is a len
		xil_printf("Len: 0x%04x\r\n", type);
	}
	else {
		xil_printf("Type: 0x%04x\r\n", type);

		if(type == 0x0806) { // This is an ARP packet - 28 Bytes

			//unsigned int dataAddr = net100ReadWord(SpiInstancePtr, RRXRDPT);
	        xil_printf("dataAddr: 0x%04x\r\n", dataAddr);
			xil_printf(" *** ARP Packet *** \r\n");
			xil_printf("ARP Data:\r\n");

			xil_printf("Hardware type : %02x %02x\r\n", net100ReadByte(SpiInstancePtr, RRXDATA), net100ReadByte(SpiInstancePtr, RRXDATA)); //Ethernet is 1.
			xil_printf("Protocol type : %02x %02x\r\n", net100ReadByte(SpiInstancePtr, RRXDATA), net100ReadByte(SpiInstancePtr, RRXDATA)); //For IPv4, this has the value 0x0800
			xil_printf("Hardware Address Length : %02x \r\n", net100ReadByte(SpiInstancePtr, RRXDATA)); //Ethernet address length is 6.
			xil_printf("Protocol Address Length : %02x \r\n", net100ReadByte(SpiInstancePtr, RRXDATA)); //IPv4 address length is 4.
			xil_printf("Request type : %02x %02x\r\n", net100ReadByte(SpiInstancePtr, RRXDATA), net100ReadByte(SpiInstancePtr, RRXDATA)); // 1 for request, 2 for reply.

			xil_printf("Tell: ");
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 7); //Sender hardware address (SHA)
			xil_printf("SHA: %02x %02x %02x %02x %02x %02x\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4], RdBuf[5], RdBuf[6]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5); //Sender protocol address (SPA)
			xil_printf("SPA: %02x %02x\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4]);
			xil_printf("Who has: ");
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 7); //Target hardware address (THA)
			xil_printf("THA: %02x %02x %02x %02x %02x %02x\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4], RdBuf[5], RdBuf[6]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5); //Target protocol address (TPA)
			xil_printf("TPA: %02x %02x\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4]);

			//sendPkt(SpiInstancePtr, dataAddr, 28+(6+6+2));



			numBytes -= 28;
		}else if(type == 0x0800) { // the paylod is an IPv4
			xil_printf(" *** IPv4 Header *** \r\n");
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
			tcpLen = RdBuf[3]*256+RdBuf[4]; // Needed if the payload is tcp
			xil_printf("Ver: 0x%01x  -  IHL: 0x%01x  -  DSCP: 0x%02x - ECN: 0x%01x -  Len: 0x%04x\r\n", RdBuf[1]>>4, RdBuf[1]&0x0F, (RdBuf[2]&0xFC)>>2, RdBuf[2]&0x03, RdBuf[3]*256+RdBuf[4]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
			xil_printf("Ident: 0x%02x  -  Flags: 0x%01x  -  Frag Off: 0x%04x\r\n", RdBuf[1]*256+RdBuf[2], RdBuf[3]>>5, (RdBuf[3]&0x1F)*256+RdBuf[4]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
			ipProto = RdBuf[2];
			xil_printf("TTL: 0x%02x  -  Prot: 0x%02x  -  HD CKS: 0x%04x\r\n", RdBuf[1], RdBuf[2], RdBuf[3]*256+RdBuf[4]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
			xil_printf("SRC IP: %03d.%03d.%03d.%03d\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4]);
			XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
			xil_printf("DST IP: %03d.%03d.%03d.%03d\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4]);
			numBytes = numBytes - 20;
			tcpLen -= 20; // subtract the IPv4 header
			//if IHL > 5 there are option rows

			if(ipProto == 6) { // TCP
				xil_printf(" *** TCP Header *** \r\n"); // this might be wrong depending on what is deciphered above
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("SRC Port: 0x%04x  -  DST Port: 0x%04x\r\n", RdBuf[1]*256+RdBuf[2], RdBuf[3]*256+RdBuf[4]);
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("Seq Num: %09d\r\n", (((((RdBuf[1]<<8)+RdBuf[2])<<8)+RdBuf[3])<<8)+RdBuf[4]);
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("Ack Num: 0x%01x\r\n", (((((RdBuf[1]<<8)+RdBuf[2])<<8)+RdBuf[3])<<8)+RdBuf[4]);
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("Data Off: 0x%01x  -  Flags: 0x%03x  -  Wnd Size: 0x%04x\r\n", RdBuf[1]>>1, (RdBuf[1]&0x01)*256+RdBuf[2], RdBuf[3]*256+RdBuf[4]);
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("Checksum: 0x%04x  -  Urgent PTR: 0x%04x\r\n", RdBuf[1]*256+RdBuf[2], RdBuf[3]*256+RdBuf[4]);
				// if Data Offset > 5 there are option rows
				numBytes = numBytes - 20;
				tcpLen -= 20; // subtract the TCP header
				xil_printf("TCP Data: 0x");
				for(int i=0; i<tcpLen; i++) {
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00}, RdBuf, 2);
				//                xil_printf("%02x", RdBuf[1]);
				}
				xil_printf("\r\n");
				numBytes = numBytes - tcpLen;
			}
			else if(ipProto == 17) { // UDP
				xil_printf(" *** UDP Head *** \r\n"); // this might be wrong depending on what is deciphered above
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				xil_printf("SRC Port: 0x%04x  -  DST Port: 0x%04x\r\n", RdBuf[1]*256+RdBuf[2], RdBuf[3]*256+RdBuf[4]);
				XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00, 0x00, 0x00, 0x00}, RdBuf, 5);
				udpLen = RdBuf[1]*256+RdBuf[2];
				xil_printf("Length: 0x%04x  -  Checksum: 0x%04x\r\n", RdBuf[1]*256+RdBuf[2], RdBuf[3]*256+RdBuf[4]);
				numBytes = numBytes - 8;
				udpLen -= 8; // subtract off header bits
				xil_printf("UDP Data: %d bytes 0x", udpLen);
				for(int i=0; i<udpLen; i++) {
					XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA, 0x00}, RdBuf, 2);
					//                xil_printf("%02x", RdBuf[1]);
				}
				xil_printf("\r\n");
				numBytes = numBytes - udpLen;
			}
		 }
		 else if(type == 0x86DD) { // the payload is an IPv6

		 }else {
	         xil_printf("Unkown Eth Protocol: %d\r\n", ipProto);
	     }
	}


    xil_printf("%d Bytes left\r\n", numBytes);
    xil_printf("Data: 0x");

    for(int i=numBytes; i>4; i--) {
        xil_printf("%02x", net100ReadByte(SpiInstancePtr, RRXDATA));
    	numBytes -= 1;
    }
    xil_printf("\r\n");

    xil_printf(" *** Ether CRC ***\r\n");
    XSpi_Transfer(SpiInstancePtr, (unsigned char []){RRXDATA}, RdBuf, 5);
    xil_printf("%02x%02x%02x%02x\r\n", RdBuf[1], RdBuf[2], RdBuf[3], RdBuf[4]);

	numBytes -= 4; // subtract off the Ehternet CRC

    xil_printf("left numBytes: 0x%04x\r\n",numBytes);



	return newTail;



}

unsigned int sendPkt(XSpi *SpiInstancePtr, unsigned char *dataAddr, int numBytes) {
	/*
	1. Initialize the MAC as described in Section 8.6
	?MAC Initialization?. Most applications should
	leave PADCFG<3:0> and TXCRCEN set to their
	default values, which enables automatic padding
	and CRC generation. For automatic insertion of
	the source MAC address during transmission, set
	the TXMAC bit to ?1?.
	*/

	//Inputs are Destination Address, Source Address, Protocol, Data.
	//MAC insertion desabled (TXMAC = 0) (ECON2<13>).
	bankSelect(SpiInstancePtr, BANK3);
	net100Writer(SpiInstancePtr,(unsigned char []){BFC+ECON2H, 0x20}, 2);
	//Automatic padding no (PADCFG<2:0> = 000) (MACON2<7:5>)
	bankSelect(SpiInstancePtr, BANK2);
	net100Writer(SpiInstancePtr,(unsigned char []){BFC+MACON2L, 0xA0}, 2);
	//CRC generation enabled (TXCRCEN = 1)  (MACON2<4>)
	net100Writer(SpiInstancePtr,(unsigned char []){BFS+MACON2L, 0x10}, 2);

	/*
	2. If desired, enable the transmit done and/or
	transmit abort interrupts by setting TXIE and/or
	TXABTIE (EIE<3:2>). Clear TXIF and TXABTIF
	(EIR<3:2>) if they are currently set. To generate
	the interrupt, also set INTIE (EIE<15>).
	*/
	//3. Copy the packet to the SRAM buffer.

	// Set EGPRDPT to desired location (Where to start to read data from rcvbuff)

	unsigned int writePtr = net100ReadWord(SpiInstancePtr, WGPWRPT); //save pkt to send current address from general buffer

    unsigned int valW;


	do {
		net100WriteWord(SpiInstancePtr, WGPRDPT, dataAddr-(6+6+2));//config data addr from rcvBuff
        valW = net100ReadWord(SpiInstancePtr, RGPRDPT);
        xil_printf("RGPRDPT: 0x%04x\r\n", valW);
	} while(valW != dataAddr-(6+6+2));

	//copy eth dest addr
	//xil_printf("pointer: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, RGPWRPT));

	//net100Writer(SpiInstancePtr,(unsigned char []){WGPDATA, dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]}, 8); //it doesn't work with 7...
	//xil_printf("pointer: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, RGPWRPT));

	/*
 	// to test bufer push
	//net100WriteWord(SpiInstancePtr, WUDARDPT, writePtr);
	for(int i=0; i<6; i++) {
		xil_printf("pointer: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, RUDARDPT));
		xil_printf("mac: 0x%02x\r\n", net100ReadByte(SpiInstancePtr, RUDADATA));
	}
	 */

	//copy proto type
	//XSpi_Transfer(SpiInstancePtr, (unsigned char []){WGPDATA, 0x08}, NULL, 3);
	//XSpi_Transfer(SpiInstancePtr, (unsigned char []){WGPDATA, 0x06}, NULL, 3);
	//copy data
	int val = 0;
    for(int i=numBytes; i>0; i--) {
        XSpi_Transfer(SpiInstancePtr, (unsigned char []){WGPDATA, net100ReadByte(SpiInstancePtr, RGPDATA)}, NULL, 3); //2 is not working
    }

	//4. Program ETXST to the start address of the 	packet. (the one copied in the transmit buff???)
	bankSelect(SpiInstancePtr, BANK0);
    net100WriteWord(SpiInstancePtr, WCR+ETXSTL, writePtr);

	//5. Program ETXLEN with the length of data copied to the memory.
	 XSpi_Transfer(SpiInstancePtr, (unsigned char []){WCR+ETXLENL, numBytes, 0x00}, NULL, 3);


	//6. Set the TXRTS bit to initiate transmission. (ECON1<1>)
	bankSelect(SpiInstancePtr, BANK0);
	net100Writer(SpiInstancePtr,(unsigned char []){BFS+ECON1L, 0x02}, 3);


	//7. Wait for the hardware to clear TXRTS and trigger a transmit interrupt, indicating transmission has completed.
	do {
		val = net100ReadByte(SpiInstancePtr, ECON1L);
	    //xil_printf("ECON1L: 0x%04x\r\n", val);
	} while((val&0x02) == 0x02);

	//8. Read the ETXSTAT register for status information as described in the next section.
    xil_printf("ETXSTATL: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, ETXSTATL));
    //xil_printf("ETXWIREL: 0x%04x\r\n", net100ReadWord(SpiInstancePtr, ETXWIREL));



	/*
		The transmit function does not modify the ETXST
		Pointer or ETXLEN data length after the operation
		completes. To send another packet, the Start Pointer
		must be manually moved to the location of the next
		packet and the transmit length must be updated. If
		desired, the application can retransmit the last packet
		by setting TXRTS again without modifying ETXST or
		ETXLEN

		eth.src == 80:d8:1a:39:ab:c7
		70:8b:cd:11:5a:62
	 */
}







