/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: atest.c,v $
 * Revision 1.1.1.2  1993/05/05  17:37:50  Pete_Snider
 * 	initial rcs submit
 *
 * 
 * $EndLog$
 */
#define  XLIB_ILLEGAL_ACCESS
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/extensions/sync.h>



int main(argc, argv)
int argc;
char **argv;
{
    Display *dpy, *dpy1;
    unsigned long sync_event, sync_error;
    int major_val, minor_val, count;
    unsigned long n_counters;
    SystemCounterRec *sys_counter_list = NULL;
    XEvent event;
    XCounterNotifyEvent *cev;
    XAlarmNotifyEvent *aev;
    Counter counter = 0;
    Counter counter1 = 0;
    unsigned long cntr_value;
    unsigned long cntr_value1;
    Alarm alarm=0;
    Alarm alarm1=0;
    Alarm alarm2=0;
    char *counter_name = "SERVERTIME";
    unsigned long values_mask;
    XSyncAlarmAttributes values;
    unsigned long delta = 0;
    unsigned long wait_value = 0;
    unsigned long limit = 0;
    int n_conditions;
    XWaitCondition wait_list;

    dpy = XOpenDisplay(NULL);

    if (!dpy) {
	fprintf(stderr, "Failed to open display!\n");
	exit(-1);
    }

  /* Should QueryExtension also return versions, maj. & minor */

    if (!XSyncQueryExtension(dpy, &sync_event, &sync_error)) {
	fprintf(stderr, "Failed to find SYNC extension\n");
	exit(-1);
    }

    XSyncInfo(dpy, &major_val, &minor_val, &sys_counter_list, &n_counters);

    printf("XSync information \"%s\":\n", DisplayString(dpy));

    printf("  Major version:       %d\n", major_val);
    printf("  Minor version:       %d\n", minor_val);
    printf("  First event number:  %d\n", sync_event);
    printf("  First error number:  %d\n", sync_error);
    printf("  Number of counters:  %d\n", n_counters);

    if (alarm == 0) {
	if (counter==0) {
	    for (count=n_counters-1; count>=0; count--) {
		if (strcmp(sys_counter_list[count].name, counter_name)==0) {
		    break;
		}
	    }
	    if (count<0) {
		fprintf(stderr, "Failed to find counter \"%s\"\n",
			counter_name);
		exit(1);
	    }
	    counter = sys_counter_list[count].counter;
	    printf("  Found counter \"%s\", id: 0x%x with resolution %.8X%.8X\n",
		   sys_counter_list[count].name, sys_counter_list[count].counter,
		   C64High32(sys_counter_list[count].resolution),
		   C64Low32(sys_counter_list[count].resolution));
	} else {
	    printf("Using counter %d\n", counter);
	}
    }

    XSyncCreateCounter(dpy, &counter, 1L); 
    XSyncQueryCounter(dpy, counter, &cntr_value);

    printf("  Created counter: 0x%lx value: %d 0x%lx\n",
	counter, cntr_value, cntr_value); 

    XSyncCreateCounter(dpy, &counter1, 10L); 
    XSyncQueryCounter(dpy, counter1, &cntr_value1);

    printf("  Created counter 1: 0x%lx value: %d 0x%lx\n",
	counter, cntr_value1, cntr_value1); 


    XSyncChangeCounter(dpy, counter, 0x0ffefdf);
    
    XSyncChangeCounter(dpy, counter1, 0x0a5a5a5a5aa5a5);
    printf("\n  Changing counter: 0x%lx value by 0x0ffefdf\n",
	counter);
    XSyncQueryCounter(dpy, counter, &cntr_value);
    printf("  Change counter: 0x%lx value: %d 0x%lx\n",
	counter, cntr_value, cntr_value); 
    printf("  Changing counter 1: 0x%lx value by 0x0a5a5a5a5aa5a5\n",
	counter1);
    XSyncQueryCounter(dpy, counter1, &cntr_value1);
    printf("  Change counter 1: 0x%lx value: %d 0x%lx\n",
	counter1, cntr_value1, cntr_value1); 

    XSyncSetCounter(dpy, counter, 0x012345678);
    XSyncSetCounter(dpy, counter1, 0xfedcba9876543210);

    XSyncQueryCounter(dpy, counter, &cntr_value);
    printf("\n  Set counter: 0x%lx value: %d 0x%lx\n",
	counter, cntr_value, cntr_value); 

    XSyncQueryCounter(dpy, counter1, &cntr_value1);
    printf("  Set counter 1: 0x%lx value: %d 0x%lx\n",
	counter1, cntr_value1, cntr_value1); 

    XSyncCreateAlarm(dpy, &alarm, NULL, NULL);
    XSyncQueryAlarm(dpy, alarm, &values);

    printf("\n  Create default alarm: 0x%lx counter: 0x%lx\n",
	alarm, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    values_mask = CACounter|CAValueType|CAValue|CATestType|CADelta|CAEvents;
    values_mask = CAValueType|CAValue|CATestType|CADelta|CAEvents;

    values.trigger.counter = 0;
    values.trigger.wait_type = 1; /* SyncRelative; */
    values.trigger.wait_value = 1000L;
    values.trigger.test_type = 3; /* NegativeComparison;*/
    values.delta = 200L;
    values.events = 1;

    printf("  Creating new alarm: 0x%lx counter: 0x%lx\n",
	alarm1, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    XSyncCreateAlarm(dpy, &alarm1, values_mask, &values);
    XSyncQueryAlarm(dpy, alarm1, &values);

    printf("  Create alarm 1: 0x%lx counter: 0x%lx\n",
	alarm1, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    values_mask = CAValueType|CAValue|CATestType|CADelta|CAEvents;

    values.trigger.counter = 0;
    values.trigger.wait_type = 2;
    values.trigger.wait_value = 0x2000000000L;
    values.trigger.test_type = 2; /* NegativeComparison;*/
    values.delta = 6L;
    values.events = 1;

    printf("\n  Changing alarm 1: 0x%lx counter: 0x%lx\n",
	alarm1, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    XSyncChangeAlarm(dpy, alarm1, values_mask, &values);
    values.trigger.wait_value = values.delta = 0;
    XSyncQueryAlarm(dpy, alarm1, &values);

    printf("  Changed alarm 1: 0x%lx counter: 0x%lx\n",
	alarm1, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n\n", 
	values.delta, values.events, values.state);

    /* alarm w/system counter */ 
    values.trigger.counter = sys_counter_list[count].counter;
    values.trigger.wait_type = 2; /* ; */
    values.trigger.wait_value = 500L;
    values.trigger.test_type = 1; /* NegativeComparison;*/
    values.delta = 20000L;  /* approx. 20 secs. between alarms. */
    values.events = 1;

    values_mask = CACounter|CAValueType|CAValue|CATestType|CADelta|CAEvents;

    printf("  Creating new alarm with system counter: 0x%lx counter: 0x%lx\n",
	alarm2, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    XSyncQueryCounter(dpy, sys_counter_list[count].counter, &cntr_value);

    printf("  query system counter: 0x%lx value: %d 0x%lx\n",
	sys_counter_list[count].counter, cntr_value, cntr_value); 

    XSyncCreateAlarm(dpy, &alarm2, values_mask, &values);
    XSyncQueryAlarm(dpy, alarm2, &values);

    printf("  Create alarm 2: 0x%lx counter: 0x%lx\n",
	alarm2, values.trigger.counter);
    printf("\tvalue_type: 0x%lx value: 0x%lx test_type: 0x%lx\n",
	values.trigger.wait_type, values.trigger.wait_value, 
	values.trigger.test_type);
    printf("\tdelta: 0x%lx events: 0x%lx state: 0x%lx\n", 
	values.delta, values.events, values.state);

    n_conditions = 1;
    wait_list.counter = counter;
    wait_list.wait_type = SyncAbsolute;
    wait_list.wait_value = 0x12345678;
    wait_list.test_type = PositiveComparison;

    XSyncAwait(dpy, &wait_list, n_conditions);


    while (True) {
        XNextEvent(dpy, &event);
        if (event.type == sync_event + CounterNotify) {
	    cev = (XCounterNotifyEvent *)&event;

	    printf(" CounterNotify: counter: %x wait value: 0x%lx ",
		   cev->counter, cev->counter_value);
	    printf(" wait_value: 0x%lx\n", cev->counter_value); 

            printf("DEBUG: counter event rec, write the event rout.\n");

	    XSyncDestroyAlarm(dpy, alarm1);
	    continue;
        }

        if (event.type == sync_event + AlarmNotify) {
            aev = (XAlarmNotifyEvent *)&event;
            printf("  alarm 0x%x: value: 0x%lx, ",
                   aev->alarm, aev->alarm_value);
            printf("counter 0x%lx ",aev->counter_value);
            printf("time 0x%x, ", aev->time);
            switch(aev->state) {
	    case AlarmActive:
		printf("Active\n");
		break;
            case AlarmInactive:
                printf("Inactive\n");
                break;
            case AlarmDestroyed:
                printf("Destroyed\n");
/*		exit(0); */
                break;
	    default:
                printf("Unknown state %d\n", aev->state);
	  }
        }
	else 
	     printf("unknown event type: %d 0x%x\n", event.type);
    }
}
