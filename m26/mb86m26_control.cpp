#include <QDebug>
#include <QLoggingCategory>
#include <QElapsedTimer>
#include <bitstream/mpeg/psi.h>
#include <bitstream/dvb/si.h>
#include <bitstream/mpeg/ts.h>
#include "mb86m26_control.h"
#include "m26/mb86m26_lib.h"

static QLoggingCategory category("M26Control");

const char idle_file[] = FIRMWARE_IDLE_FILE;
const char enc_file[] = FIRMWARE_ENC_FILE;
const char ldenc_file[] = FIRMWARE_LDENC_FILE;

mb86m26_control::pbx_std_name_t m26_std_info[] = {
        {
                .id = STD_1080i50,
                .valid = 1,
                .v_format = 1,
                .v_vin_vsync_mode = 1,
                .v_vin_yc_mux = 0,
                .v_vin_clk_div = 0,
                .v_vin_top_start_line = 1,
                .v_vin_bot_start_line = 561,
                .v_vin_max_h_count = 2640,
                .v_vin_max_v_count = 1125,
                .v_vin_h_valid_pos = 720,
                .v_vin_top_valid_line = 21,
                .v_vin_bot_valid_line = 584,
                .v_vin_valid_hcount = 1920,
                .v_vin_valid_vcount = 540,
                .sd = 0,
        },
        {
                .id = STD_720p50,
                .valid = 1,
                .v_format = 3,
                .v_vin_vsync_mode = 0,
                .v_vin_yc_mux = 0,
                .v_vin_clk_div = 0,
                .v_vin_top_start_line = 6,
                .v_vin_bot_start_line = 0,
                .v_vin_max_h_count = 1980,
                .v_vin_max_v_count = 750,
                .v_vin_h_valid_pos = 700,
                .v_vin_top_valid_line = 31,
                .v_vin_bot_valid_line = 0,
                .v_vin_valid_hcount = 1280,
                .v_vin_valid_vcount = 720,
                .sd = 0,
        },
        {
                .id = STD_625i50,
                .valid = 1,
                .v_format = 5,
                .v_vin_vsync_mode = 2,
                .v_vin_yc_mux = 1,
                .v_vin_clk_div = 1,
                .v_vin_top_start_line = 1,
                .v_vin_bot_start_line = 313,
                .v_vin_max_h_count = 1728,
                .v_vin_max_v_count = 625,
                .v_vin_h_valid_pos = 288,
                .v_vin_top_valid_line = 23,
                .v_vin_bot_valid_line = 336,
                .v_vin_valid_hcount = 1440,
                .v_vin_valid_vcount = 576,
                .sd = 1,
        },
        {
                .id = STD_1080p25,
                .valid = 1,
                .v_format = 33,
                .v_vin_vsync_mode = 0,
                .v_vin_yc_mux = 0,
                .v_vin_clk_div = 0,
                .v_vin_top_start_line = 1,
                .v_vin_bot_start_line = 561,
                .v_vin_max_h_count = 2640,
                .v_vin_max_v_count = 1125,
                .v_vin_h_valid_pos = 720,
                .v_vin_top_valid_line = 41,
                .v_vin_bot_valid_line = 584,
                .v_vin_valid_hcount = 1920,
                .v_vin_valid_vcount = 540,
                .sd = 0,
        },
        {
                .id = -1,
        }
};

#define IRQ_MASK_STATE (1<<2)
#define IRQ_MASK_CMD_ACK (1<<3)
#define IRQ_MASK_VSYNC (1<<4)

int gen_pat(int pmt_pid, int pid_sid, int program, uint8_t * buf)
{
        uint8_t *pat = psi_allocate();
        uint8_t *pat_n;
        uint8_t j = 0;
        int loop_length;
        uint8_t * loop_ptr;

        pat_init(pat);
        pat_set_length(pat, 0);
        pat_set_tsid(pat, pid_sid);
        psi_set_version(pat, 0);
        psi_set_current(pat);
        psi_set_section(pat, 0);
        psi_set_lastsection(pat, 0);
        psi_set_crc(pat);

        psi_set_length(pat, PSI_MAX_SIZE);
        pat_n = pat_get_program(pat, j++);
        patn_init(pat_n);
        patn_set_program(pat_n, program);
        patn_set_pid(pat_n, pmt_pid);

        pat_n = pat_get_program(pat, j);
        pat_set_length(pat, pat_n - pat - PAT_HEADER_SIZE);
        psi_set_crc(pat);

        loop_length = psi_get_length(pat)-1;
        loop_ptr = pat;
        memcpy(buf, loop_ptr, loop_length);

        free(pat);
        return loop_length;
}

