#!/bin/sh
openocd -s "/Applications/GNU ARM Eclipse/OpenOCD/0.10.0-201701241841" -f scripts/interface/stlink-v2.cfg -f scripts/target/stm32f0x.cfg
