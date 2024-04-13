void *Type_LUT16_Read(struct _cms_typehandler_struct* self, cmsIOHANDLER* io, cmsUInt32Number* nItems, cmsUInt32Number SizeOfTag)
{
    cmsUInt8Number InputChannels, OutputChannels, CLUTpoints;
    cmsPipeline* NewLUT = NULL;
    cmsUInt32Number nTabSize;
    cmsFloat64Number Matrix[3*3];
    cmsUInt16Number InputEntries, OutputEntries;

    *nItems = 0;

    if (!_cmsReadUInt8Number(io, &InputChannels)) return NULL;
    if (!_cmsReadUInt8Number(io, &OutputChannels)) return NULL;
    if (!_cmsReadUInt8Number(io, &CLUTpoints)) return NULL;   // 255 maximum

    if (!_cmsReadUInt8Number(io, NULL)) return NULL;

    if (InputChannels > cmsMAXCHANNELS)  goto Error;
    if (OutputChannels > cmsMAXCHANNELS) goto Error;

    NewLUT = cmsPipelineAlloc(self ->ContextID, InputChannels, OutputChannels);
    if (NewLUT == NULL) goto Error;

    if (!_cmsRead15Fixed16Number(io,  &Matrix[0])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[1])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[2])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[3])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[4])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[5])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[6])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[7])) goto Error;
    if (!_cmsRead15Fixed16Number(io,  &Matrix[8])) goto Error;


    if ((InputChannels == 3) && !_cmsMAT3isIdentity((cmsMAT3*) Matrix)) {

        if (!cmsPipelineInsertStage(NewLUT, cmsAT_END, cmsStageAllocMatrix(self ->ContextID, 3, 3, Matrix, NULL)))
            goto Error;
    }

    if (!_cmsReadUInt16Number(io, &InputEntries)) goto Error;
    if (!_cmsReadUInt16Number(io, &OutputEntries)) goto Error;

    if (InputEntries > 0x7FFF || OutputEntries > 0x7FFF) goto Error;
    if (CLUTpoints == 1) goto Error; // Impossible value, 0 for no CLUT and then 2 at least

    if (!Read16bitTables(self ->ContextID, io,  NewLUT, InputChannels, InputEntries)) goto Error;

    nTabSize = uipow(OutputChannels, CLUTpoints, InputChannels);
    if (nTabSize == (cmsUInt32Number) -1) goto Error;
    if (nTabSize > 0) {

        cmsUInt16Number *T;

        T  = (cmsUInt16Number*) _cmsCalloc(self ->ContextID, nTabSize, sizeof(cmsUInt16Number));
        if (T  == NULL) goto Error;

        if (!_cmsReadUInt16Array(io, nTabSize, T)) {
            _cmsFree(self ->ContextID, T);
            goto Error;
        }

        if (!cmsPipelineInsertStage(NewLUT, cmsAT_END, cmsStageAllocCLut16bit(self ->ContextID, CLUTpoints, InputChannels, OutputChannels, T))) {
            _cmsFree(self ->ContextID, T);
            goto Error;
        }
        _cmsFree(self ->ContextID, T);
    }


    if (!Read16bitTables(self ->ContextID, io,  NewLUT, OutputChannels, OutputEntries)) goto Error;

    *nItems = 1;
    return NewLUT;

Error:
    if (NewLUT != NULL) cmsPipelineFree(NewLUT);
    return NULL;

    cmsUNUSED_PARAMETER(SizeOfTag);
}
