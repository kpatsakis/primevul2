void Document::scheduleLayoutTreeUpdate()
{
    ASSERT(!hasPendingStyleRecalc());
    ASSERT(shouldScheduleLayoutTreeUpdate());
    ASSERT(needsLayoutTreeUpdate());

    page()->animator().scheduleVisualUpdate();
    m_lifecycle.ensureStateAtMost(DocumentLifecycle::VisualUpdatePending);

    TRACE_EVENT_INSTANT1(TRACE_DISABLED_BY_DEFAULT("devtools.timeline"), "ScheduleStyleRecalculation", TRACE_EVENT_SCOPE_THREAD, "data", InspectorRecalculateStylesEvent::data(frame()));
    InspectorInstrumentation::didScheduleStyleRecalculation(this);

    ++m_styleVersion;
}