/* DVB  Descriptor 0x46: VBI teletext descriptor */
static void build_desc46(uint8_t *desc) {
        uint8_t k = 0;
        uint8_t *lang_n;

        desc46_init(desc);
        desc_set_length(desc, 255);

        lang_n = desc46_get_language(desc, k++);
        desc46n_set_code(lang_n, (uint8_t *)"rus");
        desc46n_set_teletexttype(lang_n, 1);
        desc46n_set_teletextmagazine(lang_n, 1);
        desc46n_set_teletextpage(lang_n, 0);

        lang_n = desc46_get_language(desc, k);
        desc_set_length(desc, lang_n - desc - DESC_HEADER_SIZE);
}

/* DVB  Descriptor 0x52: Stream identifier descriptor */
static void build_desc52(uint8_t *desc) {
        desc52_init(desc);
        desc52_set_component_tag(desc, 3);
}

/* DVB  Descriptor 0x56: Teletext descriptor */
static void build_desc56(uint8_t *desc) {
        // Desc 46 is the same as desc 56, only
        // the descriptor tag is different
        build_desc46(desc);
        desc56_init(desc);
}

int gen_es_info(int audio_pid, int video_pid, int program, int aac, 
        int teletext_pid, int pcr_pid, int teletext_enable, int aux_audio_enable,
        uint16_t pid_aux_audio, 
        uint8_t * buf)
{
        uint8_t *pmt = psi_allocate();
        uint8_t *pmt_n;
        uint8_t pmt_n_counter;
        int loop_length;
        uint8_t * loop_ptr;

        // Generate empty PMT
        pmt_init(pmt);
        psi_set_version(pmt, 0);
        psi_set_current(pmt);

        pmt_set_length(pmt, 0);
        pmt_set_desclength(pmt, 0);

        psi_set_crc(pmt);

        // Add elementary streams
        pmt_init(pmt);
        psi_set_version(pmt, 1);
        psi_set_current(pmt);
        pmt_set_pcrpid(pmt, pcr_pid);
        pmt_set_program(pmt, program);


        pmt_set_length(pmt, PSI_MAX_SIZE); // This needed so pmt_get_es works

        // Process elementary streams
        pmt_n_counter = 0;

        pmt_n = pmt_get_es(pmt, pmt_n_counter++);
        pmtn_init(pmt_n);
        pmtn_set_streamtype(pmt_n, 0x1b); // H.264 Video
        pmtn_set_pid(pmt_n, video_pid);
        pmtn_set_desclength(pmt_n, 0);

        pmt_n = pmt_get_es(pmt, pmt_n_counter++);
        pmtn_init(pmt_n);
        if(aac==0)
                pmtn_set_streamtype(pmt_n, 0x03); // MPEG1 audio
        else
                pmtn_set_streamtype(pmt_n, 0x0f); // AAC audio
        pmtn_set_pid(pmt_n, audio_pid);
        pmtn_set_desclength(pmt_n, 0);

        // вторая дорожка звука
        if(aux_audio_enable){
                pmt_n = pmt_get_es(pmt, pmt_n_counter++);
                pmtn_init(pmt_n);
                pmtn_set_streamtype(pmt_n, 0x0f); // AAC audio
                pmtn_set_pid(pmt_n, pid_aux_audio);
                pmtn_set_desclength(pmt_n, 0);
        }

        // teletext
        if(teletext_enable){
                pmt_n = pmt_get_es(pmt, pmt_n_counter++);
                pmtn_init(pmt_n);
                pmtn_set_streamtype(pmt_n, 0x06); // MPEG1 audio
                pmtn_set_pid(pmt_n, teletext_pid);
                pmtn_set_desclength(pmt_n, 0);
                {
                        uint8_t *desc_loop, *desc;
                        uint8_t desc_counter;

                        // Add descriptors to transport_stream_n
                        desc_counter = 0;
                        desc_loop = pmtn_get_descs(pmt_n);
                        descs_set_length(desc_loop, DESCS_MAX_SIZE); // This is needed so descs_get_desc(x, n) works

                        desc = descs_get_desc(desc_loop, desc_counter++);
                        build_desc52(desc);
                        

                        desc = descs_get_desc(desc_loop, desc_counter++);
                        build_desc56(desc);

                        // Finish descriptor generation
                        desc = descs_get_desc(desc_loop, desc_counter); // Get next descriptor pos
                        descs_set_length(desc_loop, desc - desc_loop - DESCS_HEADER_SIZE);
                }
        }
        

        // Set transport_stream_loop length
        pmt_n = pmt_get_es(pmt, pmt_n_counter); // Get last service
        pmt_set_length(pmt, pmt_n - pmt_get_es(pmt, 0) + pmt_get_desclength(pmt));

        psi_set_crc(pmt);
        loop_length = psi_get_length(pmt)-1;
        loop_ptr = pmt;
        memcpy(buf, loop_ptr, loop_length);
        free(pmt);

        return loop_length;
}

