/*
   Copyright (c) 2015, Arduino LLC
   Original code (pre-library): Copyright (c) 2011, Peter Barrett

   Permission to use, copy, modify, and/or distribute this software for
   any purpose with or without fee is hereby granted, provided that the
   above copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
   BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
   OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
   ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
   SOFTWARE.
 */

#define MS_OS_20_SET_HEADER_DESCRIPTOR 0x00
#define MS_OS_20_SUBSET_HEADER_CONFIGURATION 0x01
#define MS_OS_20_SUBSET_HEADER_FUNCTION 0x02
#define MS_OS_20_FEATURE_COMPATIBLE_ID 0x03
#define MS_OS_20_FEATURE_REG_PROPERTY 0x04
#define MS_OS_20_DESCRIPTOR_LENGTH 0xb2

#include "WebUSB.h"

#ifdef ARDUINO_ARCH_SAMD

#define USB_SendControl				USBDevice.sendControl
#define USB_RecvControl				USBDevice.recvControl
#define USB_Available				USBDevice.available
#define USB_Recv					USBDevice.recv
#define USB_Send					USBDevice.send
#define USB_SendSpace(ep)			(EPX_SIZE - 1)
#define USB_Flush					USBDevice.flush

#define TRANSFER_PGM 0

#define EP_TYPE_BULK_IN_WEBUSB		USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_IN(0);
#define EP_TYPE_BULK_OUT_WEBUSB		USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_OUT(0);

#else

#define EP_TYPE_BULK_IN_WEBUSB		EP_TYPE_BULK_IN
#define EP_TYPE_BULK_OUT_WEBUSB		EP_TYPE_BULK_OUT

#endif

const uint8_t BOS_DESCRIPTOR_PREFIX[] PROGMEM = {
0x05,  // Length
0x0F,  // Binary Object Store descriptor
0x39, 0x00,  // Total length
0x02,  // Number of device capabilities

// WebUSB Platform Capability descriptor (bVendorCode == 0x01).
0x18,  // Length
0x10,  // Device Capability descriptor
0x05,  // Platform Capability descriptor
0x00,  // Reserved
0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47,
0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65,  // WebUSB GUID
0x00, 0x01,  // Version 1.0
0x01,  // Vendor request code
};

// Landing page (1 byte) sent in the middle.

const uint8_t BOS_DESCRIPTOR_SUFFIX[] PROGMEM {
// Microsoft OS 2.0 Platform Capability Descriptor (MS_VendorCode == 0x02)
0x1C,  // Length
0x10,  // Device Capability descriptor
0x05,  // Platform Capability descriptor
0x00,  // Reserved
0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C,
0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F,  // MS OS 2.0 GUID
0x00, 0x00, 0x03, 0x06,  // Windows version
MS_OS_20_DESCRIPTOR_LENGTH, 0x00,  // Descriptor set length
0x02,  // Vendor request code
0x00   // Alternate enumeration code
};

const uint8_t MS_OS_20_DESCRIPTOR_PREFIX[] PROGMEM = {
// Microsoft OS 2.0 descriptor set header (table 10)
0x0A, 0x00,  // Descriptor size (10 bytes)
MS_OS_20_SET_HEADER_DESCRIPTOR, 0x00,  // MS OS 2.0 descriptor set header
0x00, 0x00, 0x03, 0x06,  // Windows version (8.1) (0x06030000)
MS_OS_20_DESCRIPTOR_LENGTH, 0x00,  // Size, MS OS 2.0 descriptor set

// Microsoft OS 2.0 configuration subset header
0x08, 0x00,  // Descriptor size (8 bytes)
 MS_OS_20_SUBSET_HEADER_CONFIGURATION, 0x00,  // MS OS 2.0 configuration subset header
0x00,        // bConfigurationValue
0x00,        // Reserved
0xA8, 0x00,  // Size, MS OS 2.0 configuration subset

// Microsoft OS 2.0 function subset header
0x08, 0x00,  // Descriptor size (8 bytes)
 MS_OS_20_SUBSET_HEADER_FUNCTION, 0x00,  // MS OS 2.0 function subset header
};

