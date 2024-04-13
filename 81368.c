void QueryManager::RemoveQuery(GLuint client_id) {
  QueryMap::iterator it = queries_.find(client_id);
  if (it != queries_.end()) {
    Query* query = it->second.get();
    RemovePendingQuery(query);
    query->MarkAsDeleted();
    queries_.erase(it);
  }
}
