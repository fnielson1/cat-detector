#pragma once


void connectToWifiAndTransmitSignal();
void setupWifi();

unsigned long getTimeSinceLastIrSignal();
void setTimeSinceLastIrSignal(unsigned long time);