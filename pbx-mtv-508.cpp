 #include "pbx-mtv-508.h"
#include "str-system/video_info.h"
#include "board_config.h"

static QLoggingCategory category("PbxMtv508");

extern "C" {
#include "profitt-security/profitt-security.h"
}

void data_ready(uint8_t * data, int len, void * ctx)
{
    PbxMtv508 * ptr = (PbxMtv508*) ctx;
    //for(int i=0; i<8; i++)
        //ptr->hlsserver->add_packet((char*) data+(188+4)*i+4, 188);
    free(data);
}

//PbxMtv508::PbxMtv508(bool watchdog)
//PbxMtv508::PbxMtv508()
PbxMtv508::PbxMtv508(bool watchdog)
{
    qDebug() << "Program start";

    //this->watchdog = new Watchdog(watchdog);

    //hdmi_adv7513 = new Hdmi_adv7513();

    mtvsystem = new PbxMtvSystem;
    // connect(mtvsystem->anc_reader, &AncReader::scte_104_data, this, &PbxMtv508::slot_scte_104_data);
    // connect(mtvsystem->anc_reader, &AncReader::op47_data, this, &PbxMtv508::slot_op47_data);
    // connect(mtvsystem->anc_reader, &AncReader::op42_data, this, &PbxMtv508::slot_op42_data);
    gpio = new Gpio;
    //teletext_decoder = new TeletextDecoder;

    //connect(teletext_decoder, &TeletextDecoder::txtpage, this, &PbxMtv508::slot_txt_page);

    eventlog = new Eventlog;
    eventlog->add(Eventlog::SYSTEM, "--- Start the program ---");

    m26_control = new mb86m26_control(this, QString(M26_GPIO));
   // hlsserver = new HlsServer("/var/volatile/hls/");

    layout = new Layout(mtvsystem, m26_control, gpio, eventlog);


    hardware_diagnostics = new Hardware_diagnostics;
    // connect(hardware_diagnostics, &Hardware_diagnostics::signal_over_temperature,
     //                       this, &PbxMtv508::slot_over_temperature);
    // connect(hardware_diagnostics, &Hardware_diagnostics::signal_fan_state,
     //                       this, &PbxMtv508::slot_fan_state);

    mtv_web   = new Mtv_web(mtvsystem, hardware_diagnostics, layout);
    //mtv_web   = new Mtv_web(mtvsystem, hardware_diagnostics);
    // connect(mtv_web, &Mtv_web::signal_reconfigure, this, &PbxMtv508::slot_web_reconfigure);
    /*
    m26_control->set_std(STD_1080p25);
    m26_control->set_output_std(mb86m26_control::STD_OUT_1080);
    m26_control->set_aux_audio_enable(0);
    m26_control->set_a_format(mb86m26_control::AAC);

    m26_control->m26->set_callback(data_ready, this);
    m26_control->set_auto_null(0);
    m26_control->set_v_format(mb86m26_control::NATIVE);
    m26_control->set_bitrate(4000, 128, 10000);
    m26_control->set_pid(101, 122, 100, 1, 0, 103, 0);
    m26_control->set_gop_size(30);
    m26_control->set_gop(mb86m26_control::IBBP);
    m26_control->set_gop_mode(mb86m26_control::GOP_CLOSED);
    mtvsystem->set_dei(1);
    */
    #if (BOARD_REV==0)
    /*
    factory_defaults  = new Factory_defaults;
    // connect(factory_defaults, &Factory_defaults::signal_reset_to_factory_settings,
                        this, &PbxMtv508::slot_reset_to_factory_settings);
                        */
    #endif

    /*
    file_handle_leaks = new File_handle_leaks;
    // connect(file_handle_leaks, &File_handle_leaks::signal_too_many_open_files,
                         this, &PbxMtv508::slot_too_many_open_files);
    */

    /*
    // connect(mtvsystem->anc_reader, &AncReader::op47_data, layout, &Layout::slot_op47_data);
    // connect(mtvsystem->anc_reader, &AncReader::op42_data, layout, &Layout::slot_op47_data);
    */
    for(int i = 0; i < 8; ++i){
        //scte_104[i] = new Scte_104(i);
        //connect(scte_104[i], &Scte_104::signal_update_scte_104, layout, &Layout::slot_splice);
        //connect(scte_104[i], &Scte_104::signal_scte_104_in,  this, &PbxMtv508::slot_scte_104_in);
        //connect(scte_104[i], &Scte_104::signal_scte_104_out, this, &PbxMtv508::slot_scte_104_out);
    }

        /*
    mtv_snmp = new Mtv_snmp;
    // connect(hardware_diagnostics, &Hardware_diagnostics::signal_hardware_state,
                        mtv_snmp, &Mtv_snmp::slot_set_hardware_state);
    // connect(layout, &Layout::signal_common_alarm_changed,
                    mtv_snmp, &Mtv_snmp::slot_set_common_alarm);
    */
    device_config();
    qDebug() << "pbx-mtv-508.cpp 104 n\t\tBOARD_REV : " << BOARD_REV;
//    layout->draw_overlay();
//    layout->routing_source_video();

    #if (BOARD_REV==1)
    panel = new ProfnextFrontPanel("/dev/ttyS1");
    connect(panel, &ProfnextFrontPanel::indexed_string_event, this, &PbxMtv508::indexed_string_event);
    union{
            char data[VAL_MAX_SIZE];
            uint16_t val;
    } profnext_test_status;
    T_NODE_DYN_ATTR cmd_attr;
    //статус модуля в profnext
    profnext_test_status.val = PERIPH_STATUS_OK;
    panel->cmd_profnext(TYPE_STATUS_P, STATUS_IDX, &cmd_attr, profnext_test_status.data);
    //enable input and bars for proflex header status
    panel->cmd_profnext(TYPE_STATUS_PX_P, STATUS_PX_IDX, &cmd_attr, profnext_test_status.data);

    cmd_attr.fDisabled = 0;
    profnext_test_status.val = 1;
    panel->cmd_profnext(TYPE_INDEXED_STRING_P, CMD_0, &cmd_attr, profnext_test_status.data);
    #endif
}

