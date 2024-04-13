static irqreturn_t snd_msnd_interrupt(int irq, void *dev_id)
 {
 	struct snd_msnd *chip = dev_id;
 	void *pwDSPQData = chip->mappedbase + DSPQ_DATA_BUFF;
	u16 head, tail, size;
 
 	/* Send ack to DSP */
 	/* inb(chip->io + HP_RXL); */
 
 	/* Evaluate queued DSP messages */
	head = readw(chip->DSPQ + JQS_wHead);
	tail = readw(chip->DSPQ + JQS_wTail);
	size = readw(chip->DSPQ + JQS_wSize);
	if (head > size || tail > size)
		goto out;
	while (head != tail) {
		snd_msnd_eval_dsp_msg(chip, readw(pwDSPQData + 2 * head));
		if (++head > size)
			head = 0;
		writew(head, chip->DSPQ + JQS_wHead);
 	}
 out:
 	/* Send ack to DSP */
 	inb(chip->io + HP_RXL);
 	return IRQ_HANDLED;
}
