void snd_msndmidi_input_read(void *mpuv)
{
 	unsigned long flags;
 	struct snd_msndmidi *mpu = mpuv;
 	void *pwMIDQData = mpu->dev->mappedbase + MIDQ_DATA_BUFF;
	u16 head, tail, size;
 
 	spin_lock_irqsave(&mpu->input_lock, flags);
	head = readw(mpu->dev->MIDQ + JQS_wHead);
	tail = readw(mpu->dev->MIDQ + JQS_wTail);
	size = readw(mpu->dev->MIDQ + JQS_wSize);
	if (head > size || tail > size)
		goto out;
	while (head != tail) {
		unsigned char val = readw(pwMIDQData + 2 * head);

		if (test_bit(MSNDMIDI_MODE_BIT_INPUT_TRIGGER, &mpu->mode))
			snd_rawmidi_receive(mpu->substream_input, &val, 1);
		if (++head > size)
			head = 0;
		writew(head, mpu->dev->MIDQ + JQS_wHead);
 	}
 out:
 	spin_unlock_irqrestore(&mpu->input_lock, flags);
 }