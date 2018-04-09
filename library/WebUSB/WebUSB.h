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

#ifndef WebUSB_h
#define WebUSB_h

#include <stdint.h>
#include <Arduino.h>
#ifdef ARDUINO_ARCH_SAMD
#include "USB/PluggableUSB.h"
#else
#include "PluggableUSB.h"
#include <avr/wdt.h>
#include <util/atomic.h>
#endif

#ifndef USBCON
#error "WebUSB requires a board that supports USB client device mode."
#endif

#define USB_BOS_DESCRIPTOR_TYPE		15
#define WEBUSB_REQUEST_GET_URL			0x02

#define MS_OS_20_REQUEST_DESCRIPTOR 0x07

#ifndef WEBUSB_SHORT_NAME
// Max length 20 (ISERIAL_MAX_LEN defined in USBDesc.h)
#define WEBUSB_SHORT_NAME "WUART"
#endif


typedef struct
{
	InterfaceDescriptor dif;
	EndpointDescriptor  in;
	EndpointDescriptor  out;
} WebUSBDescriptor;

class WebUSB : public PluggableUSBModule, public Stream
{
public:
	/*
	 * Together |landingPageScheme| and |landingPageUrl| tell the browser
	 * what page the user should visit in order to interact with their
	 * device. |landingPageScheme| can have any of the following values:
	 *
	 *  0x00 -> "http://"
	 *  0x01 -> "https://"
	 *
	 * This prefix is combined with |landingPageUrl| to produce the full
	 * URL.
	 */
	WebUSB(uint8_t landingPageScheme, const char* landingPageUrl);
	void begin(unsigned long);
	void begin(unsigned long, uint8_t);
	void end(void);
	/*
	 * Sets the string reported as the USB device serial number.
	 * This should be called before |begin()|, typically in |setup()|.
	 * |name| should be a pointer to static char array containing
	 * a nul-terminated string containing at most 20 characters
	 * (not counting the final nul character).
	 */
	void setShortName(const char* name);

	virtual int available(void);
	virtual int peek(void);
	virtual int read(void);
	int availableForWrite(void);
	virtual void flush(void);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t*, size_t);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	operator bool();

	volatile uint8_t _rx_buffer_head;
	volatile uint8_t _rx_buffer_tail;
	unsigned char _rx_buffer[SERIAL_BUFFER_SIZE];

	int32_t readBreak();

	uint32_t baud();
	uint8_t stopbits();
	uint8_t paritytype();
	uint8_t numbits();
	bool dtr();
	bool rts();
	enum {
		ONE_STOP_BIT = 0,
		ONE_AND_HALF_STOP_BIT = 1,
		TWO_STOP_BITS = 2,
	};
	enum {
		NO_PARITY = 0,
		ODD_PARITY = 1,
		EVEN_PARITY = 2,
		MARK_PARITY = 3,
		SPACE_PARITY = 4,
	};

protected:
	// Implementation of the PluggableUSBModule
	int getInterface(uint8_t* interfaceCount);
	int getDescriptor(USBSetup& setup);
	bool setup(USBSetup& setup);
	uint8_t getShortName(char* name);

private:
	bool VendorControlRequest(USBSetup& setup);

#ifdef ARDUINO_ARCH_SAMD
	uint32_t epType[2];
#else
	uint8_t epType[2];
#endif
	uint16_t descriptorSize;
	uint8_t protocol;
	uint8_t idle;
	int peek_buffer;
	uint8_t landingPageScheme;
	const char* landingPageUrl;
	const char* shortName;
};

#endif // WebUSB_h
