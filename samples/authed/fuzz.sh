#!/bin/sh

REPLAY_FUZZ_INDEX=5 LD_PRELOAD=../../desock-replayer/src/desock-replayer.so afl-fuzz -i inputs/ -o outputs/ -t 2000 -f ./working_packet.data -- ./authed
