static void process_delete_command(conn *c, token_t *tokens, const size_t ntokens) {
    char *key;
    size_t nkey;
    item *it;

    assert(c != NULL);

    if (ntokens == 4) {
        if (!set_noreply_maybe(c, tokens, ntokens)) {
            out_string(c, "CLIENT_ERROR bad command line format.  "
                       "Usage: delete <key> [noreply]");
            return;
        }
    }

    key = tokens[KEY_TOKEN].value;
    nkey = tokens[KEY_TOKEN].length;

    if(nkey > KEY_MAX_LENGTH) {
        out_string(c, "CLIENT_ERROR bad command line format");
        return;
    }

    if (settings.detail_enabled) {
        stats_prefix_record_delete(key, nkey);
    }

    it = item_get(key, nkey);
    if (it) {
        MEMCACHED_COMMAND_DELETE(c->sfd, ITEM_key(it), it->nkey);

        pthread_mutex_lock(&c->thread->stats.mutex);
        c->thread->stats.slab_stats[it->slabs_clsid].delete_hits++;
        pthread_mutex_unlock(&c->thread->stats.mutex);

        item_unlink(it);
        item_remove(it);      /* release our reference */
        out_string(c, "DELETED");
    } else {
        pthread_mutex_lock(&c->thread->stats.mutex);
        c->thread->stats.delete_misses++;
        pthread_mutex_unlock(&c->thread->stats.mutex);

        out_string(c, "NOT_FOUND");
    }
}
