/*
 * @brief This file implements an USB composite device:
 * IBDAP-CMSIS-DAP HID interface and a CDC-UART interface
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * Copyricht(C) Armstart, 2015
 * Copyright(C) Pablo Ridolfi, 2016
 * All rights reserved.
 *
 */

#include "board.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "hid.h"
#include "cdc_uart.h"
#include "DAP_config.h"
#include "DAP.h"

#define TICKRATE_100msec (10)
#define TICKRATE_10msec  (100)
#define TICKRATE_1msec 	 (1000)

#define TICKRATE 		 TICKRATE_1msec

// Connected LED RED          PIN 21: PIO0_11
#define LED_ERROR_PORT         0
#define LED_ERROR_BIT          11
#define IOCON_ERROR_REG        (LPC_IOCON->PIO0[11])

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

 typedef struct _IBDAP_HND_T {
 	volatile uint8_t usb_outs_end_idx;
 	volatile uint8_t usb_outs_start_idx;
 	uint8_t usb_outs[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
 	volatile uint8_t usb_ins_end_idx;
 	volatile uint8_t usb_ins_start_idx;
 	volatile uint8_t usb_ins_busy;
 	uint8_t usb_ins[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
 }IBDAP_HND_T;

USBD_HANDLE_T g_hUsb;
const  USBD_API_T *g_pUsbApi;
static volatile uint32_t sys_time = 0;

// HID callbacks and buffer
IBDAP_HND_T* ibdap = 0;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initialize pin and clocks for USB port */
static void usb_pin_clk_init(void)
{
	/* enable USB main clock */
	Chip_Clock_SetUSBClockSource(SYSCTL_USBCLKSRC_PLLOUT, 1);
	/* Enable AHB clock to the USB block and USB RAM. */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USBRAM);
	/* power UP USB Phy */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_USBPAD_PD);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from USB0
 * @return	Nothing
 */
void USB_IRQHandler(void)
{
	uint32_t *addr = (uint32_t *) LPC_USB->EPLISTSTART;

	/*	WORKAROUND for artf32289 ROM driver BUG:
	    As part of USB specification the device should respond
	    with STALL condition for any unsupported setup packet. The host will send
	    new setup packet/request on seeing STALL condition for EP0 instead of sending
	    a clear STALL request. Current driver in ROM doesn't clear the STALL
	    condition on new setup packet which should be fixed.
	 */
	if ( LPC_USB->DEVCMDSTAT & _BIT(8) ) {	/* if setup packet is received */
		addr[0] &= ~(_BIT(29));	/* clear EP0_OUT stall */
		addr[2] &= ~(_BIT(29));	/* clear EP0_IN stall */
	}
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type.  */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}

void indicator_code (int code);

void init_error_led () {
	IOCON_ERROR_REG = 1 | 1 << 4 | 1 << 7;
	LPC_GPIO->DIR[LED_ERROR_PORT] |= 1 << LED_ERROR_BIT;
	LPC_GPIO->SET[LED_ERROR_PORT] = 1 << LED_ERROR_BIT;
}

INLINE void LED_ERROR_ON () {
	LPC_GPIO->CLR[LED_ERROR_PORT] = 1 << LED_ERROR_BIT;
}

INLINE void LED_ERROR_OFF () {
	LPC_GPIO->SET[LED_ERROR_PORT] = 1 << LED_ERROR_BIT;
}

void SysTick_Handler(void)
{
	sys_time++;
}

void sleep_ms(int ms)
{
	int e = sys_time * (1000/TICKRATE) + ms;
	while ((sys_time * (1000/TICKRATE)) <= e) {
		__WFI ();
	}
}

ErrorCode_t DAP_GetReport_Callback( USBD_HANDLE_T hHid, USB_SETUP_PACKET* pSetup, uint8_t** pBuffer, uint16_t* length) {
	return LPC_OK;
}

ErrorCode_t DAP_SetReport_Callback( USBD_HANDLE_T hHid, USB_SETUP_PACKET* pSetup, uint8_t** pBuffer, uint16_t length) {
	return LPC_OK;
}

ErrorCode_t DAP_EpInOut_Hdlr_Callback (USBD_HANDLE_T hUsb, void* data, uint32_t event) {
	USB_HID_CTRL_T *pHidCtrl = (USB_HID_CTRL_T *) data;
	switch (event) {
	case USB_EVT_IN:
		if (ibdap->usb_ins_start_idx == ibdap->usb_ins_end_idx) {
			ibdap->usb_ins_busy = 0;
			break;
		}
		USBD_API->hw->WriteEP(hUsb, pHidCtrl->epin_adr, ibdap->usb_ins[ibdap->usb_ins_start_idx], DAP_PACKET_SIZE);
		ibdap->usb_ins_start_idx = (ibdap->usb_ins_start_idx+1) % DAP_PACKET_COUNT;
		//LED_RUNNING_OUT (1);
		//LED_CONNECTED_OUT (0);
		break;
	case USB_EVT_OUT:
		USBD_API->hw->ReadEP(hUsb, pHidCtrl->epout_adr, ibdap->usb_outs[ibdap->usb_outs_end_idx]);
		ibdap->usb_outs_end_idx = (ibdap->usb_outs_end_idx+1) % DAP_PACKET_COUNT;
		//LED_CONNECTED_OUT (1);
		//LED_RUNNING_OUT (0);
		break;
	}
	return LPC_OK;
}

void suspend () {
	while (1) {
		LED_ERROR_ON ();
		sleep_ms (1000);
		LED_ERROR_OFF ();
		sleep_ms (1000);
	}
}

void error_code (int code) {
	LED_ERROR_ON ();
	while (1) {
		if (code & 0x01 && !(code & 0x02 )) { // code == 1
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (0);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
		} else if (!(code & 0x01) && (code & 0x02)) { // code == 2
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (1);
			sleep_ms (1000);
			LED_RUNNING_OUT (0);
		} else if ((code & 0x01) == 3) {
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (1);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (0);
		} else {
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (0);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (1);
		}

		sleep_ms (1000);
	}
}

void indicator_code (int code) {
	LED_ERROR_OFF ();
	while (1) {
		if (code & 0x01 && !(code & 0x02 )) { // code == 1
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (0);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
		} else if (!(code & 0x01) && (code & 0x02)) { // code == 2
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (1);
			sleep_ms (1000);
			LED_RUNNING_OUT (0);
		} else if ((code & 0x01) == 3) {
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (1);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (0);
		} else {
			LED_CONNECTED_OUT (1);
			LED_RUNNING_OUT (0);
			sleep_ms (1000);
			LED_CONNECTED_OUT (0);
			LED_RUNNING_OUT (1);
		}

		sleep_ms (1000);
	}
}

/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;

	/* Initialize board and chip */
	Board_Init();
	SystemCoreClockUpdate();

	SysTick_Config(SystemCoreClock / TICKRATE);

	init_error_led ();

	DAP_Setup ();

	/* enable clocks and pinmux */
	usb_pin_clk_init();

	/* initialize USBD ROM API pointer. */
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB0_BASE;
	/*	WORKAROUND for artf44835 ROM driver BUG:
	    Code clearing STALL bits in endpoint reset routine corrupts memory area
	    next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
	    EP2_IN are used we need to specify 3 here. But as a workaround for this
	    issue specify 4. So that extra EPs control structure acts as padding buffer
	    to avoid data corruption. Corruption of padding memory doesn’t affect the
	    stack/program behaviour.
	 */
	usb_param.max_num_ep = 5 + 1;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *) USB_StringDescriptor;

	/* Note, to pass USBCV test full-speed only devices should have both
	 * descriptor arrays point to same location and device_qualifier set
	 * to 0.
	 */
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = 0;

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {

		/*	WORKAROUND for artf32219 ROM driver BUG:
		    The mem_base parameter part of USB_param structure returned
		    by Init() routine is not accurate causing memory allocation issues for
		    further components.
		 */
		usb_param.mem_base = USB_STACK_MEM_BASE + (USB_STACK_MEM_SIZE - usb_param.mem_size);

		ret = init_usb_hid (&usb_param,
		    		&DAP_GetReport_Callback, &DAP_SetReport_Callback,
					&DAP_EpInOut_Hdlr_Callback, &DAP_EpInOut_Hdlr_Callback,
					(uint8_t**)&ibdap, sizeof (IBDAP_HND_T));

		if (ret == LPC_OK) {
			/* Init UCOM interface */
			ret = UCOM_init(g_hUsb, &desc, &usb_param);
			if (ret == LPC_OK) {
				/*  enable USB interrupts */
				NVIC_EnableIRQ(USB0_IRQn);
				/* now connect */
				USBD_API->hw->Connect(g_hUsb, 1);
			}
		}
	}

	while (1) {
		if (ibdap->usb_outs_start_idx == ibdap->usb_outs_end_idx) {
    		__WFI ();
    		continue;
    	} else {
    		LED_CONNECTED_OUT (1);
    	}
    	DAP_ProcessCommand (ibdap->usb_outs[ibdap->usb_outs_start_idx], ibdap->usb_ins[ibdap->usb_ins_end_idx]);
    	ibdap->usb_outs_start_idx = (ibdap->usb_outs_start_idx+1) % DAP_PACKET_COUNT;
    	ibdap->usb_ins_end_idx = (ibdap->usb_ins_end_idx+1) % DAP_PACKET_COUNT;
    	if (!ibdap->usb_ins_busy) { // kickstart
    		ibdap->usb_ins_busy = 1;
    		uint8_t idx = ibdap->usb_ins_start_idx;
    		ibdap->usb_ins_start_idx = (ibdap->usb_ins_start_idx+1) % DAP_PACKET_COUNT;
    		USBD_API->hw->WriteEP(g_hUsb, HID_EP_IN, ibdap->usb_ins[idx], DAP_PACKET_SIZE);
    	}

	}
}
