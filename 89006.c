void RenderFrameSubmissionObserver::Quit() {
  if (run_loop_)
    run_loop_->Quit();
}
