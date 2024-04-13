static int amd_gpio_remove(struct platform_device *pdev)
{
	struct amd_gpio *gpio_dev;

 	gpio_dev = platform_get_drvdata(pdev);
 
 	gpiochip_remove(&gpio_dev->gc);
 
 	return 0;
 }
