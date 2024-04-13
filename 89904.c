void SimpleSoftOMXComponent::onChangeState(OMX_STATETYPE state) {
    CHECK_EQ((int)mState, (int)mTargetState);

 switch (mState) {
 case OMX_StateLoaded:
            CHECK_EQ((int)state, (int)OMX_StateIdle);
 break;
 case OMX_StateIdle:
            CHECK(state == OMX_StateLoaded || state == OMX_StateExecuting);
 break;
 case OMX_StateExecuting:
 {
            CHECK_EQ((int)state, (int)OMX_StateIdle);

 for (size_t i = 0; i < mPorts.size(); ++i) {
                onPortFlush(i, false /* sendFlushComplete */);
 }

            mState = OMX_StateIdle;
            notify(OMX_EventCmdComplete, OMX_CommandStateSet, state, NULL);
 break;
 }

 default:
            TRESPASS();
 }

    mTargetState = state;

    checkTransitions();
}
