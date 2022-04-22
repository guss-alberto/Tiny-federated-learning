#ifndef __MICROPHONE_H__
#define __MICROPHONE_H__
#include "config.h"
#include "lib/init.h"


void _micInit();
void recordSample(uint16_t* buffer);

#endif