uint32_t mb86m26_control::mb_print_state()
{
        uint16_t state = MB_STATE_XERROR;
        int ret;
        uint16_t data;

        ret = m26->mb_read(MB_REG_STATE, &state);
        if(ret){
                state = MB_STATE_XERROR;
                qCDebug(category) << "State: XERROR";
                goto out;
        }
                
        switch(state){
        case MB_STATE_BOOT:
                qCDebug(category) << "State: Boot";
                break;
        case MB_STATE_STOP:
                qCDebug(category) << "State: STOP";
                break;
        case MB_STATE_START:
                qCDebug(category) << "State: Start";
                break;
        case MB_STATE_NULL_OUTPUT:
                qCDebug(category) << "State: NULL output";
                break;
        case MB_STATE_CERT:
                qCDebug(category) << "State: Secure certification";
                break;
        case MB_STATE_IDLE:
                qCDebug(category) << "State: IDLE";
                break;
        default:
                qCDebug(category) << "State: XERROR";
                m26->mb_read(0x80080, &data);
                qCDebug(category) << "M_ERROR_INFO_H:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x80082, &data);
                qCDebug(category) << "M_ERROR_INFO_L:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x80084, &data);
                qCDebug(category) << "V_ERROR_INFO_H:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x80086, &data);
                qCDebug(category) << "V_ERROR_INFO_L:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x80088, &data);
                qCDebug(category) << "A_ERROR_INFO_H:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x8008A, &data);
                qCDebug(category) << "A_ERROR_INFO_L:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x8008C, &data);
                qCDebug(category) << "A2_ERROR_INFO_H:" << QString("%1").arg(data, 0, 16);
                m26->mb_read(0x8008E, &data);
                qCDebug(category) << "A2_ERROR_INFO_L:" << QString("%1").arg(data, 0, 16);
                break;
        }
        // if(state==MB_STATE_START)
        //         mb_enc_state = 1;
        // else
        //         mb_enc_state = 0;
        out:
        return state;
}

void mb86m26_control::mb_cmd(libusb_device_handle * devh, uint16_t cmd, uint16_t body)
{
        uint16_t data;

        if(devh==NULL)
                return;
        Q_CHECK_PTR(devh);
        m26->mb_write(0x80004, cmd);
        m26->mb_write(0x80006, body);
        m26->mb_write(0x90070, 0x0004);
        mb_wait_irq_ack(devh);
        m26->mb_read(0x80014, &data);
}

void mb86m26_control::mb_boot_state(libusb_device_handle * dev_handle)
{
        qCDebug(category) << "Downloading firmware EP2";
        m26->mb_firmware_ep2(dev_handle, idle_file);
        m26->mb_write(MB_REG_API_SETIRQ, (1<<2));
}

void mb86m26_control::mb_cert_state(libusb_device_handle * dev_handle)
{
        //idle data
        //data
        m26->mb_write(0x80074, 0x0000);
        m26->mb_write(0x80076, 0x0000);
        m26->mb_write(0x80078, 0x0500);
        //idle cmd
        mb_cmd(dev_handle, 0x0100, 0x0000);
        state_timeount_start();
}

void mb86m26_control::mb_wait_irq_ack(libusb_device_handle * dev_handle)
{
        uint16_t value;
        uint16_t mask = (1<<3);
        QElapsedTimer timer;
        timer.start();

        m26->mb_read(MB_REG_API_IRQST, &value);
        while(!(value&mask)){
                if(timer.elapsed() > 1000){
                        qCDebug(category) << "IRQ timeout";
                        goto out;
                }
                m26->mb_wait_irq();
                m26->mb_read(MB_REG_API_IRQST, &value);
        }
        m26->mb_write(MB_REG_API_IRQST, mask);
        out:
        return;
}

