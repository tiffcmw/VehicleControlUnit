#include "../../Inc/Sensors/Sensor.h"
#include "../../Inc/Utils/Updateable.h"

void initSensor(Sensor* sensor, const char* name, int hz, SensorType type, void* child) {
    initUpdateable(&sensor->updateable, name, hz, SENSOR, sensor);
    sensor->type = type;
    sensor->child = child;
}
