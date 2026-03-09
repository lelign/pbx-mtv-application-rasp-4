#include <stdio.h>
#include <libusb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include "../board_config.h"

#define FIRMWARE_FILE "/var/spirom_writer_usb_c.bin"
#define GPIO_VALUE "/sys/class/gpio/%s/value"
#define USB_MB_VENDOR_ID 0x04c5
#define USB_MB_SPI_PRODUCT_ID 0x202f
#define USB_TIMEOUT 1000

//bmRequestType
#define CTRL_OUT 0x40
#define CTRL_IN 0xC0

//bRequest
#define MB_WRITE_SPI_STATE 0xC8
#define MB_LOAD_FIRM 0xC9
#define MB_LOAD_FIRM2 0xCA
#define CONTROL_REG 0xBC

void gpio_value(const char * name, int value)
{
        FILE * f;
        char buf[128];
        
        sprintf(buf, GPIO_VALUE, name);
        f = fopen(buf, "w");
        if(!f)
                printf("GPIO error");
        sprintf(buf, "%d\n", value);
        fwrite(buf, 1, strlen(buf), f);
        fclose(f);
}

int mb86m26_write_spi_state(libusb_device_handle * dev_handle, char * status)
{
        int ret = 0;
        unsigned char data[1];

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

        wValue = last;
        ret = libusb_control_transfer(dev_handle, CTRL_OUT, MB_LOAD_FIRM2, wValue, 0, 
                data, len, USB_TIMEOUT);
        if(ret<0){
                printf("%s error (%s)\n", __FUNCTION__, libusb_error_name(ret));
                retval = 1;
        }
        return retval;
}

static int mb_load_firmware2(libusb_device_handle * dev_handle)
{
        int ret;
        unsigned char data[4096];
        int f;
        int r;
        int firmware_size;
        int index = 0;
        int firmware_end;
        char firmware_status;
        int retval = 0;

        f = open(FIRMWARE_FILE, O_RDONLY);
        if(f<0){
                printf("%s\n", "Failed to open file");
                goto out2;
        }
        firmware_size = lseek(f, 0, SEEK_END);
        lseek(f, 0, SEEK_SET);
        assert(f>=0);
        do{
                r = read(f, data, sizeof(data));
                index += r;
                if(r>0){
                        if(index==firmware_size)
                                firmware_end = 0;
                        else
                                firmware_end = 1;
                        ret = mb86_load_firm2(dev_handle, data, r, firmware_end);
                        if(ret){
                                retval = 1;
                                goto out;
                        }
                }
                
        }
        while (r>0);
        
        usleep(10000);
        do{
                ret = mb86m26_write_spi_state(dev_handle, &firmware_status);
                if(ret){
                        retval = 1;
                        goto out;
                }
                if(firmware_status==0x01){
                        printf("Writing to SPI-ROM has been completed\n");
                        break;
                }
                else if(firmware_status==0x80){
                        printf("Writing to SPI-ROM failed\n");
                        break;
                }
                else if(firmware_status==0x00){
                        printf("Writing to SPI-ROM is in progress\n");
                }
                else{
                        printf("SPI-ROM unknown status\n");
                        break;
                }
                usleep(100000);
        }
        while(ret!=1);

        out:
        close(f);
        out2:
        return retval;
}

int main(int argc, char *argv[])
{
        int ret;
        libusb_device_handle * dev_handle;

        // reset
        gpio_value(M26_GPIO, 0);
        usleep(100000);
        gpio_value(M26_GPIO, 1);
        sleep(1);

        libusb_init(NULL);

        dev_handle = libusb_open_device_with_vid_pid(NULL, USB_MB_VENDOR_ID, USB_MB_SPI_PRODUCT_ID);
        if(dev_handle){
                printf("M26 SPI device found\n");
                ret = mb_load_firmware2(dev_handle);
                if(ret==0)
                        printf("SPIROM programmed\n");
                else
                        printf("SPIROM programming failed\n");
                libusb_close(dev_handle);
        }
        libusb_exit(NULL);
        gpio_value(M26_GPIO, 0);
}
