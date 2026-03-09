#include <QDebug>
#include <QLoggingCategory>
#include <QTimer>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include "mb86m26.h"
#include "mb86m26_lib.h"

static QLoggingCategory category("M26");

static libusb_hotplug_callback_handle libusb_handle;

int mb_hotplug_callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data)
{
        MB86M26 * mb = (MB86M26*) user_data;
        mb->callback(ctx, device, event, 1);
        return 0;
}

void mb_stream_callback(struct libusb_transfer *transfer)
{
        MB86M26 * mb = (MB86M26*) transfer->user_data;
        Q_CHECK_PTR(mb);
        mb->stream_callback(transfer);
}

void mb_interrupt_callback(struct libusb_transfer *transfer)
{
        MB86M26 * mb = (MB86M26*) transfer->user_data;
        Q_CHECK_PTR(mb);
        mb->interrupt_callback(transfer);
}

void mb_control_callback(struct libusb_transfer *transfer)
{
        MB86M26 * mb = (MB86M26*) transfer->user_data;
        Q_CHECK_PTR(mb);
        mb->control_callback(transfer);
}

void MB86M26::mb_firmware_ep2(libusb_device_handle * dev_handle, const char * fname)
{
        int ret;
        int f;
        unsigned char * data;
        int firmware_size;
        int transferred;

        if(dev_handle==NULL)
                return;
        Q_CHECK_PTR(dev_handle);
        f = open(fname, O_RDONLY);
        if(f<0){
                qCDebug(category) << "Failed to open file" << fname;
                goto out;
        }
        firmware_size = lseek(f, 0, SEEK_END);
        lseek(f, 0, SEEK_SET);
        data = (unsigned char*) malloc(firmware_size);
        ret = read(f, data, firmware_size);
        assert(ret == firmware_size);
        ret = libusb_bulk_transfer(dev_handle, 2, data, firmware_size, &transferred, 1000);
        if(ret<0)
                qCDebug(category) << "error" << libusb_error_name(ret);
        free(data);
        close(f);
        out:
        return;
}

void MB86M26::mb_wait_irq()
{
        mutex.lock();
        irq_condition.wait(&mutex, 1000);
        mutex.unlock();
}

void MB86M26::mb_hotplug(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event)
{
        int ret;

        if(event==LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
                qCDebug(category) << "detected in NORMAL mode";
                ret = libusb_open(device, &mb_devh);
                if(ret){
                        qCDebug(category) << "Failed to open device";
                        goto out;
                }
                mb_hotplug_handle();
                emit hotplug();
        }
        else{
                qCDebug(category) << "MB86M26 lost";
                free_usb();
                mb_devh = NULL;
        }
        out:
        return;
}

void MB86M26::stream_callback(struct libusb_transfer *transfer)
{
        if(transfer->actual_length==0)
                goto out;
        if(data_ready)
                data_proc(transfer->buffer, transfer->actual_length);
        out:
        libusb_submit_transfer(transfer);
}

void MB86M26::data_proc(uint8_t * data, int len)
{
        memcpy(buffer + 512*buffer_index, data, len);
        if(buffer_index<2){
                buffer_index += 1;
        }
        else{
                uint8_t * data_out;
                data_out = (uint8_t*) malloc(512*3);
                memcpy(data_out, buffer, 512*3);
                data_ready(data_out, 512*3, callback_context);
                buffer_index = 0;
        }
        
}

void MB86M26::mb_start_bulk(libusb_device_handle * dev_handle)
{
        struct libusb_transfer *transfer;
        int ret;

        if(dev_handle==NULL)
                return;
        Q_CHECK_PTR(dev_handle);

        for (int i = 0; i < 8; i++){
                transfer = usb_transfer[i];
                libusb_fill_bulk_transfer(transfer, dev_handle, MB_STREAM_1_EP, 
                        mb_stream_data, sizeof(mb_stream_data)-8, 
                        mb_stream_callback, this, MB_USB_TIMEOUT);
                ret = libusb_submit_transfer(transfer);
                if(ret){
                        qCDebug(category) << "USB submission error";
                        return;
                }
        }

        

}

