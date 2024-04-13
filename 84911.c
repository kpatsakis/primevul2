void PDFiumEngine::OnCallback(int id) {
  if (!timers_.count(id))
    return;

  timers_[id].second(id);
  if (timers_.count(id))  // The callback might delete the timer.
    client_->ScheduleCallback(id, timers_[id].first);
}
