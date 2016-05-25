/*
 * @brief Configuration file needed for USB ROM stack based applications.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * Copyright(C) Pablo Ridolfi, 2016
 * All rights reserved.
 *
 */
#include "lpc_types.h"
#include "error.h"
#include "usbd_rom_api.h"

#ifndef __APP_USB_CFG_H_
#define __APP_USB_CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** @ingroup EXAMPLES_USBDROM_11UXX_COMPOSITE
 * @{
 */

/* HID In/Out Endpoint Address */
#define HID_EP_IN                           0x81
#define HID_EP_OUT                          0x01

#define USB_HID_IF_NUM                      0

/* Manifest constants defining interface numbers and endpoints used by a
   CDC class interfaces in this application.
 */
#define USB_CDC_CIF_NUM                     1
#define USB_CDC_DIF_NUM                     2
#define USB_CDC_IN_EP                       0x82
#define USB_CDC_OUT_EP                      0x02
#define USB_CDC_INT_EP                      0x83

/* The following manifest constants are used to define this memory area to be used
   by USBD ROM stack.
 */
#define USB_STACK_MEM_BASE      0x10001000
#define USB_STACK_MEM_SIZE      0x1000

/* Manifest constants used by USBD ROM stack. These values SHOULD NOT BE CHANGED
   for advance features which require usage of USB_CORE_CTRL_T structure.
   Since these are the values used for compiling USB stack.
 */
#define USB_MAX_IF_NUM          8		/*!< Max interface number used for building USBDL_Lib. DON'T CHANGE. */
#define USB_MAX_EP_NUM          5		/*!< Max number of EP used for building USBD ROM. DON'T CHANGE. */
#define USB_MAX_PACKET0         64		/*!< Max EP0 packet size used for building USBD ROM. DON'T CHANGE. */
#define USB_FS_MAX_BULK_PACKET  64		/*!< MAXP for FS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_HS_MAX_BULK_PACKET  512		/*!< MAXP for HS bulk EPs used for building USBD ROM. DON'T CHANGE. */
#define USB_DFU_XFER_SIZE       2048	/*!< Max DFU transfer size used for building USBD ROM. DON'T CHANGE. */

/* USB descriptor arrays defined *_desc.c file */
extern const uint8_t USB_DeviceDescriptor[];
extern uint8_t USB_HsConfigDescriptor[];
extern uint8_t USB_FsConfigDescriptor[];
extern const uint8_t USB_StringDescriptor[];
extern const uint8_t USB_DeviceQualifier[];

/**
 * @brief	Find the address of interface descriptor for given class type.
 * @param	pDesc		: Pointer to configuration descriptor in which the desired class
 *			interface descriptor to be found.
 * @param	intfClass	: Interface class type to be searched.
 * @return	If found returns the address of requested interface else returns NULL.
 */
extern USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __APP_USB_CFG_H_ */