void MB86M26::allocate_usb()
{
        struct libusb_transfer *transfer;
        for (int i = 0; i < 8; i++){
                transfer = libusb_alloc_transfer(0);
                if (!transfer){
                        qCDebug(category) << "Cannot allocate usb transfer";
                        return;
                }
                usb_transfer[i] = transfer;
        }
}

void MB86M26::free_usb()
{
        for (int i = 0; i < 8; i++){
                libusb_free_transfer(usb_transfer[i]);
        }
        libusb_close(mb_devh);
}

int MB86M26::mb_read(uint32_t addr, uint16_t * value)
{
        QMutexLocker locker(&control_mutex2);
        struct libusb_transfer *transfer;
        int ret;
        uint16_t wValue;
        uint16_t wIndex;

        if(mb_devh==NULL)
                return 0;
        Q_CHECK_PTR(mb_devh);

        wValue = libusb_cpu_to_le16((0x00)|(((addr>>16)&0xFF)<<8));
        wIndex = libusb_cpu_to_le16((addr&0xFF)|(((addr>>8)&0xFF)<<8));

        libusb_fill_control_setup(mb_control_data, CTRL_IN, CONTROL_REG, 
                wValue, wIndex, sizeof(mb_control_data)-8);
        transfer = libusb_alloc_transfer(0);
        if (!transfer) {
                qCDebug(category) << "Cannot allocate usb transfer";
                return 1;
        }
        libusb_fill_control_transfer(transfer, mb_devh,
                mb_control_data,
                mb_control_callback, this, MB_USB_TIMEOUT);
        ret = libusb_submit_transfer(transfer);
        if(ret){
                qCDebug(category) << "USB submission error";
                return 1;
        }
        control_mutex.lock();
        control_condition.wait(&control_mutex, 1000);
        *value = control_data;
        control_mutex.unlock();
        return 0;

}


int MB86M26::mb_write(uint32_t addr, uint16_t value)
{
        QMutexLocker locker(&control_mutex2);
        struct libusb_transfer *transfer;
        int ret;
        uint16_t wValue;
        uint16_t wIndex;

        if(mb_devh==NULL)
                return 0;
        Q_CHECK_PTR(mb_devh);
        wValue = libusb_cpu_to_le16((0x00)|(((addr>>16)&0xFF)<<8));
        wIndex = libusb_cpu_to_le16((addr&0xFF)|(((addr>>8)&0xFF)<<8));
        mb_control_data[8] = value>>8;
        mb_control_data[9] = value&0xff;

        libusb_fill_control_setup(mb_control_data, CTRL_OUT, CONTROL_REG, 
                wValue, wIndex, sizeof(mb_control_data)-8);
        transfer = libusb_alloc_transfer(0);
        if (!transfer) {
                qCDebug(category) << "Cannot allocate usb transfer";
                return 1;
        }
        libusb_fill_control_transfer(transfer, mb_devh,
                mb_control_data,
                mb_control_callback, this, MB_USB_TIMEOUT);
        ret = libusb_submit_transfer(transfer);
        if(ret){
                qCDebug(category) << "USB submission error";
                return 1;
        }
        control_mutex.lock();
        control_condition.wait(&control_mutex, 1000);
        control_mutex.unlock();
        return 0;

}

void MB86M26::interrupt_callback(struct libusb_transfer *transfer)
{
        if(transfer->status!=LIBUSB_TRANSFER_TIMED_OUT){
                if(transfer->actual_length==0)
                        goto out;
                switch(transfer->buffer[0]){
                case 0x10:
                        break;
                default:
                        qCDebug(category) << "unknowin interrupt transfer";
                        break;
                }
                mutex.lock();
                irq_condition.wakeAll();
                mutex.unlock();
                emit interrupt();
        }


        out:
        libusb_submit_transfer(transfer);
        return;
}

