/*
 * Copyright 1993 by the Hewlett-Packard Company.
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, MIT, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, MIT, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: gtbtnmppn.m,v 1.4 94/01/29 15:16:21 rws Exp $
 */
>>TITLE XGetDeviceButtonMapping XINPUT
int
xname()
Display	*display = Dsp;
XDevice	*device;
unsigned char	*map_return = Map;
int 	nmap = MAPSIZE;
>>EXTERN
extern ExtDeviceInfo Devs;

/*
 * The number of buttons should be between 1 and 255
 */
#define	MAPSIZE	255
static	unsigned char	Map[MAPSIZE];

>>ASSERTION Good B 3
>>#A call to xname
>>#returns the logical button numbers corresponding to the
>>#physical button i+1, where i is an index into the returned array
>>#.A map_return .
A call to xname
returns in map_return[i] the logical button numbers of the
physical buttons i+1.
>>STRATEGY
>># Can't do much apart from setting a mapping and checking it.
>># This will be done in SetDeviceButtonMapping.
Call xname to get pointer mapping.
UNTESTED touch test only.
>>CODE
int 	nbuttons;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	nbuttons = XCALL;
	report("This assertion is tested elsewhere.");
	UNTESTED;


>>ASSERTION Good B 3
A call to xname
returns the number of physical buttons actually on the pointer.
>>STRATEGY
Compare the number of buttons returned by xname and XListInputDevices.
>>CODE
int 	i, j, nbuttons, ndevices;
XDeviceInfo *list;
XAnyClassPtr any;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	nbuttons = XCALL;
	list = XListInputDevices(display, &ndevices);
	for (i=0; i<ndevices; i++,list++)
	    {
	    if (list->id==Devs.Button->device_id)
		{
		any = (XAnyClassPtr) (list->inputclassinfo);
		for (j=0; j<list->num_classes; j++)
		    {
		    if (any->class == ButtonClass)
			{
			if (((XButtonInfo *) any)->num_buttons == nbuttons)
			    PASS;
			else
			    {
			    report("# buttons returned doesn't match XListInputDevices.");
			    FAIL;
			    }
			break;
			}
	    	    any = (XAnyClassPtr) ((char *) any + any->length);
		    }
		break;
		}
	    }

>>ASSERTION Good B 3
When the
.A nmap
argument is less than the number of elements in the pointer mapping,
then only the first
.A nmap
elements are returned in
.A map_return .
>>STRATEGY
Set all elements of map_return to 255.
Set nmap to a value less than number of elements in the pointer mapping.
Call xname.
Verify that elements of map_return beyond nmap-1 are still 255.
>>CODE
int 	nbuttons;
int 	i;
#define	TEST_VAL	((unsigned char)255)

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	nbuttons = XCALL;

	for (i = 0; i < MAPSIZE; i++)
		Map[i] = TEST_VAL;

	/*
	 * Its not clear that asking for 0 elements is really sensible so we
	 * only do it when there is only one button.
	 */
	if (nbuttons == 1)
		nmap = 0;
	else
		nmap = 1;

	XCALL;

	for (i = nmap; i < MAPSIZE; i++) {
		if (Map[i] != TEST_VAL) {
			report("An element beyond the first nmap was returned");
			report(" element %d was %u, expecting %u", i, (unsigned)Map[i],
				(unsigned)TEST_VAL);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(MAPSIZE-nmap);
>>ASSERTION Bad B 3
If the specified device does not have any buttons, 
a BadMatch error is returned.
>>STRATEGY
Specify a device that is known to have no buttons.  Verify that a BadMatch
error is returned.
>>CODE BadMatch
int 	nbuttons;

	if (!Setup_Extension_DeviceInfo(NBtnsMask))
	    {
	    untested("%s: No input device without buttons.\n", TestName);
	    return;
	    }
	device = Devs.NoButtons;
	nbuttons = XCALL;

	if (geterr() == BadMatch)
		PASS;
	else {
		report("No BadMatch for device with no buttons");
		FAIL;
		}
>>ASSERTION Bad B 3
If the specified device is invalid
a BadDevice error is returned.
>>STRATEGY
Specify the X pointer device.  Verify that a BadDevice error is returned.
>>CODE baddevice
XDevice bogus;
int 	nbuttons, baddevice;

	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	BadDevice(display,baddevice);
	device = &bogus;
	device->device_id = -1;
	nbuttons = XCALL;

	if (geterr() == baddevice)
		PASS;
	else {
		report("No BadDevice for x pointer device");
		FAIL;
		}
