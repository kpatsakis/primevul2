void Browser::TabStripEmpty() {
  base::ThreadTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::Bind(&Browser::CloseFrame, weak_factory_.GetWeakPtr()));

  instant_controller_.reset();
}
