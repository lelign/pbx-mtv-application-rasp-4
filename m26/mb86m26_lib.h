#ifndef MB86M26_LIB_H
#define MB86M26_LIB_H

#define USB_TIMEOUT 1000

//bmRequestType
#define CTRL_OUT 0x40
#define CTRL_IN 0xC0

//bRequest
#define MB_WRITE_SPI_STATE 0xC8
#define MB_LOAD_FIRM 0xC9
#define MB_LOAD_FIRM2 0xCA
#define CONTROL_REG 0xBC

void usb_mutex_lock();
void usb_mutex_unlock();
int mb86m26_write_spi_state(libusb_device_handle * dev_handle, char * status);
int mb86_load_firm2(libusb_device_handle * dev_handle, unsigned char * data, int len, int last);

#endif