void mb86m26_control::mb_handle_state(libusb_device_handle * dev_handle)
{
        uint16_t state;

        if(dev_handle==NULL)
                return;
        Q_CHECK_PTR(dev_handle);
        mb86m26_control::pbx_std_name_t * video_std = get_std_info();

        state_timeout_timer->stop();

        state = mb_print_state();
        m26_state = 0;
        switch(state){
        case MB_STATE_BOOT:
                mb_boot_state(dev_handle);
                break;
        case MB_STATE_CERT:
                mb_cert_state(dev_handle);
                break;
        case MB_STATE_IDLE:
                if(video_std->valid==1){
                        emit restart();
                        mb_encode_1080(dev_handle);
                        mb_encoder_firmware(dev_handle);
                        stream_reset();
                }
                break;
        case MB_STATE_XERROR:
                QTimer::singleShot(5000, this, SLOT(mb_restart_request()));
                break;
        case MB_STATE_START:
                m26_state = 1;
                break;
        case MB_STATE_STOP:
                        mb_encoder_cmd(dev_handle);
                break;
        default:
                qCDebug(category) << "State: UNKNOWN " << state;
                break;
        }
        return;
}

void mb86m26_control::mb_restart_request()
{
        m26->mb_reset();
}

void mb86m26_control::hotplug()
{
}

int mb86m26_control::check_video_bitrate(int bitrate)
{
        mb86m26_control::pbx_std_name_t * video_std = get_std_info();

        int width_tmp;
        int ret = bitrate;

        if(is_native(output_video_std) || (v_ip_format == V_FORMAT::NATIVE)){
                width_tmp = video_std->v_vin_valid_hcount;
                if(video_std->sd)
                        width_tmp = width_tmp / 2;
        }
        else{
                width_tmp = get_std_width(output_video_std);
        }

        if(width_tmp >= 1920){
                if(ret < 6000)
                        ret = 6000;
        } else if(width_tmp >= 1280){
                if(ret < 2500)
                        ret = 2500;
        } else{
                if(ret < 1500)
                        ret = 1500;
        }
        if(ret > 20000)
                ret = 20000;
        
        return ret;
}

int mb86m26_control::check_audio_bitrate(int bitrate)
{
        int ret = bitrate;
        if(audio_format==AUDIO::MPEG1){
                if(ret >= 384)
                        ret = 384;
                else if(ret >= 320)
                        ret = 320;
                else if(ret >= 256)
                        ret = 256;
                else if(ret >= 224)
                        ret = 224;
                else if(ret >= 192)
                        ret = 192;
                else if(ret >= 160)
                        ret = 160;
                else if(ret >= 128)
                        ret = 128;
                else if(ret >= 112)
                        ret = 112;
                else if(ret >= 96)
                        ret = 96;
                else
                        ret = 64;
        } else {
                if(ret >= 384)
                        ret = 384;
                else if(ret < 48)
                        ret = 48;
        }

        return ret;
}

int mb86m26_control::check_system_bitrate(uint32_t bitrate)
{
        video_bitrate = check_video_bitrate(video_bitrate);
        audio_bitrate = check_audio_bitrate(audio_bitrate);

        mb_v_max_bitrate = video_bitrate*95/100;
        mb_v_ave_bitrate = mb_v_max_bitrate-100;
        mb_v_min_bitrate = mb_v_max_bitrate-200;
        mb_bitrate = mb_v_max_bitrate*102/100;

        mb_a_bitrate = audio_bitrate;
        mb_a_max_bitrate = mb_a_bitrate*12/10;

        int ts_audio_bitrate = (mb_a_max_bitrate-48)*105/100+70;
        
        mb_system_bitrate = (mb_bitrate+ts_audio_bitrate)*104/100;

        if(teletext_enable)
                mb_system_bitrate += 128;
        if(aux_audio_enable)
                mb_system_bitrate += (aux_audio_bitrate-48)*105/100+70;

        if(bitrate > mb_system_bitrate)
                mb_system_bitrate = bitrate;
        if(mb_system_bitrate > 21000)
                mb_system_bitrate = 21000;
        return mb_system_bitrate;
}

