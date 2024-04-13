vm_op_set_value (ecma_value_t base, /**< base object */
                 ecma_value_t property, /**< property name */
                 ecma_value_t value, /**< ecma value */
                 bool is_strict) /**< strict mode */
{
  ecma_value_t result = ECMA_VALUE_EMPTY;
  ecma_object_t *object_p;
  ecma_string_t *property_p;

  if (JERRY_UNLIKELY (!ecma_is_value_object (base)))
  {
    if (JERRY_UNLIKELY (ecma_is_value_null (base) || ecma_is_value_undefined (base)))
    {
#if JERRY_ERROR_MESSAGES
      result = ecma_raise_standard_error_with_format (JERRY_ERROR_TYPE,
                                                      "Cannot set property '%' of %",
                                                      property,
                                                      base);
#else /* !JERRY_ERROR_MESSAGES */
      result = ecma_raise_type_error (NULL);
#endif /* JERRY_ERROR_MESSAGES */
      ecma_free_value (property);
      return result;
    }

    if (JERRY_UNLIKELY (!ecma_is_value_prop_name (property)))
    {
      property_p = ecma_op_to_string (property);
      ecma_fast_free_value (property);

      if (JERRY_UNLIKELY (property_p == NULL))
      {
        ecma_free_value (base);
        return ECMA_VALUE_ERROR;
      }
    }
    else
    {
      property_p = ecma_get_prop_name_from_value (property);
    }

    ecma_value_t object = ecma_op_to_object (base);
    JERRY_ASSERT (!ECMA_IS_VALUE_ERROR (object));

    object_p = ecma_get_object_from_value (object);
    ecma_op_ordinary_object_prevent_extensions (object_p);

    result = ecma_op_object_put_with_receiver (object_p,
                                               property_p,
                                               value,
                                               base,
                                               is_strict);

    ecma_free_value (base);
  }
  else
  {
    object_p = ecma_get_object_from_value (base);

    if (JERRY_UNLIKELY (!ecma_is_value_prop_name (property)))
    {
      property_p = ecma_op_to_string (property);
      ecma_fast_free_value (property);

      if (JERRY_UNLIKELY (property_p == NULL))
      {
        ecma_deref_object (object_p);
        return ECMA_VALUE_ERROR;
      }
    }
    else
    {
      property_p = ecma_get_prop_name_from_value (property);
    }

    if (!ecma_is_lexical_environment (object_p))
    {
      result = ecma_op_object_put_with_receiver (object_p,
                                                 property_p,
                                                 value,
                                                 base,
                                                 is_strict);
    }
    else
    {
      result = ecma_op_set_mutable_binding (object_p,
                                            property_p,
                                            value,
                                            is_strict);
    }
  }

  ecma_deref_object (object_p);
  ecma_deref_ecma_string (property_p);
  return result;
} /* vm_op_set_value */