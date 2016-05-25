/*
 * usb_driver.c
 *
 *  Created on: Jun 18, 2015
 *      Author: yliu
 */

#include <stdint.h>
#include <string.h>

#include "board.h"

#include "hid.h"

#define PDRUNCFGUSEMASK 0x0000E800
#define PDRUNCFGMASKTMP 0x000005FF

extern USBD_HANDLE_T g_hUsb;

const  USBD_API_T *g_pUsbApi;

extern const uint8_t hid_report_size;
extern const uint8_t HID_ReportDescriptor[];

int init_usb_hid (USBD_API_INIT_PARAM_T *usb_param,
		HID_GetReport_Func_T getreport_fun, HID_SetReport_Func_T setreport_fun,
		HID_EpIn_Hdlr_Func_T epin_hdlr_fun, HID_EpOut_Hdlr_Func_T epout_hdlr_fun,
		uint8_t** report_saddr, int report_size) {
	USBD_HID_INIT_PARAM_T hid_param;
	USB_HID_REPORT_T reports_data[1];
	ErrorCode_t ret = LPC_OK;

	memset((void *) &hid_param, 0, sizeof(USBD_HID_INIT_PARAM_T));
	hid_param.max_reports = 1;

	/* Init reports_data */
	reports_data[0].len = hid_report_size;
	reports_data[0].idle_time = 0;
	reports_data[0].desc = (uint8_t *) &HID_ReportDescriptor[0];

	USB_INTERFACE_DESCRIPTOR *pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) &USB_FsConfigDescriptor[sizeof(USB_CONFIGURATION_DESCRIPTOR)];

	if ((pIntfDesc == 0) || (pIntfDesc->bInterfaceClass != USB_DEVICE_CLASS_HUMAN_INTERFACE)) {
		return -1;
	}

	hid_param.mem_base = usb_param->mem_base;
	hid_param.mem_size = usb_param->mem_size;
	hid_param.intf_desc = (uint8_t *) pIntfDesc;
	/* user defined functions */
	hid_param.HID_GetReport = getreport_fun;
	hid_param.HID_SetReport = setreport_fun;
	hid_param.HID_EpIn_Hdlr  = epin_hdlr_fun;
	hid_param.HID_EpOut_Hdlr = epout_hdlr_fun;
	hid_param.report_data  = reports_data;

	ret = USBD_API->hid->init(g_hUsb, &hid_param);
	if (ret != LPC_OK) return -2;
	/* allocate USB accessable memory space for report data */
	*report_saddr =  (uint8_t *) hid_param.mem_base;
	memset ((void *)hid_param.mem_base, 0, report_size);
	hid_param.mem_base += report_size;
	hid_param.mem_size -= report_size;
	/* update memory variables */
	usb_param->mem_base = hid_param.mem_base;
	usb_param->mem_size = hid_param.mem_size;
	return 0;
}