void MB86M26::control_callback(struct libusb_transfer *transfer)
{
        unsigned char * data;
        if(transfer->actual_length==0)
                goto out;
        if(transfer->status!=LIBUSB_TRANSFER_TIMED_OUT){
                data = libusb_control_transfer_get_data(transfer);
                control_data = data[1]|(data[0]<<8);
        }
        libusb_free_transfer(transfer);
        control_mutex.lock();
        control_condition.wakeAll();
        control_mutex.unlock();
        out:
        return;
}

void MB86M26::mb_start_interrupt(libusb_device_handle * dev_handle)
{
        struct libusb_transfer *transfer;
        int ret;

        if(dev_handle==NULL)
                return;
        Q_CHECK_PTR(dev_handle);
        transfer = libusb_alloc_transfer(0);
        if (!transfer) {
                qCDebug(category) << "Cannot allocate usb transfer";
                return;
        }
        libusb_fill_interrupt_transfer(transfer, dev_handle, MB_ITERRUPT_EP, 
                mb_interrupt_data, sizeof(mb_interrupt_data), 
                mb_interrupt_callback, this, MB_USB_TIMEOUT);
        ret = libusb_submit_transfer(transfer);
        if(ret){
                qCDebug(category) << "USB submission error";
                return;
        }
}

void MB86M26::mb_hotplug_handle()
{
        buffer_index = 0;
        allocate_usb();
        mb_start_interrupt(mb_devh);
        mb_start_bulk(mb_devh);
}

void MB86M26::callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, int type)
{
        switch(type){
        case 1:
                mb_hotplug(ctx, device, event);
                break;
        }
}

void MB86M26::run()
{
        struct timeval tv = {1 ,0};
        int status;

        mb_reset();

        status = libusb_init(NULL);
        if (status < 0) {
                qCDebug(category) << "libusb_init() failed" << libusb_error_name(status);
                return;
        }

        libusb_hotplug_register_callback(NULL, 
                LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                LIBUSB_HOTPLUG_ENUMERATE,
                USB_MB_VENDOR_ID,
                USB_MB_PRODUCT_ID,
                LIBUSB_HOTPLUG_MATCH_ANY,
                mb_hotplug_callback,
                this,
                &libusb_handle
        );
        libusb_hotplug_register_callback(NULL, 
                LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                LIBUSB_HOTPLUG_ENUMERATE,
                USB_MB_VENDOR_ID,
                USB_MB_PRODUCT_ID,
                LIBUSB_HOTPLUG_MATCH_ANY,
                mb_hotplug_callback,
                this,
                &libusb_handle
        );
        while(thread_exit_flag==0){
                libusb_handle_events_timeout_completed(NULL, &tv, NULL);
        }
}

void MB86M26::stop()
{
        thread_exit_flag = 1;
        wait();
}

void MB86M26::set_callback(void (*data_ready) (uint8_t *, int, void *), void * ctx)
{
        this->data_ready = data_ready;
        this->callback_context = ctx;
}

void MB86M26::gpio_value(const char * name, int value)
{
        FILE * f;
        char buf[128];
        
        sprintf(buf, GPIO_VALUE, name);
        f = fopen(buf, "w");
        if(!f){
                qCDebug(category) << "GPIO error";
                return;
        }
        sprintf(buf, "%d\n", value);
        fwrite(buf, 1, strlen(buf), f);
        fclose(f);
}

void MB86M26::mb_reset()
{
        QMutexLocker locker(&control_mutex);
        QMutexLocker locker2(&control_mutex2);
        gpio_value(reset_gpio.toLatin1().data(), 0);
        QTimer::singleShot(5000, this, &MB86M26::mb_reset_deassert);
}

void MB86M26::mb_reset_deassert()
{
        gpio_value(reset_gpio.toLatin1().data(), 1);
}

MB86M26::MB86M26(QObject *parent, QString reset_gpio):
        QThread(parent),
        thread_exit_flag(0)
{
        this->reset_gpio = reset_gpio;
        data_ready = NULL;
        callback_context = NULL;
        buffer_index = 0;
}

MB86M26::~MB86M26()
{
        stop();
}
