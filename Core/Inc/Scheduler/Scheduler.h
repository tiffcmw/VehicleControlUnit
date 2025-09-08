#ifndef RENSSELAERMOTORSPORT_SCHEDULER_H
#define RENSSELAERMOTORSPORT_SCHEDULER_H

#include "../Utils/Updateable.h"
#include "Task.h"

#define MAX_SENSORS 32
#define MAX_HZ 1000

typedef struct {
    int running;  // Changed from bool to int for C compatibility
} Scheduler;

void SchedulerInit(Scheduler* scheduler, Updateable* updatableArray[]);
void SchedulerRun(Scheduler* scheduler);
void SchedulerStop(Scheduler* scheduler);

// Additional utility functions
void SchedulerSuspendUpdateable(const char* name);
void SchedulerResumeUpdateable(const char* name);
void SchedulerGetStats(void);
void SchedulerCleanup(void);  // Add this new function

#endif // RENSSELAERMOTORSPORT_SCHEDULER_H
