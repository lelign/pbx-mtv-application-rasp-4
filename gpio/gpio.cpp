#include "gpio.h"

#if (BOARD_REV==0)

#define PATH_TO_GPIO_IN_0 "/var/volatile/gpio/SOLO_IN_1"
#define PATH_TO_GPIO_IN_1 "/var/volatile/gpio/SOLO_IN_2"
#define PATH_TO_GPIO_IN_2 "/var/volatile/gpio/SOLO_IN_3"
#define PATH_TO_GPIO_IN_3 "/var/volatile/gpio/SOLO_IN_4"
#define PATH_TO_GPIO_IN_4 "/var/volatile/gpio/SOLO_IN_5"
#define PATH_TO_GPIO_IN_5 "/var/volatile/gpio/SOLO_IN_6"
#define PATH_TO_GPIO_IN_6 "/var/volatile/gpio/SOLO_IN_7"
#define PATH_TO_GPIO_IN_7 "/var/volatile/gpio/SOLO_IN_8"
#define PATH_TO_GPIO_IN_8 "/var/volatile/gpio/SOLO_IN_9"
#define PATH_TO_GPIO_IN_9 "/var/volatile/gpio/SOLO_IN_10"
#define PATH_TO_GPIO_IN_10 "/var/volatile/gpio/SOLO_IN_11"
#define PATH_TO_GPIO_IN_11 "/var/volatile/gpio/SOLO_IN_12"
#define PATH_TO_GPIO_IN_12 "/var/volatile/gpio/SOLO_IN_13"
#define PATH_TO_GPIO_IN_13 "/var/volatile/gpio/SOLO_IN_14"
#define PATH_TO_GPIO_IN_14 "/var/volatile/gpio/SOLO_IN_15"
#define PATH_TO_GPIO_IN_15 "/var/volatile/gpio/SOLO_IN_16"


#define PATH_TO_GPIO_OUT  "/var/volatile/gpio/COMMON_ALARM"
#define SOLO_DISABLE  "/var/volatile/gpio/SOLO_DISABLE"
#define TIME_COUNTER "/var/volatile/gpio/TIME_COUNTER"

#define LED_HPS_B  "/gpio/gpio517/value" // ign
//#define LED_HPS_A  "/gpio/gpio518/value" // ign

#else

#define PATH_TO_GPIO_IN_0 "/var/volatile/gpio/SOLO_IN_1"
#define PATH_TO_GPIO_IN_1 "/var/volatile/gpio/SOLO_IN_2"
#define PATH_TO_GPIO_IN_2 "/var/volatile/gpio/SOLO_IN_3"
#define PATH_TO_GPIO_IN_3 "/var/volatile/gpio/SOLO_IN_4"
#define PATH_TO_GPIO_IN_4 "/var/volatile/gpio/SOLO_IN_5"
#define PATH_TO_GPIO_IN_5 "/var/volatile/gpio/SOLO_IN_6"
#define PATH_TO_GPIO_IN_6 "/var/volatile/gpio/SOLO_IN_7"
#define PATH_TO_GPIO_IN_7 "/var/volatile/gpio/SOLO_IN_8"
#define PATH_TO_GPIO_IN_8 "/var/volatile/gpio/SOLO_IN_9"
#define PATH_TO_GPIO_IN_9 "/var/volatile/gpio/SOLO_IN_10"
#define PATH_TO_GPIO_IN_10 "/var/volatile/gpio/SOLO_IN_11"
#define PATH_TO_GPIO_IN_11 "/var/volatile/gpio/SOLO_IN_12"
#define PATH_TO_GPIO_IN_12 "/var/volatile/gpio/SOLO_IN_13"
#define PATH_TO_GPIO_IN_13 "/var/volatile/gpio/SOLO_IN_14"
#define PATH_TO_GPIO_IN_14 "/var/volatile/gpio/SOLO_IN_15"
#define PATH_TO_GPIO_IN_15 "/var/volatile/gpio/SOLO_IN_16"


#define PATH_TO_GPIO_OUT  "/var/volatile/gpio/COMMON_ALARM"
#define SOLO_DISABLE  "/var/volatile/gpio/SOLO_DISABLE"
#define TIME_COUNTER "/var/volatile/gpio/TIME_COUNTER"

#define LED_HPS_B  "/gpio/gpio517/value" // ign
//#define LED_HPS_A  "/gpio/gpio518/value" // ign

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
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_8});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_9});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_10});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_11});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_12});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_13});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_14});
    input.append({.old_state = 1, .path_to_gpio = PATH_TO_GPIO_IN_15});

    input_SOLO_desable ={.old_state = 1, .path_to_gpio = SOLO_DISABLE};
    input_time_counter ={.old_state = 1, .path_to_gpio = TIME_COUNTER};


    old_common_alarm = -1;

    timer_gpio_update = new QTimer;
    timer_gpio_update->start(100);

    gpio_mode = TALLY;

    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_time_counter);
    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_solo_mode_desebled);
    connect(timer_gpio_update, &QTimer::timeout, this, &Gpio::slot_update_state);

    /*
    
    
    */


    timer_led_hps_b = new QTimer; // ign
    timer_led_hps_b->start(500); // ign
    connect(timer_led_hps_b, &QTimer::timeout, this, &Gpio::slot_led_hps_b); // ign


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

/*
void Gpio::slot_led_hps_a(){
    int state = get_value(LED_HPS_A);
    if(state == 0){
        set_state(LED_HPS_A, "1");
        qDebug(category) << "LED_HPS_A OFF";
    }else{
        set_state(LED_HPS_A, "0");
    }

}
*/


void Gpio::slot_led_hps_b(){
    int state = get_value(LED_HPS_B);
    if(state == 0){
        set_state(LED_HPS_B, "1");
        //qDebug(category) << "LED_HPS_B OFF";
    }else{
        set_state(LED_HPS_B, "0");
        //qDebug(category) << "LED_HPS_B ON";
    }

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