void mb86m26_control::mb_encode_1080(libusb_device_handle * devh)
{
        uint16_t tmp;
        uint8_t buf[1024];
        int len;

        if(devh==NULL)
                return;
        Q_CHECK_PTR(devh);
        mb86m26_control::pbx_std_name_t * video_std = get_std_info();

        // initializaton parameters
        //S
        m26->mb_write(0x1002, 0x0002);
        m26->mb_write(0x1004, 0x8D04);
        m26->mb_write(0x1006, 0x8000);
        m26->mb_write(0x100C, 0x3020);
        m26->mb_write(0x100A, 0x0010);
        m26->mb_write(0x1106, 0x0000);
        m26->mb_write(0x1108, 0x0000);
        m26->mb_write(0x110A, 0x0000);
        m26->mb_write(0x110C, 0x0000);
        m26->mb_write(0x110E, 0x0000);
        m26->mb_write(0x1110, 0x0000);
        m26->mb_write(0x112C, pid_pmt);
        m26->mb_write(0x112E, pid_pcr);
        m26->mb_write(0x1134, 0x001F);
        m26->mb_write(0x1126, pid_video);
        m26->mb_write(0x1128, pid_audio);
        m26->mb_write(0x1132, 0x00C0);
        m26->mb_write(0x113C, 0x0000);
        m26->mb_write(0x1144, 0x00B0);
        m26->mb_write(0x1146, 0x1100);
        m26->mb_write(0x1148, 0x00C1);
        m26->mb_write(0x114A, 0x0000);
        m26->mb_write(0x114C, 0x0001);
        m26->mb_write(0x114E, 0xE100);
        m26->mb_write(0x1150, 0x0000);
        m26->mb_write(0x1152, 0xE01F);
        m26->mb_write(0x113E, 0x0000);
        m26->mb_write(0x1140, 0x0000);
        m26->mb_write(0x1142, 0x0000);
        m26->mb_write(0x113e, 0x0E00);
        m26->mb_write(0x1010, 0x7FF0);
        m26->mb_write(0x1012, 0x0FFF);
        m26->mb_write(0x1014, 0xFFC1);
        m26->mb_write(0x1016, 0x0000);
        m26->mb_write(0x1018, 0xF000);
        m26->mb_write(0x101A, 0x0001);
        m26->mb_write(0x101C, 0xC000);

        //V
        tmp = 0
                |(video_std->v_format<<0)
                ;
        m26->mb_write(0x1502, tmp);
        m26->mb_write(0x1504, 0x0002 | (sd_aspect<<8));
        m26->mb_write(0x1540, 0x0000);
        m26->mb_write(0x1542, 0x0000);
        //параметры сихросигналов
        tmp = 0x1050
                |(video_std->v_vin_vsync_mode<<8)
                |(video_std->v_vin_clk_div<<1)
                |(video_std->v_vin_yc_mux<<0);
        m26->mb_write(0x1800, tmp);

        m26->mb_write(0x1802, video_std->v_vin_top_start_line);
        m26->mb_write(0x1804, video_std->v_vin_bot_start_line);
        m26->mb_write(0x1806, video_std->v_vin_max_h_count);
        m26->mb_write(0x1808, video_std->v_vin_max_v_count);
        m26->mb_write(0x180A, video_std->v_vin_h_valid_pos-1);
        m26->mb_write(0x180C, video_std->v_vin_top_valid_line);
        m26->mb_write(0x180E, video_std->v_vin_bot_valid_line);
        m26->mb_write(0x1810, video_std->v_vin_valid_hcount);
        m26->mb_write(0x1812, video_std->v_vin_valid_vcount);
        

        m26->mb_write(0x1510, 0x0311
                | (vlc_mode<<3)
                | (gop_mode<<5)
        );
        m26->mb_write(0x1514, 0x0100);
        m26->mb_write(0x151A, 0x0000);
        m26->mb_write(0x151E, 0x0000);
        m26->mb_write(0x1104, mb_system_bitrate);
        m26->mb_write(0x1532, mb_bitrate);
        m26->mb_write(0x1534, mb_v_max_bitrate);
        m26->mb_write(0x1544, mb_v_max_bitrate);
        m26->mb_write(0x1536, mb_v_ave_bitrate);
        m26->mb_write(0x1546, mb_v_ave_bitrate);
        m26->mb_write(0x1538, mb_v_min_bitrate);
        m26->mb_write(0x1548, mb_v_min_bitrate);
        m26->mb_write(0x153A, mb_v_min_bitrate);
        m26->mb_write(0x1570, 0xca11);
        m26->mb_write(0x1572, 0x0000);
        if(delay_mode==DELAY_HIGH)
                m26->mb_write(0x1574, 10);
        else
                m26->mb_write(0x1574, 3);
        m26->mb_write(0x157C, 0x0000);
        m26->mb_write(0x157E, 0x1050);
        m26->mb_write(0x1580, 0x0000);
        m26->mb_write(0x1584, 0);
        m26->mb_write(0x1586, 50);
        m26->mb_write(0x1588, 1);
        m26->mb_write(0x158A, 0x0000);
        m26->mb_write(0x1130, 0x00e0);

        //profile level
        m26->mb_write(0x153C, (h264_profile<<8) | h264_level);
        m26->mb_write(0x1526, (h264_profile<<8) | h264_level);

        // width height
        if(v_ip_format == V_FORMAT::NATIVE){
                m26->mb_write(0x152C, 0);
                m26->mb_write(0x152E, 0);

                m26->mb_write(0x1540, 0);
                m26->mb_write(0x1542, 0);
        } else {
                m26->mb_write(0x152C, get_std_width(output_video_std));
                m26->mb_write(0x152E, get_std_height(output_video_std));

                m26->mb_write(0x1540, get_std_width(output_video_std));
                m26->mb_write(0x1542, get_std_height(output_video_std));
        }
        

        if(v_ip_format == V_FORMAT::NATIVE){
                if(is_interlaced())
                        tmp = 0;
                else
                        tmp = 2;
        }else{
              tmp = 1;  
        }
        m26->mb_write(0x1512, tmp<<8);
        //no null packet
        m26->mb_write(0x1102, 0x4000 | (auto_null<<0));
        m26->mb_write(0x1134, 0x1FFF);
        // gop
        m26->mb_write(0x1518, 0x0000 
                | (gop_size<<8)
                | (v_gop_struct<<0)
        );

        
        if(audio_format==0){
                // mpeg-2
                len = gen_es_info(pid_audio, pid_video, pid_program, 0, pid_teletext, 
                        pid_pcr,
                        teletext_enable, 
                        aux_audio_enable,
                        pid_aux_audio,
                        buf);
                m26->mb_write(0x100A, 0x0010);
                m26->mb_write(0x100C, 0x3020);
        }else{
                len = gen_es_info(pid_audio, pid_video, pid_program, 1, pid_teletext, 
                        pid_pcr,
                        teletext_enable, 
                        aux_audio_enable,
                        pid_aux_audio,
                        buf);
                m26->mb_write(0x100A, 0x0010);
                m26->mb_write(0x100C, 0x5020);
                m26->mb_write(0x1A1E, mb_a_max_bitrate);
                m26->mb_write(0x1A1C, 0x2812);
        }

        // PMT
        for(int i=0; i<len; i+=2){
                uint16_t tmp;
                tmp = buf[i];
                tmp = tmp << 8;
                tmp |= buf[i+1];
                m26->mb_write(0x1174+i, tmp);
        }

        // PAT
        int pat_len = gen_pat(pid_pmt, pid_sid, pid_program, buf);
        for(int i=0; i<pat_len; i+=2){
                uint16_t tmp;
                tmp = buf[i];
                tmp = tmp << 8;
                tmp |= buf[i+1];
                m26->mb_write(0x1144+i, tmp);
        }
        m26->mb_write(0x1138, (pat_len<<8) | len);
        
        //IDR
        m26->mb_write(0x151A, idr_interval/100);

        //A
        m26->mb_write(0x1A08, 0x10C0);
        m26->mb_write(0x1A04, mb_a_bitrate);
        m26->mb_write(0x1A16, 0x0000);

        // low delay
        if(ldmode){
                m26->mb_write(0x1508, 0x0000);
                m26->mb_write(0x150A, 50);

                m26->mb_write(0x158c, 0x0000);

                m26->mb_write(0x1518, 0x0000 
                        | (0<<8)
                        | (1<<0)
                );
                m26->mb_write(0x1510, 0x0309);
                m26->mb_write(0x1584, 0);
                m26->mb_write(0x151A, 0);
                m26->mb_write(0x1526, (1<<8) | 40);
        }
}

