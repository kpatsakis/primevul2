static int spl_object_storage_compare_info(zval *e1, zval *e2) /* {{{ */
{
	spl_SplObjectStorageElement *s1 = (spl_SplObjectStorageElement*)Z_PTR_P(e1);
	spl_SplObjectStorageElement *s2 = (spl_SplObjectStorageElement*)Z_PTR_P(e2);
	zval result;

	if (compare_function(&result, &s1->inf, &s2->inf) == FAILURE) {
		return 1;
	}

	return Z_LVAL(result) > 0 ? 1 : (Z_LVAL(result) < 0 ? -1 : 0);
}
/* }}} */
