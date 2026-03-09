#include "led_class.h"

const QStringList  path_to_led_sdi_list =
{
    #if (BOARD_REV==0)
    "/home/pi/gpio_from_508/gpio504/value", /* led 1 */
    "/home/pi/gpio_from_508/gpio506/value", /* led 2 */
    "/home/pi/gpio_from_508/gpio508/value", /* led 3 */
    "/home/pi/gpio_from_508/gpio510/value", /* led 4 */
    "/home/pi/gpio_from_508/gpio505/value", /* led 5 */
    "/home/pi/gpio_from_508/gpio507/value", /* led 6 */
    "/home/pi/gpio_from_508/gpio509/value", /* led 7 */
    "/home/pi/gpio_from_508/gpio511/value"  /* led 8 */
    #else
    "/home/pi/gpio_from_508/gpio439/value", /* led 1 */
    "/home/pi/gpio_from_508/gpio441/value", /* led 2 */
    "/home/pi/gpio_from_508/gpio443/value", /* led 3 */
    "/home/pi/gpio_from_508/gpio445/value", /* led 4 */
    "/home/pi/gpio_from_508/gpio447/value", /* led 5 */
    "/home/pi/gpio_from_508/gpio449/value", /* led 6 */
    "/home/pi/gpio_from_508/gpio451/value", /* led 7 */
    "/home/pi/gpio_from_508/gpio453/value"  /* led 8 */
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