// First interface number (1 byte) sent here.

const uint8_t MS_OS_20_DESCRIPTOR_SUFFIX[] PROGMEM = {
0x00,        // Reserved
0xA0, 0x00,  // Size, MS OS 2.0 function subset

// Microsoft OS 2.0 compatible ID descriptor (table 13)
0x14, 0x00,  // wLength
MS_OS_20_FEATURE_COMPATIBLE_ID, 0x00,  // MS_OS_20_FEATURE_COMPATIBLE_ID
'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


const uint8_t MS_OS_20_CUSTOM_PROPERTY[] PROGMEM = {
0x84, 0x00,   //wLength: 
MS_OS_20_FEATURE_REG_PROPERTY, 0x00,   // wDescriptorType: MS_OS_20_FEATURE_REG_PROPERTY: 0x04 (Table 9)
0x07, 0x00,   //wPropertyDataType: REG_MULTI_SZ (Table 15)
0x2a, 0x00,   //wPropertyNameLength: 
//bPropertyName: “DeviceInterfaceGUID”
'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 
'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 
0x00, 0x00,
0x50, 0x00,   // wPropertyDataLength
//bPropertyData: “{975F44D9-0D08-43FD-8B3E-127CA8AFFF9D}”.
'{', 0x00, '9', 0x00, '7', 0x00, '5', 0x00, 'F', 0x00, '4', 0x00, '4', 0x00, 'D', 0x00, '9', 0x00, '-', 0x00, 
'0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00, 
'8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00, 
'8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef struct
{
	uint32_t	dwDTERate;
	uint8_t	bCharFormat;
	uint8_t 	bParityType;
	uint8_t 	bDataBits;
	uint8_t	lineState;
} LineInfo;

static volatile LineInfo _usbLineInfo = { 57600, 0x00, 0x00, 0x00, 0x00 };
static volatile int32_t breakValue = -1;

int WebUSB::getInterface(uint8_t* interfaceCount)
{
	*interfaceCount += 1; // uses 1 interface
	WebUSBDescriptor webUSBInterface = {
		D_INTERFACE(pluggedInterface, 2, 0xff, 0, 0),
		D_ENDPOINT(USB_ENDPOINT_OUT(pluggedEndpoint),USB_ENDPOINT_TYPE_BULK,0x40,0),
		D_ENDPOINT(USB_ENDPOINT_IN (pluggedEndpoint+1),USB_ENDPOINT_TYPE_BULK,0x40,0)
	};
	return USB_SendControl(0, &webUSBInterface, sizeof(webUSBInterface));
}

int WebUSB::getDescriptor(USBSetup& setup)
{
	if (USB_BOS_DESCRIPTOR_TYPE == setup.wValueH)
	{
		if (setup.wValueL == 0 && setup.wIndex == 0) {
			if (USB_SendControl(TRANSFER_PGM, &BOS_DESCRIPTOR_PREFIX, sizeof(BOS_DESCRIPTOR_PREFIX)) < 0)
				return -1;
			uint8_t landingPage = landingPageUrl ? 1 : 0;
			if (USB_SendControl(0, &landingPage, 1) < 0)
				return -1;
			if (USB_SendControl(TRANSFER_PGM, &BOS_DESCRIPTOR_SUFFIX, sizeof(BOS_DESCRIPTOR_SUFFIX)) < 0)
				return -1;
			return sizeof(BOS_DESCRIPTOR_PREFIX) + 1 + sizeof(BOS_DESCRIPTOR_SUFFIX);
		}
	}
	return 0;
}

uint8_t WebUSB::getShortName(char* name)
{
	uint8_t len = strlen(shortName);
	memcpy(name, shortName, len);
	return len;
}

bool WebUSB::VendorControlRequest(USBSetup& setup)
{
	if (setup.bmRequestType == (REQUEST_DEVICETOHOST | REQUEST_VENDOR | REQUEST_DEVICE)) {
		if (setup.bRequest == 0x01 && setup.wIndex == WEBUSB_REQUEST_GET_URL && landingPageUrl)
		{
			if (setup.wValueL != 1)
				return false;
			uint8_t urlLength = strlen(landingPageUrl);
			uint8_t descriptorLength = urlLength + 3;
			if (USB_SendControl(0, &descriptorLength, 1) < 0)
				return false;
			uint8_t descriptorType = 3;
			if (USB_SendControl(0, &descriptorType, 1) < 0)
				return false;
			if (USB_SendControl(0, &landingPageScheme, 1) < 0)
				return false;
			return USB_SendControl(0, landingPageUrl, urlLength) >= 0;
		}
		else if (setup.bRequest == 0x02 && setup.wIndex == MS_OS_20_REQUEST_DESCRIPTOR)
		{
			if (USB_SendControl(TRANSFER_PGM, &MS_OS_20_DESCRIPTOR_PREFIX, sizeof(MS_OS_20_DESCRIPTOR_PREFIX)) < 0)
				return false;
			if (USB_SendControl(0, &pluggedInterface, 1) < 0)
				return false;
			if (USB_SendControl(TRANSFER_PGM, &MS_OS_20_DESCRIPTOR_SUFFIX, sizeof(MS_OS_20_DESCRIPTOR_SUFFIX)) < 0)
			        return false;
			if (USB_SendControl(TRANSFER_PGM, &MS_OS_20_CUSTOM_PROPERTY, sizeof(MS_OS_20_CUSTOM_PROPERTY)) < 0)
			        return false;
			return true;
		}
	}
	return false;
}


bool WebUSB::setup(USBSetup& setup)
{
	uint8_t r = setup.bRequest;
	uint8_t requestType = setup.bmRequestType;

	if (REQUEST_CLASS == (requestType & REQUEST_TYPE) && (pluggedInterface == setup.wIndex)) {
		if (REQUEST_DEVICETOHOST_CLASS_INTERFACE == requestType)
		{
			if (CDC_GET_LINE_CODING == r)
			{
				return USB_SendControl(0,(void*)&_usbLineInfo,7) >= 0;
			}
		}

		if (REQUEST_HOSTTODEVICE_CLASS_INTERFACE == requestType)
		{
			if (CDC_SEND_BREAK == r)
			{
				breakValue = ((uint16_t)setup.wValueH << 8) | setup.wValueL;
			}

			if (CDC_SET_LINE_CODING == r)
			{
				USB_RecvControl((void*)&_usbLineInfo,7);
			}

			if (CDC_SET_CONTROL_LINE_STATE == r)
			{
				_usbLineInfo.lineState = setup.wValueL;
			}
			return true;
		}
	} else if (REQUEST_VENDOR == (requestType & REQUEST_TYPE)) {
		return VendorControlRequest(setup);
	}
	return false;
}

WebUSB::WebUSB(uint8_t landingPageScheme, const char* landingPageUrl)
	: PluggableUSBModule(2, 1, epType),
	  landingPageScheme(landingPageScheme), landingPageUrl(landingPageUrl),
	  shortName(WEBUSB_SHORT_NAME)
{
	// one interface, 2 endpoints
	epType[0] = EP_TYPE_BULK_OUT_WEBUSB;
	epType[1] = EP_TYPE_BULK_IN_WEBUSB;
	PluggableUSB().plug(this);
}

void WebUSB::begin(unsigned long /* baud_count */)
{
	peek_buffer = -1;
}

void WebUSB::begin(unsigned long /* baud_count */, byte /* config */)
{
	peek_buffer = -1;
}

void WebUSB::end(void)
{
}

void WebUSB::setShortName(const char* name)
{
	shortName = name;
}

int WebUSB::available(void)
{
	if (peek_buffer >= 0) {
		return 1 + USB_Available(pluggedEndpoint);
	}
	return USB_Available(pluggedEndpoint);
}

int WebUSB::peek(void)
{
	if (peek_buffer < 0)
		peek_buffer = USB_Recv(pluggedEndpoint);
	return peek_buffer;
}

int WebUSB::read(void)
{
	if (peek_buffer >= 0) {
		int c = peek_buffer;
		peek_buffer = -1;
		return c;
	}
	return USB_Recv(pluggedEndpoint);
}

int WebUSB::availableForWrite(void)
{
	return USB_SendSpace(pluggedEndpoint+1);
}

void WebUSB::flush(void)
{
	USB_Flush(pluggedEndpoint+1);
}

size_t WebUSB::write(uint8_t c)
{
	return write(&c, 1);
}

size_t WebUSB::write(const uint8_t *buffer, size_t size)
{
	/* only try to send bytes if the high-level CDC connection itself 
	 is open (not just the pipe) - the OS should set lineState when the port
	 is opened and clear lineState when the port is closed.
	 bytes sent before the user opens the connection or after
	 the connection is closed are lost - just like with a UART. */
	
	// TODO - ZE - check behavior on different OSes and test what happens if an
	// open connection isn't broken cleanly (cable is yanked out, host dies
	// or locks up, or host virtual serial port hangs)
	if (_usbLineInfo.lineState > 0)	{
		int r = USB_Send(pluggedEndpoint+1,buffer,size);
		if (r > 0) {
			return r;
		} else {
			setWriteError();
			return 0;
		}
	}
	setWriteError();
	return 0;
}

// This operator is a convenient way for a sketch to check whether the
// port has actually been configured and opened by the host (as opposed
// to just being connected to the host).  It can be used, for example, in 
// setup() before printing to ensure that an application on the host is
// actually ready to receive and display the data.
// We add a short delay before returning to fix a bug observed by Federico
// where the port is configured (lineState != 0) but not quite opened.
WebUSB::operator bool() {
	bool result = false;
	if (_usbLineInfo.lineState > 0) 
		result = true;
	delay(10);
	return result;
}

unsigned long WebUSB::baud() {
	// Disable interrupts while reading a multi-byte value
	uint32_t baudrate;
#ifdef ARDUINO_ARCH_SAMD
	// nothing needed
#else
	// Disable IRQs while reading and clearing breakValue to make
	// sure we don't overwrite a value just set by the ISR.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#endif
		baudrate =  _usbLineInfo.dwDTERate;
#ifdef ARDUINO_ARCH_SAMD
	// nothing needed
#else
	}
#endif
	return baudrate;
}

uint8_t WebUSB::stopbits() {
	return _usbLineInfo.bCharFormat;
}

uint8_t WebUSB::paritytype() {
	return _usbLineInfo.bParityType;
}

uint8_t WebUSB::numbits() {
	return _usbLineInfo.bDataBits;
}

bool WebUSB::dtr() {
	return _usbLineInfo.lineState & 0x1;
}

bool WebUSB::rts() {
	return _usbLineInfo.lineState & 0x2;
}

int32_t WebUSB::readBreak() {
	int32_t ret;
#ifdef ARDUINO_ARCH_SAMD
	uint8_t enableInterrupts = ((__get_PRIMASK() & 0x1) == 0);

	// disable interrupts,
	// to avoid clearing a breakValue that might occur 
	// while processing the current break value
	__disable_irq();
#else
	// Disable IRQs while reading and clearing breakValue to make
	// sure we don't overwrite a value just set by the ISR.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#endif
		ret = breakValue;
		breakValue = -1;
#ifdef ARDUINO_ARCH_SAMD
	if (enableInterrupts) {
		// re-enable the interrupts
		__enable_irq();
	}
#else
	}
#endif
	return ret;
}
