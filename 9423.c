  ~SliceHelper() {
    CHECK(copy_out_.empty());
    for (const auto& entry : pool_) {
      CHECK(!entry.second.second);  // nothing is in use
    }
  }