#!/bin/sh
#---------------------------
#export ETH_CONF_FILE=
#export ETH_IRC_NICK=
#export ETH_LOG_FILE=
#---------------------------

export LD_PRELOAD=./libETH.so
./et.x86 $*
unset LD_PRELOAD
exit 0
