#ifndef MB86M26_H
#define MB86M26_H

#include <QString>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <libusb.h>

#define USB_MB_VENDOR_ID 0x04c5
#define USB_MB_SPI_PRODUCT_ID 0x202f
#define USB_MB_PRODUCT_ID 0x202e

#define MB_REG_STATE 0x82008
#define MB_REG_API_SETIRQ 0x90070
#define MB_REG_API_IRQST 0x90074

#define MB_ITERRUPT_EP 0x83
#define MB_STREAM_1_EP 0x81
#define MB_STREAM_5_EP 0x85
#define MB_USB_TIMEOUT 1000

//STATE
#define MB_STATE_BOOT 0x0000
#define MB_STATE_STOP 0x0001
#define MB_STATE_START 0x0002
#define MB_STATE_NULL_OUTPUT 0x0004
#define MB_STATE_CERT 0x0010
#define MB_STATE_IDLE 0x0011
#define MB_STATE_XERROR 0xFFFF

#define FIRMWARE_FILE "/var/spirom_writer_usb_c.bin"
#define FIRMWARE_IDLE_FILE "/var/mb86m21_assp_nsec_idle.bin"
#define FIRMWARE_ENC_FILE "/var/mb86m21_assp_nsec_enc_h.bin"
#define FIRMWARE_LDENC_FILE "/var/mb86m21_assp_nsec_ldenc_h.bin"
//#define GPIO_VALUE "/sys/class/gpio/%s/value"
#define GPIO_VALUE "/home/pi/gpio_from_508/%s/value"

// 458


class MB86M26 : public QThread
{
        Q_OBJECT
public:
        int thread_exit_flag;
        libusb_device_handle * mb_devh;
        void run();
        void stop();
        void callback(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, int type);
        void stream_callback(struct libusb_transfer *transfer);
        void mp4_callback(struct libusb_transfer *transfer);
        void interrupt_callback(struct libusb_transfer *transfer);
        void control_callback(struct libusb_transfer *transfer);
        void mb_firmware_ep2(libusb_device_handle * dev_handle, const char * fname);
        void mb_wait_irq();
        void mb_reset();
        int mb_read(uint32_t addr, uint16_t * value);
        int mb_write(uint32_t addr, uint16_t value);
        void data_proc(uint8_t * data, int len);
        void set_callback(void (*data_ready) (uint8_t *, int, void *), void * ctx);
        MB86M26(QObject *parent, QString reset_gpio);
        ~MB86M26();
private:
        unsigned char mb_interrupt_data[64];
        unsigned char mb_stream_data[512+8];
        unsigned char mb_stream_mp4_data[512+8];
        unsigned char mb_control_data[2+8];
        QMutex mutex;
        QWaitCondition irq_condition;
        QMutex control_mutex;
        QMutex control_mutex2;
        QWaitCondition control_condition;
        uint16_t control_data;
        QString reset_gpio;
        int buffer_index;
        char buffer[512*3];
        struct libusb_transfer * usb_transfer[8];
        void (*data_ready) (uint8_t * data, int len, void * ctx);
        void * callback_context;
        void mb_hotplug(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event);
        void spi_firmware(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event);
        void mb_start_bulk(libusb_device_handle * dev_handle);
        void mb_start_interrupt(libusb_device_handle * dev_handle);
        void mb_hotplug_handle();
        void gpio_value(const char * name, int value);
        void allocate_usb();
        void free_usb();
Q_SIGNALS:
        void hotplug();
        
        void interrupt();
private Q_SLOTS:
        void mb_reset_deassert();
};

#endif
