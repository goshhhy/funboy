
#define MAX_ALARMS 32

typedef void (AlarmChangedCallback_t)(void * data);
typedef int (AlarmGetTimePassedCallback_t)(void * data);
typedef unsigned long (AlarmGetFrameTimeCallback_t)(void * data);

typedef struct alarm_s {
    char * name;
    void * callbackData;
    void (*Callback)(void * data);
    long when;
} alarm_t;

typedef struct alarmManager_s {
    void * alarmChangedCallbackData;
    void * alarmGetTimePassedCallbackData;
    void * alarmGetFrameTimeCallbackData;
    AlarmChangedCallback_t * AlarmChangedCallback;
    AlarmGetTimePassedCallback_t * AlarmGetTimePassedCallback;
    AlarmGetFrameTimeCallback_t * AlarmGetFrameTimeCallback;
    alarm_t *alarms[32];
    int numAlarms;
} alarmManager_t;

alarmManager_t * AlarmManager( AlarmChangedCallback_t AlarmChangedCallback, void * alarmChangedCallbackData );

void AlarmAdd( alarmManager_t * manager, alarm_t * alarm );
alarm_t * AlarmGetNext( alarmManager_t * manager );
void AlarmTimePassed( alarmManager_t * manager, int howmuch, int runCallbacks );