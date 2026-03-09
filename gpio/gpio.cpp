#include "gpio.h"

#if (BOARD_REV==0)

#define PATH_TO_GPIO_IN_0 "/home/pi/gpio_from_508/gpio480/value"
#define PATH_TO_GPIO_IN_1 "/home/pi/gpio_from_508/gpio481/value"
#define PATH_TO_GPIO_IN_2 "/home/pi/gpio_from_508/gpio482/value"
#define PATH_TO_GPIO_IN_3 "/home/pi/gpio_from_508/gpio483/value"
#define PATH_TO_GPIO_IN_4 "/home/pi/gpio_from_508/gpio484/value"
#define PATH_TO_GPIO_IN_5 "/home/pi/gpio_from_508/gpio485/value"
#define PATH_TO_GPIO_IN_6 "/home/pi/gpio_from_508/gpio486/value"
#define PATH_TO_GPIO_IN_7 "/home/pi/gpio_from_508/gpio487/value"
#define PATH_TO_GPIO_IN_8 "/home/pi/gpio_from_508/gpio489/value"
#define PATH_TO_GPIO_IN_9 "/home/pi/gpio_from_508/gpio488/value"

#define PATH_TO_GPIO_OUT  "/home/pi/gpio_from_508/gpio491/value"

#else

#define PATH_TO_GPIO_IN_0 "/home/pi/gpio_from_508/gpio422/value"
#define PATH_TO_GPIO_IN_1 "/home/pi/gpio_from_508/gpio423/value"
#define PATH_TO_GPIO_IN_2 "/home/pi/gpio_from_508/gpio424/value"
#define PATH_TO_GPIO_IN_3 "/home/pi/gpio_from_508/gpio425/value"
#define PATH_TO_GPIO_IN_4 "/home/pi/gpio_from_508/gpio426/value"
#define PATH_TO_GPIO_IN_5 "/home/pi/gpio_from_508/gpio427/value"
#define PATH_TO_GPIO_IN_6 "/home/pi/gpio_from_508/gpio428/value"
#define PATH_TO_GPIO_IN_7 "/home/pi/gpio_from_508/gpio429/value"
#define PATH_TO_GPIO_IN_8 "/home/pi/gpio_from_508/gpio431/value"
#define PATH_TO_GPIO_IN_9 "/home/pi/gpio_from_508/gpio430/value"

#define PATH_TO_GPIO_OUT  "/home/pi/gpio_from_508/gpio433/value"

#endif

static QLoggingCategory category("Gpio_Class");

Gpio::Gpio()
{
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_0});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_1});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_2});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_3});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_4});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_5});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_6});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_7});

    input_SOLO_desable ={.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_8};
    input_time_counter ={.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_9};


    old_common_alarm = -1;

    timer_gpio_update = new QTimer;
    timer_gpio_update->start(100);

    gpio_mode = TALLY;

    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_time_counter);
    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_solo_mode_desebled);
    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_state);
}

void Gpio::set_mode(int mode)
{
    gpio_mode = mode;
    qDebug(category) << "New mode gpio";
}

void Gpio::slot_update_state()
{
    switch(gpio_mode){
    case SOLO:
           slot_update_SOLO_state();
        break;
    case TALLY:
           slot_update_TALLY_state();
        break;
    case PRESET:
           slot_update_PRESET_state();
        break;
    default:
           slot_update_SOLO_state();
        break;
    }
}

void Gpio::slot_update_PRESET_state()
{
    for(int i = 0; i < input.size(); ++i){
        int state = get_value(input[i].path_to_gpio);
        if(input[i].old_state && (state == 0)){
            emit signal_preset(i);
            qDebug(category) << "Preset number:" << i;
        }
        input[i].old_state = state;
    }
}

void Gpio::slot_update_solo_mode_desebled()
{
    int state = get_value(input_SOLO_desable.path_to_gpio);
    if(input_SOLO_desable.old_state != state){
        emit signal_solo_mode_desebled();
        qDebug(category) << "SOLO mode disabled";
    }
     input_SOLO_desable.old_state = state;
}


void Gpio::slot_update_time_counter()
{
    int state = get_value(input_time_counter.path_to_gpio);

    if(input_time_counter.old_state != state){
        if(state){
            emit signal_time_count_start();
            qDebug(category) << QString("Time Counter Start");
        }
        else{
            emit signal_time_count_stop();
            qDebug(category) << QString("Time Counter Stop");
        }

        input_time_counter.old_state = state;
    }
}

void Gpio::slot_update_TALLY_state()
{
    for(int i = 0; i < input.size(); ++i){
        int state = get_value(input[i].path_to_gpio);
        if(input[i].old_state != state){
            emit signal_TALLY(i, state^1);
            qDebug(category) << QString("Tally state: %1. Input: %2").arg(state).arg(i);
        }
        input[i].old_state = state;
    }
}

void Gpio::slot_update_SOLO_state()
{
    for(int i = 0; i < input.size(); ++i){
        int state = get_value(input[i].path_to_gpio);
        if(input[i].old_state && (state == 0)){
            emit signal_solo(i);
            qDebug(category) << "Solo input:" << i;
        }
        input[i].old_state = state;
    }
}


void Gpio::set_common_alarm(int common_alarm)
{
    if(old_common_alarm == common_alarm) return;
    old_common_alarm = common_alarm;

    if(common_alarm)
        set_state(PATH_TO_GPIO_OUT, "0");
    else
        set_state(PATH_TO_GPIO_OUT, "1");
}


int Gpio::get_value(QString file_name)
{
QString line;
int value;
    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug(category) << "Could not open file" << file_name;
        return -1;
    }

    line = file.readAll();
    file.close();

    value = line.toInt();

    return value;
}

void Gpio::set_state(QString file_name, const char *state)
{
    QFile file(file_name);
    if(!file.open(QIODevice::ReadWrite)){
        qDebug(category) << "Could not open file" << file_name;
    }
    file.write(state);
    file.close();
}
