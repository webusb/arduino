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

#include "WebUSB.h"

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
0x2e, 0x00,  // Descriptor set length
0x02,  // Vendor request code
0x00   // Alternate enumeration code
};

const uint8_t MS_OS_20_DESCRIPTOR_PREFIX[] PROGMEM = {
// Microsoft OS 2.0 descriptor set header (table 10)
0x0A, 0x00,  // Descriptor size (10 bytes)
0x00, 0x00,  // MS OS 2.0 descriptor set header
0x00, 0x00, 0x03, 0x06,  // Windows version (8.1) (0x06030000)
0x2e, 0x00,  // Size, MS OS 2.0 descriptor set

// Microsoft OS 2.0 configuration subset header
0x08, 0x00,  // Descriptor size (8 bytes)
0x01, 0x00,  // MS OS 2.0 configuration subset header
0x00,        // bConfigurationValue
0x00,        // Reserved
0x24, 0x00,  // Size, MS OS 2.0 configuration subset

// Microsoft OS 2.0 function subset header
0x08, 0x00,  // Descriptor size (8 bytes)
0x02, 0x00,  // MS OS 2.0 function subset header
};

// First interface number (1 byte) sent here.

const uint8_t MS_OS_20_DESCRIPTOR_SUFFIX[] PROGMEM = {
0x00,        // Reserved
0x1c, 0x00,  // Size, MS OS 2.0 function subset

// Microsoft OS 2.0 compatible ID descriptor (table 13)
0x14, 0x00,  // wLength
0x03, 0x00,  // MS_OS_20_FEATURE_COMPATIBLE_ID
'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

typedef struct
{
	u32	dwDTERate;
	u8	bCharFormat;
	u8 	bParityType;
	u8 	bDataBits;
	u8	lineState;
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
	memcpy(name, "WUART", 5);
	return 5;
}

bool WebUSB::VendorControlRequest(USBSetup& setup)
{
	if (setup.bmRequestType == (REQUEST_DEVICETOHOST | REQUEST_VENDOR | REQUEST_DEVICE)) {
		if (setup.bRequest == 0x01 && setup.wIndex == WEBUSB_REQUEST_GET_ALLOWED_ORIGINS)
		{
			uint8_t allowedOriginsPrefix[] = {
				// Allowed Origins Header, bNumConfigurations = 1
				0x05, 0x00, 0x0c + numAllowedOrigins, 0x00, 0x01,
				// Configuration Subset Header, bNumFunctions = 1
				0x04, 0x01, 0x01, 0x01,
				// Function Subset Header, bFirstInterface = pluggedInterface
				0x03 + numAllowedOrigins, 0x02, pluggedInterface
			};
			if (USB_SendControl(0, &allowedOriginsPrefix, sizeof(allowedOriginsPrefix)) < 0)
				return false;
			return USB_SendControl(0, allowedOrigins, numAllowedOrigins) >= 0;
		}
		else if (setup.bRequest == 0x01 && setup.wIndex == WEBUSB_REQUEST_GET_URL)
		{
                        if (setup.wValueL == 0 || setup.wValueL > numUrls)
				return false;
			const WebUSBURL& url = urls[setup.wValueL - 1];
			uint8_t urlLength = strlen(url.url);
			uint8_t descriptorLength = urlLength + 3;
			if (USB_SendControl(0, &descriptorLength, 1) < 0)
				return false;
			uint8_t descriptorType = 3;
			if (USB_SendControl(0, &descriptorType, 1) < 0)
				return false;
			if (USB_SendControl(0, &url.scheme, 1) < 0)
				return false;
			return USB_SendControl(0, url.url, urlLength) >= 0;
		}
		else if (setup.bRequest == 0x02 && setup.wIndex == MS_OS_20_REQUEST_DESCRIPTOR)
		{
			if (USB_SendControl(TRANSFER_PGM, &MS_OS_20_DESCRIPTOR_PREFIX, sizeof(MS_OS_20_DESCRIPTOR_PREFIX)) < 0)
				return false;
			if (USB_SendControl(0, &pluggedInterface, 1) < 0)
				return false;
			return USB_SendControl(TRANSFER_PGM, &MS_OS_20_DESCRIPTOR_SUFFIX, sizeof(MS_OS_20_DESCRIPTOR_SUFFIX)) >= 0;
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

WebUSB::WebUSB(const WebUSBURL* urls, uint8_t numUrls, uint8_t landingPage,
               const uint8_t* allowedOrigins, uint8_t numAllowedOrigins)
	: PluggableUSBModule(2, 1, epType),
	  urls(urls), numUrls(numUrls), landingPage(landingPage),
	  allowedOrigins(allowedOrigins), numAllowedOrigins(numAllowedOrigins)
{
	// one interface, 2 endpoints
	epType[0] = EP_TYPE_BULK_OUT;
	epType[1] = EP_TYPE_BULK_IN;
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
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		baudrate =  _usbLineInfo.dwDTERate;
	}
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
	// Disable IRQs while reading and clearing breakValue to make
	// sure we don't overwrite a value just set by the ISR.
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		ret = breakValue;
		breakValue = -1;
	}
	return ret;
}
