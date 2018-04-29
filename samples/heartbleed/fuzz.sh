#!/bin/sh

REPLAY_EXIT_ON_INJECTION=1 REPLAY_FUZZ_INDEX=18 LD_PRELOAD=../../desock-replayer/src/desock-replayer.so afl-fuzz -t 200 -i inputs/ -o outputs/ -m none -f ./working_packet.data -- ./heartbleed