void mb86m26_control::mb_encoder_firmware(libusb_device_handle * devh)
{
        if(devh==NULL)
                return;
        Q_CHECK_PTR(devh);
        qCDebug(category) << "Encoder firmware";
        mb_cmd(devh, 0x0400, 0x0000);
        
        if(!ldmode){
                m26->mb_firmware_ep2(devh, enc_file);
                mb_cmd(devh, 0x04a0, 0x0000);
        }
        else{
                m26->mb_firmware_ep2(devh, ldenc_file);
                mb_cmd(devh, 0x04C0, 0x0000);
        }
                
}

void mb86m26_control::mb_encoder_cmd(libusb_device_handle * devh)
{
        if(devh==NULL)
                return;
        Q_CHECK_PTR(devh);
        //encode cmd
        mb_cmd(devh, 0x0500, 0x0002);
        state_timeount_start();
}

void mb86m26_control::stop()
{
        mb_endcode_stop(m26->mb_devh);
        thread_exit_flag = 1;
        m26->stop();
}

void mb86m26_control::mb_endcode_stop(libusb_device_handle * devh)
{
        printf("Null output cmd\n");
        mb_cmd(devh, 0x0500, 0x0004);
        mb_wait_irq_ack(devh);
        printf("STOP cmd\n");
        mb_cmd(devh, 0x0500, 0x0001);
        mb_wait_irq_ack(devh);
}

