/*
 * @brief This file contains USB composite class descriptors.
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * Copyright(C) Armstart, 2015
 * Copyright(C) Pablo Ridolfi, 2016
 * All rights reserved.
 *
 */

#include "app_usbd_cfg.h"
#include "DAP_config.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

ALIGNED(4) const uint8_t HID_ReportDescriptor[] = {
	HID_UsagePageVendor(0x00),
	HID_Usage(0x01),
	HID_Collection(HID_Application),
	HID_LogicalMin(0),	/* value range: 0 - 0xFF */
	HID_LogicalMaxS(0xFF),
	HID_ReportSize(8),	/* 8 bits */
	HID_ReportCount(DAP_PACKET_SIZE),
	HID_Usage(0x01),
	HID_Input(HID_Data | HID_Variable | HID_Absolute),
	HID_ReportCount(DAP_PACKET_SIZE),
	HID_Usage(0x01),
	HID_Output(HID_Data | HID_Variable | HID_Absolute),
	HID_ReportCount(1),
	HID_Usage(0x01),
	HID_Feature(HID_Data | HID_Variable | HID_Absolute),
	HID_EndCollection,
};

const uint8_t hid_report_size = sizeof (HID_ReportDescriptor);

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4) const uint8_t USB_DeviceDescriptor[] = {
	USB_DEVICE_DESC_SIZE,			/* bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0200),					/* bcdUSB : 2.00*/
	0x00,							/* bDeviceClass */
	0x00,							/* bDeviceSubClass */
	0x00,							/* bDeviceProtocol */
	USB_MAX_PACKET0,				/* bMaxPacketSize0 */
	WBVAL(0x1FC9),					/* idVendor */
	WBVAL(0x0081),					/* idProduct */
	WBVAL(0x0100),					/* bcdDevice : 1.00 */
	0x01,							/* iManufacturer */
	0x02,							/* iProduct */
	0x03,							/* iSerialNumber */
	0x01							/* bNumConfigurations */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4) uint8_t USB_FsConfigDescriptor[] = {
	/* Configuration 1 */
	USB_CONFIGURATION_DESC_SIZE,			/* bLength */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(									/* wTotalLength */
		USB_CONFIGURATION_DESC_SIZE     +
		/* HID class related descriptors */
		USB_INTERFACE_DESC_SIZE         +
		HID_DESC_SIZE                   +
		USB_ENDPOINT_DESC_SIZE			+
		USB_ENDPOINT_DESC_SIZE          +
		/* CDC class related descriptors */
		USB_INTERFACE_ASSOC_DESC_SIZE   +	/* interface association descriptor */
		USB_INTERFACE_DESC_SIZE         +	/* communication control interface */
		0x0013                          +	/* CDC functions */
		1 * USB_ENDPOINT_DESC_SIZE      +	/* interrupt endpoint */
		USB_INTERFACE_DESC_SIZE         +	/* communication data interface */
		2 * USB_ENDPOINT_DESC_SIZE      +	/* bulk endpoints */
		0
		),
	0x03,							/* bNumInterfaces */
	0x01,							/* bConfigurationValue */
	0x00,							/* iConfiguration */
	USB_CONFIG_SELF_POWERED,		/* bmAttributes */
	USB_CONFIG_POWER_MA(100),		/* bMaxPower */

	/* Interface 0, Alternate Setting 0, HID Class */
	USB_INTERFACE_DESC_SIZE,		/* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE,	/* bDescriptorType */
	USB_HID_IF_NUM,					/* bInterfaceNumber */
	0x00,							/* bAlternateSetting */
	0x02,							/* bNumEndpoints */
	USB_DEVICE_CLASS_HUMAN_INTERFACE,	/* bInterfaceClass */
	HID_SUBCLASS_NONE,				/* bInterfaceSubClass */
	HID_PROTOCOL_NONE,				/* bInterfaceProtocol */
	0x04,							/* iInterface */
	/* HID Class Descriptor */
	/* HID_DESC_OFFSET = 0x0012 */
	HID_DESC_SIZE,					/* bLength */
	HID_HID_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0111),					/* bcdHID : 1.11*/
	0x00,							/* bCountryCode */
	0x01,							/* bNumDescriptors */
	HID_REPORT_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(sizeof(HID_ReportDescriptor)),	/* wDescriptorLength */
	/* Endpoint, HID Interrupt In */
	USB_ENDPOINT_DESC_SIZE,			/* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType */
	HID_EP_IN,						/* bEndpointAddress */
	USB_ENDPOINT_TYPE_INTERRUPT,	/* bmAttributes */
	WBVAL(64),					/* wMaxPacketSize */
	0x01,		/* 1ms */           /* bInterval */
	/* Endpoint, HID Interrupt Out */
	USB_ENDPOINT_DESC_SIZE,			/* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType */
	HID_EP_OUT,						/* bEndpointAddress */
	USB_ENDPOINT_TYPE_INTERRUPT,	/* bmAttributes */
	WBVAL(64),					/* wMaxPacketSize */
	0x01,							/* bInterval: 1ms */

	/* Interface association descriptor IAD*/
	USB_INTERFACE_ASSOC_DESC_SIZE,		/* bLength */
	USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,	/* bDescriptorType */
	USB_CDC_CIF_NUM,					/* bFirstInterface */
	0x02,								/* bInterfaceCount */
	CDC_COMMUNICATION_INTERFACE_CLASS,	/* bFunctionClass */
	CDC_ABSTRACT_CONTROL_MODEL,			/* bFunctionSubClass */
	0x00,								/* bFunctionProtocol */
	0x05,								/* iFunction */

	/* Interface 1, Alternate Setting 0, Communication class interface descriptor */
	USB_INTERFACE_DESC_SIZE,			/* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	USB_CDC_CIF_NUM,					/* bInterfaceNumber: Number of Interface */
	0x00,								/* bAlternateSetting: Alternate setting */
	0x01,								/* bNumEndpoints: One endpoint used */
	CDC_COMMUNICATION_INTERFACE_CLASS,	/* bInterfaceClass: Communication Interface Class */
	CDC_ABSTRACT_CONTROL_MODEL,			/* bInterfaceSubClass: Abstract Control Model */
	0x00,								/* bInterfaceProtocol: no protocol used */
	0x05,								/* iInterface: */
	/* Header Functional Descriptor*/
	0x05,								/* bLength: CDC header Descriptor size */
	CDC_CS_INTERFACE,					/* bDescriptorType: CS_INTERFACE */
	CDC_HEADER,							/* bDescriptorSubtype: Header Func Desc */
	WBVAL(CDC_V1_10),					/* bcdCDC 1.10 */
	/* Call Management Functional Descriptor*/
	0x05,								/* bFunctionLength */
	CDC_CS_INTERFACE,					/* bDescriptorType: CS_INTERFACE */
	CDC_CALL_MANAGEMENT,				/* bDescriptorSubtype: Call Management Func Desc */
	0x01,								/* bmCapabilities: device handles call management */
	USB_CDC_DIF_NUM,					/* bDataInterface: CDC data IF ID */
	/* Abstract Control Management Functional Descriptor*/
	0x04,								/* bFunctionLength */
	CDC_CS_INTERFACE,					/* bDescriptorType: CS_INTERFACE */
	CDC_ABSTRACT_CONTROL_MANAGEMENT,	/* bDescriptorSubtype: Abstract Control Management desc */
	0x02,								/* bmCapabilities: SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE supported */
	/* Union Functional Descriptor*/
	0x05,								/* bFunctionLength */
	CDC_CS_INTERFACE,					/* bDescriptorType: CS_INTERFACE */
	CDC_UNION,							/* bDescriptorSubtype: Union func desc */
	USB_CDC_CIF_NUM,					/* bMasterInterface: Communication class interface is master */
	USB_CDC_DIF_NUM,					/* bSlaveInterface0: Data class interface is slave 0 */
	/* Endpoint 1 Descriptor*/
	USB_ENDPOINT_DESC_SIZE,				/* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType */
	USB_CDC_INT_EP,						/* bEndpointAddress */
	USB_ENDPOINT_TYPE_INTERRUPT,		/* bmAttributes */
	WBVAL(0x0010),						/* wMaxPacketSize */
	0x02,			/* 2ms */           /* bInterval */

	/* Interface 2, Alternate Setting 0, Data class interface descriptor*/
	USB_INTERFACE_DESC_SIZE,			/* bLength */
	USB_INTERFACE_DESCRIPTOR_TYPE,		/* bDescriptorType */
	USB_CDC_DIF_NUM,					/* bInterfaceNumber: Number of Interface */
	0x00,								/* bAlternateSetting: no alternate setting */
	0x02,								/* bNumEndpoints: two endpoints used */
	CDC_DATA_INTERFACE_CLASS,			/* bInterfaceClass: Data Interface Class */
	0x00,								/* bInterfaceSubClass: no subclass available */
	0x00,								/* bInterfaceProtocol: no protocol used */
	0x05,								/* iInterface: */
	/* Endpoint, EP Bulk Out */
	USB_ENDPOINT_DESC_SIZE,				/* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType */
	USB_CDC_OUT_EP,						/* bEndpointAddress */
	USB_ENDPOINT_TYPE_BULK,				/* bmAttributes */
	WBVAL(USB_FS_MAX_BULK_PACKET),		/* wMaxPacketSize */
	0x00,								/* bInterval: ignore for Bulk transfer */
	/* Endpoint, EP Bulk In */
	USB_ENDPOINT_DESC_SIZE,				/* bLength */
	USB_ENDPOINT_DESCRIPTOR_TYPE,		/* bDescriptorType */
	USB_CDC_IN_EP,						/* bEndpointAddress */
	USB_ENDPOINT_TYPE_BULK,				/* bmAttributes */
	WBVAL(64),							/* wMaxPacketSize */
	0x00,								/* bInterval: ignore for Bulk transfer */

	/* Terminator */
	0								/* bLength */
};