void PbxMtv508::indexed_string_event(uint8_t idx, uint16_t val)
{
    union{
        char data[VAL_MAX_SIZE];
        uint16_t val;
    } profnext_test_status;
    memset(&profnext_test_status.data, 0, VAL_MAX_SIZE);
    T_NODE_DYN_ATTR cmd_attr;
    cmd_attr.fDisabled = 0;


    //обработка комманды типа ресет
    if (idx == CMD_1 && val){
        qCDebug(category) << "profnext reset recieved";
        profnext_test_status.val = 0;
        //panel->cmd_profnext(TYPE_INDEXED_STRING_P, (T_CMD_NUM)idx, &cmd_attr, profnext_test_status.data);
        slot_reset_to_factory_settings();
    }
}

PbxMtv508::~PbxMtv508()
{
    //delete watchdog;
    //delete hdmi_adv7513;
    //delete m26_control;
//    delete hlsserver;
//    delete layout;
    delete mtvsystem;
    delete mtv_web;
    #if (BOARD_REV==1)
        delete panel;
    #endif
    //delete teletext_decoder;
}

/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_reset_to_factory_settings()
{
    Led_class led;
    for(int i = 0; i < 4; ++i){
        led.set_all_led_off();
        QThread::msleep(400);
        led.set_all_led_on();
        QThread::msleep(400);
    };

 //   hlsserver->delete_files_from_folder("/etc/network/", "interfaces");
  //  hlsserver->delete_files_from_folder("/etc/xdg/", "*.conf");

    secutiry_set_password("");

    qDebug(category) << "Reset to the factory Network Settings";
    qDebug(category) << "The system will reboot now!";
    system("reboot");
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::device_config()
{
    //hdmi_adv7513->adv_7513_set_color(layout->hdmi_color);
    //hdmi_adv7513->adv_7513_set_hdmi_format(Hdmi_adv7513::HDMI_HD);
}
void PbxMtv508::slot_web_reconfigure()
{
    device_config();
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_over_temperature(QString str)
{
  //  eventlog->add(Eventlog::WARNING, str);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_fan_state(int fan_state)
{
QString str;

    if(fan_state)
        str = "Fan Status: OK";
    else
        str = "Fan Status: FAULT";

    //eventlog->add(Eventlog::WARNING, str);

    send_cascade_log(Eventlog::WARNING, str);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::send_cascade_log(int category, QString str)
{
//    str += QString(" (Cascade %1)").arg(layout->cascade.num);
//    layout->eventlog_cascade(category, str);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_scte_104_data(int channel, QByteArray data)
{
    //scte_104[channel]->slot_scte_104_message(data);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_op47_data(int channel, QByteArray data)
{
    Q_UNUSED(channel);
    //teletext_decoder->add_data(data);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_op42_data(int channel, QByteArray data)
{
    Q_UNUSED(channel);
    //teletext_decoder->add_data_op42(data);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_txt_page(int page)
{
    int channel = 0;

    if(mtvsystem->get_sdi_format(channel) == 15)  return;   // 15 - loss video
//    if(!layout->op47[channel]) return;

//    if(layout->teletext_cell.enable && layout->layout_object[channel].screen_plan.enable_video && (layout->teletext_cell.page==page)){

//        //QImage image_txt = teletext_decoder->get_page(layout->teletext_cell.page);
//        QImage image_txt; // ign added
//        if(image_txt.isNull()) return;
//        layout->display_teletext(image_txt);
//    }
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_too_many_open_files(QString str)
{
  //  eventlog->add(Eventlog::WARNING, str);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_scte_104_in(int channel)
{
//    int k = layout->cascade.num * 8 + channel;
//    if(!layout->layout_object[k].cell.scte_104_display) return;

//    layout->eventlog_add_input_state("SCTE-104 Input %1: In", channel);
}
/*---------------------------------------------------------------------------*/
void PbxMtv508::slot_scte_104_out(int channel)
{
//    int k = layout->cascade.num * 8 + channel;
//    if(!layout->layout_object[k].cell.scte_104_display) return;

//    layout->eventlog_add_input_state("SCTE-104 Input %1: Out", channel);
}
