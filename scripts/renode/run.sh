#!/usr/bin/env bash

RESC_STM32L051="boards/stm32l051/stm32l051.resc"
RESC_LM_MBR_1="boards/LM.MBR.1/stm32l073.resc"

RESC=$RESC_LM_MBR_1

# FIXME: Currently 'renode' is a function (or alias) in profile
#        Should think of more permanent solution
source ~/.profile

# Start renode with console (monitor) in the same TTY as this script
# without GUI (USART window) and execute stm32l0x1.resc right away
renode --console --disable-gui -e "i $RESC"
