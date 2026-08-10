#ifndef STATE_FUNC_H

#define STATE_FUNC_H

typedef void (*state_func)(void);
typedef enum {
    STATE_IDLE = 0,
    STATE_RUNNING = 1,
    STATE_ALARM= 2,
}SystemState ;

#ifdef __cplusplus
extern "C" {
#endif

void state_running_run(void);
void state_alarm_run(void);
void state_idle_run(void);
 
extern SystemState current_state;
extern const state_func state_table[]; 
#ifdef __cplusplus
}
#endif

#endif 