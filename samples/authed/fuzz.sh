#!/bin/sh

REPLAY_FUZZ_INDEX=5 LD_PRELOAD=../../desock-replayer/src/desock-replayer.so afl-fuzz -i inputs/ -o outputs/ -m none -f ./working_packet.data -- ./authed
