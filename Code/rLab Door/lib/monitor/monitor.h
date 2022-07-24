#ifndef MONITOR_H
#define MONITOR_H

#define LOOP_FREQ   (50)  // How many times per second should the main loop run?
#define DOOR_OPEN_MAX_MS    (5000)

void monitorTask( void * pvParameters);


#endif