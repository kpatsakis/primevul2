void snd_seq_device_load_drivers(void)
{
	queue_autoload_drivers();
 	flush_work(&autoload_work);
 }
