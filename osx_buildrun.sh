#!/bin/bash
cc main.c -o SteamNES $(pkg-config --cflags --libs sdl2)
./SteamNES  
