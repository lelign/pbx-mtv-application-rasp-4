#include "led_class.h"

const QStringList  path_to_led_sdi_list =
{
    #if (BOARD_REV==0)
    "/sys/class/gpio/gpio504/value", /* led 1 */
    "/sys/class/gpio/gpio506/value", /* led 2 */
    "/sys/class/gpio/gpio508/value", /* led 3 */
    "/sys/class/gpio/gpio510/value", /* led 4 */
    "/sys/class/gpio/gpio505/value", /* led 5 */
    "/sys/class/gpio/gpio507/value", /* led 6 */
    "/sys/class/gpio/gpio509/value", /* led 7 */
    "/sys/class/gpio/gpio511/value"  /* led 8 */
    #else
    "/sys/class/gpio/gpio439/value", /* led 1 */
    "/sys/class/gpio/gpio441/value", /* led 2 */
    "/sys/class/gpio/gpio443/value", /* led 3 */
    "/sys/class/gpio/gpio445/value", /* led 4 */
    "/sys/class/gpio/gpio447/value", /* led 5 */
    "/sys/class/gpio/gpio449/value", /* led 6 */
    "/sys/class/gpio/gpio451/value", /* led 7 */
    "/sys/class/gpio/gpio453/value"  /* led 8 */
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
