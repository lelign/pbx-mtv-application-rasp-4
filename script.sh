# cat /sys/kernel/debug/gpio
# leds led/led_class.cpp
GPIO=/var/volatile/gpio
rm -rf $GPIO && mkdir $GPIO
for ((i=1; i<=16; i++)); do 
echo 0 > $GPIO/LED_$i
done

echo 1 > $GPIO/reset_button # reset_button factory_defaults/factory_defaults.cpp (444)

# gpio inputs solo solo_disable time_counter gpio/gpio.cpp
for ((i=1; i<=16; i++)); do 
echo 1 > $GPIO/SOLO_IN_$i # /var/volatile/gpio/SOLO_IN_
done
echo 1 > $GPIO/COMMON_ALARM # gpio/gpio.cpp PATH_TO_GPIO_OUT (491)
echo 1 > $GPIO/SOLO_DISABLE # gpio/gpio.cpp SOLO_DISABLE (???) new var
echo 1 > $GPIO/TIME_COUNTER # gpio/gpio.cpp TIME_COUNTER (???) new var


# model_device 
echo 1 > $GPIO/MODEL_DEVICE # mtv-web/mtv_web.cpp (445)


#hardware_diagnostics
echo 1 > $GPIO/FAN_STATE # hardware_diagnostics/hardware_diagnostics.cpp PATH_TO_FAN_STATE (501)
echo 1 > $GPIO/FAN_STATE_RESET # hardware_diagnostics/hardware_diagnostics.cpp PATH_TO_FAN_STATE (502)
echo 7340000 > $GPIO/POWER1_INPUT # temp issue

# leds on board
echo 517 > /sys/class/gpio/export
echo low > /sys/class/gpio/gpio517/direction
echo 518 > /sys/class/gpio/export
echo low > /sys/class/gpio/gpio518/direction


# unexport
echo 517 > /sys/class/gpio/unexport
echo 518 > /sys/class/gpio/unexport
# rm -rf $GPIO


