void V4L2JpegEncodeAccelerator::EncodedInstanceDmaBuf::Dequeue() {
  DCHECK(parent_->encoder_task_runner_->BelongsToCurrentThread());
  struct v4l2_buffer dqbuf;
  struct v4l2_plane planes[kMaxNV12Plane];
  while (InputBufferQueuedCount() > 0) {
    DCHECK(input_streamon_);
    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(planes, 0, sizeof(planes));
    dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    dqbuf.memory = V4L2_MEMORY_DMABUF;
    dqbuf.length = base::size(planes);
    dqbuf.m.planes = planes;
    if (device_->Ioctl(VIDIOC_DQBUF, &dqbuf) != 0) {
      if (errno == EAGAIN) {
        break;
      }
      VPLOGF(1) << "ioctl() failed: input buffer VIDIOC_DQBUF failed.";
      NotifyError(kInvalidBitstreamBufferId, PLATFORM_FAILURE);
      return;
    }
    free_input_buffers_.push_back(dqbuf.index);

    if (dqbuf.flags & V4L2_BUF_FLAG_ERROR) {
      VLOGF(1) << "Error in dequeued input buffer.";
      NotifyError(kInvalidBitstreamBufferId, PARSE_IMAGE_FAILED);
      running_job_queue_.pop();
    }
  }

  while (!running_job_queue_.empty() && OutputBufferQueuedCount() > 0) {
    DCHECK(output_streamon_);
    memset(&dqbuf, 0, sizeof(dqbuf));
    memset(planes, 0, sizeof(planes));
    dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    dqbuf.memory = V4L2_MEMORY_DMABUF;
    dqbuf.length = base::size(planes);
    dqbuf.m.planes = planes;
    if (device_->Ioctl(VIDIOC_DQBUF, &dqbuf) != 0) {
      if (errno == EAGAIN) {
        break;
      }
      VPLOGF(1) << "ioctl() failed: output buffer VIDIOC_DQBUF failed.";
      NotifyError(kInvalidBitstreamBufferId, PLATFORM_FAILURE);
      return;
    }
    free_output_buffers_.push_back(dqbuf.index);

    std::unique_ptr<JobRecord> job_record =
        std::move(running_job_queue_.front());
    running_job_queue_.pop();

    if (dqbuf.flags & V4L2_BUF_FLAG_ERROR) {
      VLOGF(1) << "Error in dequeued output buffer.";
      NotifyError(kInvalidBitstreamBufferId, PARSE_IMAGE_FAILED);
      return;
    }

    size_t jpeg_size =
        FinalizeJpegImage(job_record->output_frame, planes[0].bytesused,
                          std::move(job_record->exif_shm));

    if (!jpeg_size) {
      NotifyError(job_record->task_id, PLATFORM_FAILURE);
      return;
    }
    DVLOGF(4) << "Encoding finished, returning bitstream buffer, id="
              << job_record->task_id;

    parent_->VideoFrameReady(job_record->task_id, jpeg_size);
  }
}
