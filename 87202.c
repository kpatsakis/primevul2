base::TerminationStatus BrowserChildProcessHostImpl::GetTerminationStatus(
    bool known_dead, int* exit_code) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  if (!child_process_)  // If the delegate doesn't use Launch() helper.
    return base::GetTerminationStatus(data_.handle, exit_code);
  return child_process_->GetChildTerminationStatus(known_dead,
                                                   exit_code);
}