/**
 * USB String Descriptor (optional)
 */
const uint8_t USB_StringDescriptor[] = {
	/* Index 0x00: LANGID Codes */
	0x04,							/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	WBVAL(0x0409),					/* wLANGID : US English*/
	/* Index 0x01: Manufacturer */
	(8 * 2 + 2),					/* bLength (8 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'A', 0,
	'R', 0,
	'M', 0,
	'S', 0,
	'T', 0,
	'A', 0,
	'R', 0,
	'T', 0,
	/* Index 0x02: Product */
	(24 * 2 + 2),					/* bLength (24 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'I', 0,
	'B', 0,
	'D', 0,
	'A', 0,
	'P', 0,
	'-', 0,
	'L', 0,
	'P', 0,
	'C', 0,
	'1', 0,
	'1', 0,
	'U', 0,
	'3', 0,
	'5', 0,
	' ', 0,
	'C', 0,
	'M', 0,
	'S', 0,
	'I', 0,
	'S', 0,
	'-', 0,
	'D', 0,
	'A', 0,
	'P', 0,
	/* Index 0x03: Serial Number */
	(11 * 2 + 2),					/* bLength (13 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'I', 0,
	'B', 0,
	'D', 0,
	'A', 0,
	'P', 0,
	'2', 0,
	'0', 0,
	'1', 0,
	'5', 0,
	'0', 0,
	'7', 0,
	/* Index 0x04: Interface 0, Alternate Setting 0 */
	(3 * 2 + 2),					/* bLength (3 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,		/* bDescriptorType */
	'H', 0,
	'I', 0,
	'D', 0,
	/* Index 0x05: Interface 1, Alternate Setting 0 */
	( 4 * 2 + 2),						/* bLength (4 Char + Type + lenght) */
	USB_STRING_DESCRIPTOR_TYPE,			/* bDescriptorType */
	'V', 0,
	'C', 0,
	'O', 0,
	'M', 0,
};
