#include "led_class.h"

const QStringList  path_to_led_sdi_list =
{
    #if (BOARD_REV==0)
    "/var/volatile/gpio/led_1", /* led 1 */
    "/var/volatile/gpio/led_2", /* led 2 */
    "/var/volatile/gpio/led_3", /* led 3 */
    "/var/volatile/gpio/led_4", /* led 4 */
    "/var/volatile/gpio/led_5", /* led 5 */
    "/var/volatile/gpio/led_6", /* led 6 */
    "/var/volatile/gpio/led_7", /* led 7 */
    "/var/volatile/gpio/led_8"  /* led 8 */
    "/var/volatile/gpio/led_9", /* led 9 */
    "/var/volatile/gpio/led_10", /* led 10 */
    "/var/volatile/gpio/led_11", /* led 11 */
    "/var/volatile/gpio/led_12", /* led 12 */
    "/var/volatile/gpio/led_13", /* led 13 */
    "/var/volatile/gpio/led_14", /* led 14 */
    "/var/volatile/gpio/led_15", /* led 15 */
    "/var/volatile/gpio/led_16"  /* led 16 */
    #else
    "/var/volatile/gpio/led_1", /* led 1 */
    "/var/volatile/gpio/led_2", /* led 2 */
    "/var/volatile/gpio/led_3", /* led 3 */
    "/var/volatile/gpio/led_4", /* led 4 */
    "/var/volatile/gpio/led_5", /* led 5 */
    "/var/volatile/gpio/led_6", /* led 6 */
    "/var/volatile/gpio/led_7", /* led 7 */
    "/var/volatile/gpio/led_8"  /* led 8 */
    "/var/volatile/gpio/led_9", /* led 9 */
    "/var/volatile/gpio/led_10", /* led 10 */
    "/var/volatile/gpio/led_11", /* led 11 */
    "/var/volatile/gpio/led_12", /* led 12 */
    "/var/volatile/gpio/led_13", /* led 13 */
    "/var/volatile/gpio/led_14", /* led 14 */
    "/var/volatile/gpio/led_15", /* led 15 */
    "/var/volatile/gpio/led_16"  /* led 16 */
    #endif
};

Led_class::Led_class(QObject *parent) : QObject(parent)
{
    set_all_led_off();
}

/* -------------------------------------------------------------------- */
void Led_class::set_led_state(int num, int state)
{
    #if (BOARD_REV==0)
    if(state)
        set_state(path_to_led_sdi_list[num], "1");
    else
        set_state(path_to_led_sdi_list[num], "0");
    #else
    if(state)
        set_state(path_to_led_sdi_list[num], "0");
    else
        set_state(path_to_led_sdi_list[num], "1");
    #endif
}
/* -------------------------------------------------------------------- */
void Led_class::set_all_led_off()
{
    for(int i = 0; i < path_to_led_sdi_list.size(); i++)
        set_state(path_to_led_sdi_list[i], "0");
}

/* -------------------------------------------------------------------- */
void Led_class::set_all_led_on()
{
    for(int i = 0; i < path_to_led_sdi_list.size(); i++)
        set_state(path_to_led_sdi_list[i], "1");
}

/* -------------------------------------------------------------------- */
void Led_class::set_state(QString file_name, const char *state)
{
    QFile file(file_name);
    file.open(QIODevice::ReadWrite);
    file.write(state);
    file.close();
}
