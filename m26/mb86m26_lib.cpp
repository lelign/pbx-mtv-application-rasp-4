#include <unistd.h>
#include <libusb.h>
#include <stdio.h>
#include <QDebug>
#include <QMutexLocker>
#include "mb86m26_lib.h"

QMutex usb_mutex;

int mb86m26_write_spi_state(libusb_device_handle * dev_handle, char * status)
{
        int ret = 0;
        unsigned char data[1];

        Q_CHECK_PTR(dev_handle);
        QMutexLocker locker(&usb_mutex);

        ret = libusb_control_transfer(dev_handle, CTRL_IN, MB_WRITE_SPI_STATE, 0, 0, data, sizeof(data), USB_TIMEOUT);
        if(ret<0){
                printf("%s error (%s)\n", __FUNCTION__, libusb_error_name(ret));
                ret = 1;
        }
        else{
                ret = 0;
        }

        *status = data[0];
        return ret;
}

int mb86_load_firm2(libusb_device_handle * dev_handle, unsigned char * data, int len, int last)
{
        int ret;
        int wValue;
        int retval = 0;
        Q_CHECK_PTR(dev_handle);
        QMutexLocker locker(&usb_mutex);

        wValue = last;
        ret = libusb_control_transfer(dev_handle, CTRL_OUT, MB_LOAD_FIRM2, wValue, 0, 
                data, len, USB_TIMEOUT);
        if(ret<0){
                printf("%s error (%s)\n", __FUNCTION__, libusb_error_name(ret));
                retval = 1;
        }
        return retval;
}
