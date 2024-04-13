ProcXkbSetDeviceInfo(ClientPtr client)
{
DeviceIntPtr		dev;
unsigned		change;
char *			wire;
xkbExtensionDeviceNotify ed;

    REQUEST(xkbSetDeviceInfoReq);
    REQUEST_AT_LEAST_SIZE(xkbSetDeviceInfoReq);

    if (!(client->xkbClientFlags&_XkbClientInitialized))
	return BadAccess;

    change= stuff->change;

    CHK_ANY_DEVICE(dev, stuff->deviceSpec, client, DixManageAccess);
    CHK_MASK_LEGAL(0x01,change,XkbXI_AllFeaturesMask);

    wire= (char *)&stuff[1];
    if (change&XkbXI_ButtonActionsMask) {
	if (!dev->button) {
	    client->errorValue = _XkbErrCode2(XkbErr_BadClass,ButtonClass);
	    return XkbKeyboardErrorCode;
	}
	if ((stuff->firstBtn+stuff->nBtns)>dev->button->numButtons) {
	    client->errorValue= _XkbErrCode4(0x02,stuff->firstBtn,stuff->nBtns,
						dev->button->numButtons);
	    return BadMatch;
	}
	wire+= (stuff->nBtns*SIZEOF(xkbActionWireDesc));
    }
    if (stuff->change&XkbXI_IndicatorsMask) {
	int status= Success;
	wire= CheckSetDeviceIndicators(wire,dev,stuff->nDeviceLedFBs,
							&status,client);
	if (status!=Success)
	    return status;
    }
    if (((wire-((char *)stuff))/4)!=stuff->length)
	return BadLength;

    bzero((char *)&ed,SIZEOF(xkbExtensionDeviceNotify));
    ed.deviceID=	dev->id;
    wire= (char *)&stuff[1];
    if (change&XkbXI_ButtonActionsMask) {
	int			nBtns,sz,i;
	XkbAction *		acts;
	DeviceIntPtr		kbd;

	nBtns= dev->button->numButtons;
	acts= dev->button->xkb_acts;
	if (acts==NULL) {
	    acts= _XkbTypedCalloc(nBtns,XkbAction);
	    if (!acts)
		return BadAlloc;
	    dev->button->xkb_acts= acts;
	}
	sz= stuff->nBtns*SIZEOF(xkbActionWireDesc);
	memcpy((char *)&acts[stuff->firstBtn],(char *)wire,sz);
	wire+= sz;
	ed.reason|=	XkbXI_ButtonActionsMask;
	ed.firstBtn=	stuff->firstBtn;
	ed.nBtns=	stuff->nBtns;

	if (dev->key)	kbd= dev;
	else		kbd= inputInfo.keyboard;
	acts= &dev->button->xkb_acts[stuff->firstBtn];
	for (i=0;i<stuff->nBtns;i++,acts++) {
	    if (acts->type!=XkbSA_NoAction)
		XkbSetActionKeyMods(kbd->key->xkbInfo->desc,acts,0);
	}
    }
    if (stuff->change&XkbXI_IndicatorsMask) {
	int status= Success;
	wire= SetDeviceIndicators(wire,dev,change,stuff->nDeviceLedFBs,
							&status,client,&ed);
	if (status!=Success)
	    return status;
    }
    if ((stuff->change)&&(ed.reason))
	XkbSendExtensionDeviceNotify(dev,client,&ed);
    return client->noClientException;
}