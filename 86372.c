void DownloadItemImpl::Resume() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  DVLOG(20) << __func__ << "() download = " << DebugString(true);
  switch (state_) {
    case CANCELLED_INTERNAL:  // Nothing to resume.
    case COMPLETE_INTERNAL:
    case COMPLETING_INTERNAL:
    case INITIAL_INTERNAL:
    case INTERRUPTED_TARGET_PENDING_INTERNAL:
    case RESUMING_INTERNAL:   // Resumption in progress.
      return;

    case TARGET_PENDING_INTERNAL:
    case IN_PROGRESS_INTERNAL:
      if (!IsPaused())
        return;
      if (job_)
        job_->Resume(true);
      UpdateObservers();
      return;

    case INTERRUPTED_INTERNAL:
      auto_resume_count_ = 0;  // User input resets the counter.
      ResumeInterruptedDownload(ResumptionRequestSource::USER);
      UpdateObservers();
      return;

    case MAX_DOWNLOAD_INTERNAL_STATE:
    case TARGET_RESOLVED_INTERNAL:
      NOTREACHED();
  }
}
