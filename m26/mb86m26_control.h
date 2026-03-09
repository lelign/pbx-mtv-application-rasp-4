#ifndef MB86M26_CONTROL_H
#define MB86M26_CONTROL_H

#include <QString>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <libusb.h>
#include <QTimer>
#include "../m26/mb86m26.h"
#include "../str-system/video_info.h"

class mb86m26_control : public QObject
{
        Q_OBJECT
public:
        typedef struct {
                int id;
                int valid;
                int v_format;
                int v_vin_vsync_mode;
                int v_vin_yc_mux;
                int v_vin_clk_div;
                int v_vin_top_start_line;
                int v_vin_bot_start_line;
                int v_vin_max_h_count;
                int v_vin_max_v_count;
                int v_vin_h_valid_pos;
                int v_vin_top_valid_line;
                int v_vin_bot_valid_line;
                int v_vin_valid_hcount;
                int v_vin_valid_vcount;
                int sd;
        } pbx_std_name_t;

        enum out_std {
                STD_OUT_1080,
                STD_OUT_720,
                STD_OUT_480,
                STD_OUT_360,
                STD_OUT_240
        };

        enum V_FORMAT {
                NATIVE,
                PULLDOWN
        };

        enum PROFILE {
                H264_HIGH = 1,
                H264_MAIN = 2,
        };

        enum LEVEL {
                LVL_30 = 30,
                LVL_31 = 31,
                LVL_32 = 32,
                LVL_40 = 40,
                LVL_41 = 41,
                LVL_42 = 42
        };

        enum GOP {
                IBBP = 0,
                IPPP = 1
        };

        enum AUDIO {
                MPEG1 = 0,
                AAC = 1
        };

        enum SD_ASPCET {
                ASPECT_4_3 = 0,
                ASPECT_16_9 = 2,
        };

        enum VLC_MODE {
                CABAC,
                CAVLC,
        };

        enum GOP_MODE {
                GOP_OPEN,
                GOP_CLOSED,
        };

        enum DELAY_MODE {
                DELAY_LOW,
                DELAY_MED,
                DELAY_HIGH,
        };

        int thread_exit_flag;
        MB86M26 * m26;
        void stop();
        void set_std(int std);
        void set_output_std(out_std std);
        void set_output_std(int std);
        void set_bitrate(uint32_t video, uint32_t audio, uint32_t system);
        void set_v_format(V_FORMAT value);
        void set_auto_null(int value);
        void set_gop(GOP value);
        void set_h264_profile(PROFILE value);
        void set_h264_level(LEVEL value);
        void set_a_format(AUDIO value);
        void set_pid(uint16_t pid_video, uint16_t pid_audio, 
                uint16_t pid_pmt, uint16_t pid_program,
                uint16_t pid_sid, uint16_t pid_pcr, uint16_t pid_teletext = 1000);
        void set_idr_interval(int value);
        void set_sd_aspect(SD_ASPCET value);
        int check_video_bitrate(int bitrate);
        int check_audio_bitrate(int bitrate);
        int check_system_bitrate(uint32_t bitrate);
        int get_state();
        void set_teletext_enable(int value);
        void set_vlc_mode(VLC_MODE value);
        void set_gop_mode(GOP_MODE value);
        void set_gop_size(int value);
        int check_gop_size(int value);
        void set_delay_mode(DELAY_MODE value);
        void set_aux_audio_enable(int value);
        void set_pid_aux_audio(int value);
        void set_aux_audio_bitrate(int value);
        mb86m26_control(QObject *parent, QString reset_gpio);
        ~mb86m26_control();
private:
        uint8_t * packet_buffer;
        int buffer_index;
        int input_video_std;
        out_std output_video_std;
        QTimer * state_timeout_timer;
        uint32_t video_bitrate;
        uint32_t audio_bitrate;
        V_FORMAT v_ip_format;
        int auto_null;
        PROFILE h264_profile;
        LEVEL h264_level;
        GOP v_gop_struct;
        AUDIO audio_format;
        SD_ASPCET sd_aspect;
        uint16_t pid_video;
        uint16_t pid_audio;
        uint16_t pid_pmt;
        uint16_t pid_sid;
        uint16_t pid_pcr;
        uint16_t pid_program;
        uint16_t pid_teletext;
        int teletext_enable;
        int idr_interval;
        int m26_state;
        int mb_v_max_bitrate;
        int mb_v_ave_bitrate;
        int mb_v_min_bitrate;
        int mb_bitrate;
        int mb_a_bitrate;
        int mb_a_max_bitrate;
        uint32_t mb_system_bitrate;
        VLC_MODE vlc_mode;
        GOP_MODE gop_mode;
        int gop_size;
        int ldmode;
        DELAY_MODE delay_mode;
        int aux_audio_enable;
        uint16_t pid_aux_audio;
        int aux_audio_bitrate;
        uint32_t mb_print_state();
        void mb_cmd(libusb_device_handle * devh, uint16_t cmd, uint16_t body);
        void mb_boot_state(libusb_device_handle * dev_handle);
        void mb_cert_state(libusb_device_handle * dev_handle);
        void mb_wait_irq_ack(libusb_device_handle * dev_handle);
        void mb_handle_state(libusb_device_handle * dev_handle);
        void mb_encode_1080(libusb_device_handle * devh);
        void mb_encoder_firmware(libusb_device_handle * devh);
        void mb_encoder_cmd(libusb_device_handle * devh);
        void mb_endcode_stop(libusb_device_handle * devh);
        void stream_reset();
        pbx_std_name_t * get_std_info();
        void state_timeount_start();
        int get_std_width(out_std std);
        int get_std_height(out_std std);
        int is_native(out_std std);
        int is_interlaced();

Q_SIGNALS:
        void data_ready(uint8_t * data, int len);
        void restart();
public Q_SLOTS:
        void hotplug();
        void interrupt();
private Q_SLOTS:
        void mb_restart_request();
        void state_timeout();
};

#endif
