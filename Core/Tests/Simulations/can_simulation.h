#ifndef CAN_SIMULATION_H
#define CAN_SIMULATION_H

#include <stdint.h>

// CAN frame structure
typedef struct {
    uint32_t id;           // CAN ID
    uint8_t dlc;          // Data length code
    uint8_t data[8];      // CAN data bytes
    uint32_t timestamp;    // Timestamp in ms
} CANFrame;

// Buffer configuration
#define CAN_BUFFER_SIZE 256
#define MAX_CAN_BUSES 2

// CAN simulation functions
void initCANSimulation(void);
void transmitCANFrame(uint8_t bus, CANFrame frame);
int receiveCANFrame(uint8_t bus, CANFrame* frame);
void registerCANCallback(uint8_t bus, uint32_t id, void (*callback)(CANFrame));

#endif /* CAN_SIMULATION_H */