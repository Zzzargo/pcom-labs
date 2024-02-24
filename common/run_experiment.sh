#!/bin/bash

SPEED=1
DELAY=1
LOSS=0
# Adjust the corruption
CORRUPT=0

# Second bit corruption rate
CORRUPT2=0

{
    pkill -9 link
    pkill -9 recv
    pkill -9 send
} &> /dev/null

./link_emulator/link speed=$SPEED delay=$DELAY loss=$LOSS corrupt=$CORRUPT \
    corrupt2=$CORRUPT2 > link_emulator.out 2> link_emulator.err &
sleep 1
./recv &
sleep 1

./send