void mb86m26_control::interrupt()
{
        uint16_t value;

        m26->mb_read(MB_REG_API_IRQST, &value);
        m26->mb_write(MB_REG_API_IRQST, value);
        if(value&IRQ_MASK_STATE){
                mb_handle_state(m26->mb_devh);
        }
        
}

void mb86m26_control::stream_reset()
{
        buffer_index = 0;
}

mb86m26_control::pbx_std_name_t * mb86m26_control::get_std_info()
{
        for (int i=0; ; i++){
                if(m26_std_info[i].id==-1)
                        break;
                if(m26_std_info[i].id==input_video_std)
                        return &m26_std_info[i];
        }
        return &m26_std_info[0];
}

void mb86m26_control::state_timeount_start()
{
        state_timeout_timer->start();
}

void mb86m26_control::state_timeout()
{
        qCDebug(category) << "State transition timeout";
        QTimer::singleShot(1000, this, SLOT(mb_restart_request()));
}

void mb86m26_control::set_bitrate(uint32_t video, uint32_t audio, uint32_t system)
{
        int r = 0;
        if(this->video_bitrate!=video)
                r = 1;
        if(this->audio_bitrate!=audio)
                r = 1;
        if(this->mb_system_bitrate!=system)
                r = 1;
        this->video_bitrate = video;
        this->audio_bitrate = audio;
        this->mb_system_bitrate = system;
        check_system_bitrate(system);
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_v_format(V_FORMAT value)
{
        int r = 0;
        if(value==v_ip_format)
                r = 1;
        v_ip_format = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_auto_null(int value)
{
        int r = 0;
        if(value==auto_null)
                r = 1;
        auto_null = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_h264_profile(PROFILE value)
{
        int r = 0;
        if(value==h264_profile)
                r = 1;
        h264_profile = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_gop(GOP value)
{
        int r = 0;
        if(value==v_gop_struct)
                r = 1;
        v_gop_struct = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_h264_level(LEVEL value)
{
        int r = 0;
        if(value==h264_level)
                r = 1;
        h264_level = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_a_format(AUDIO value)
{
        int r = 0;
        if(this->audio_format!=value)
                r = 1;
        audio_format = value;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_std(int std)
{
        this->input_video_std = std;
}

int mb86m26_control::is_native(out_std std)
{
        int ret = 0;

        switch(input_video_std){
        case STD_1080p25:
        case STD_1080i50:
                if(std<=STD_OUT_1080)
                        ret = 1;
                break;
        case STD_720p50:
                if(std<=STD_OUT_720)
                        ret = 1;
                break;
        case STD_625i50:
                if(std<=STD_OUT_480)
                        ret = 1;
                break;
        }
        return ret;
}

int mb86m26_control::is_interlaced()
{
        int ret = 0;

        switch(input_video_std){
        case STD_1080i50:
                ret = 1;
                break;
        case STD_720p50:
                ret = 0;
                break;
        case STD_625i50:
                ret = 1;
                break;
        }
        return ret;
}

void mb86m26_control::set_pid(uint16_t pid_video, uint16_t pid_audio, 
                uint16_t pid_pmt, uint16_t pid_program,
                uint16_t pid_sid, uint16_t pid_pcr, uint16_t pid_teletext)
{
        if((pid_video == pid_audio) || (pid_pmt == pid_video) || (pid_pmt == pid_audio))
                return;
        this->pid_video = pid_video;
        this->pid_audio = pid_audio;
        this->pid_pmt = pid_pmt;
        this->pid_sid = pid_sid;
        this->pid_program = pid_program;
        this->pid_teletext = pid_teletext;
        this->pid_pcr = pid_pcr;
}

void mb86m26_control::set_idr_interval(int value)
{
        if((value < 1000) && (value != 0))
                value = 1000;
        this->idr_interval = value;
}

void mb86m26_control::set_sd_aspect(SD_ASPCET value)
{
        this->sd_aspect = value;
}

void mb86m26_control::set_output_std(out_std std)
{
        int r = 0;
        if(this->output_video_std!=std)
                r = 1;
        output_video_std = std;
        if(r)
                mb_restart_request();
}

void mb86m26_control::set_output_std(int std)
{
        set_output_std((out_std) std);
}

int mb86m26_control::get_std_width(mb86m26_control::out_std std)
{
        int ret = 0;

        if(is_native(std))
                return 0;

        switch(std){
        case STD_OUT_1080:
                ret = 1920;
                break;
        case STD_OUT_720:
                ret = 1280;
                break;
        case STD_OUT_480:
                ret = 854;
                break;
        case STD_OUT_360:
                ret = 640;
                break;
        case STD_OUT_240:
                ret = 426;
                break;
        }

        return ret;
}

int mb86m26_control::get_std_height(mb86m26_control::out_std std)
{
        int ret = 0;

        if(is_native(std))
                return 0;

        switch(std){
        case STD_OUT_1080:
                ret = 1080;
                break;
        case STD_OUT_720:
                ret = 720;
                break;
        case STD_OUT_480:
                ret = 480;
                break;
        case STD_OUT_360:
                ret = 360;
                break;
        case STD_OUT_240:
                ret = 240;
                break;
        }

        return ret;
}

int mb86m26_control::get_state()
{
        return m26_state;
}

void mb86m26_control::set_teletext_enable(int value)
{
        teletext_enable = value;
}

void mb86m26_control::set_vlc_mode(VLC_MODE value)
{
        vlc_mode = value;
}

void mb86m26_control::set_gop_mode(GOP_MODE value)
{
        gop_mode = value;
}

void mb86m26_control::set_gop_size(int value)
{
        gop_size = check_gop_size(value);
}

int mb86m26_control::check_gop_size(int value)
{
        if(value < 6)
                value = 6;
        else if(value>63)
                value = 63;
        value = (value/3)*3;
        return value;
}

void mb86m26_control::set_delay_mode(DELAY_MODE value)
{
        delay_mode = value;
        if(delay_mode==DELAY_LOW)
                ldmode = 1;
        else
                ldmode = 0;
}

void mb86m26_control::set_aux_audio_enable(int value)
{
        aux_audio_enable = value;
}

void mb86m26_control::set_pid_aux_audio(int value)
{
        pid_aux_audio = value;
}

void mb86m26_control::set_aux_audio_bitrate(int value)
{
        aux_audio_bitrate = value;
}

mb86m26_control::mb86m26_control(QObject *parent, QString reset_gpio)
{
        packet_buffer = (uint8_t*) malloc(188);
        buffer_index = 0;
        video_bitrate = 6000;
        audio_bitrate = 128;
        mb_system_bitrate = 7000;
        output_video_std = STD_OUT_1080;
        v_ip_format = V_FORMAT::NATIVE;
        auto_null = 1;
        h264_profile = PROFILE::H264_HIGH;
        h264_level = LEVEL::LVL_41;
        v_gop_struct = GOP::IBBP;
        audio_format = AUDIO::AAC;
        pid_video = 8176;
        pid_audio = 8177;
        pid_pmt = 8170;
        pid_sid = 0;
        pid_program = 1;
        idr_interval = 2000;
        sd_aspect = SD_ASPCET::ASPECT_4_3;
        m26_state = 0;
        teletext_enable = 0;
        vlc_mode = CABAC;
        gop_mode = GOP_OPEN;
        gop_size = 15;
        ldmode = 0;
        delay_mode = DELAY_HIGH;
        aux_audio_enable = 1;
        pid_aux_audio = 999;
        aux_audio_bitrate = 128;

        m26 = new MB86M26(NULL, reset_gpio);
        QObject::connect(m26, &MB86M26::hotplug, this, &mb86m26_control::hotplug);
        QObject::connect(m26, &MB86M26::interrupt, this, &mb86m26_control::interrupt);

        set_std(STD_1080i50);
        set_output_std(STD_OUT_1080);

        state_timeout_timer = new QTimer(this);
        state_timeout_timer->setSingleShot(true);
        state_timeout_timer->setInterval(2000);
        QObject::connect(state_timeout_timer, &QTimer::timeout, this, &mb86m26_control::state_timeout);
        thread_exit_flag = 0;
        m26->start();
}

mb86m26_control::~mb86m26_control()
{
        stop();
        free(packet_buffer);
}
