OperationID FileSystemOperationRunner::Truncate(const FileSystemURL& url,
                                                int64_t length,
                                                StatusCallback callback) {
  base::File::Error error = base::File::FILE_OK;
  std::unique_ptr<FileSystemOperation> operation = base::WrapUnique(
      file_system_context_->CreateFileSystemOperation(url, &error));
  FileSystemOperation* operation_raw = operation.get();
  OperationID id = BeginOperation(std::move(operation));
  base::AutoReset<bool> beginning(&is_beginning_operation_, true);
  if (!operation_raw) {
    DidFinish(id, std::move(callback), error);
    return id;
  }
  PrepareForWrite(id, url);
  operation_raw->Truncate(url, length,
                          base::BindOnce(&FileSystemOperationRunner::DidFinish,
                                         weak_ptr_, id, std::move(callback)));
  return id;
}
