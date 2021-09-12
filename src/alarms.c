
#include <stdlib.h>

#include "alarms.h"

alarmManager_t * AlarmManager( AlarmChangedCallback_t AlarmChangedCallback, void * alarmChangedCallbackData ) {
    alarmManager_t * manager = calloc( 1, sizeof(alarmManager_t) );
    
    if ( !manager )
        return NULL;

    manager->AlarmChangedCallback = AlarmChangedCallback;
    manager->alarmChangedCallbackData = alarmChangedCallbackData;
    manager->numAlarms = 0;

    return manager;
}

alarm_t * AlarmGetNext( alarmManager_t * manager ) {
    int i;
    alarm_t * next = NULL;
    
    if ( !manager )
        return NULL;
    
    for ( i = 0; i < manager->numAlarms; i++ ) {
        alarm_t * alarm = manager->alarms[i];

        if ( !next ) {
            if ( alarm ) {
                next = alarm;
            }
            continue;
        }

        if ( !alarm )
            continue;

        if ( ( alarm->when > 0 ) && ( alarm->when < next->when ) ) {
            next = alarm;
            continue;
        }
    }

    return next;
}

void AlarmTimePassed( alarmManager_t * manager, int howmuch, int runCallbacks ) {
    int i = 0;

    for ( i = 0; i < MAX_ALARMS; i++ ) {
        alarm_t * alarm = manager->alarms[i];
        
        if ( alarm && ( alarm->when >= 0 ) ) {
            alarm->when = alarm->when - howmuch;
            if ( alarm->when < 1 ) {
                alarm->when = -1;
                if ( runCallbacks ) {
                    alarm->Callback( alarm->callbackData );
                }
            }
        }
    }
}

void AlarmAdd( alarmManager_t * manager, alarm_t * alarm ) {
    int i;
    
    for ( i = 0; i < MAX_ALARMS; i++ ) {
        if ( ! manager->alarms[i] ) {
            manager->alarms[i] = alarm;
            manager->numAlarms++;
            return;
        }
    }
}