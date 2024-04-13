vm_op_get_value (ecma_value_t object, /**< base object */
                 ecma_value_t property) /**< property name */
{
  if (ecma_is_value_object (object))
  {
    ecma_object_t *object_p = ecma_get_object_from_value (object);
    ecma_string_t *property_name_p = NULL;

    if (ecma_is_value_integer_number (property))
    {
      ecma_integer_value_t int_value = ecma_get_integer_from_value (property);

      if (int_value >= 0 && int_value <= ECMA_DIRECT_STRING_MAX_IMM)
      {
        if (ecma_get_object_type (object_p) == ECMA_OBJECT_TYPE_ARRAY)
        {
          ecma_extended_object_t *ext_object_p = (ecma_extended_object_t *) object_p;

          if (JERRY_LIKELY (ecma_op_array_is_fast_array (ext_object_p)
                            && (uint32_t) int_value < ext_object_p->u.array.length))
          {
            ecma_value_t *values_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, object_p->u1.property_list_cp);

            if (JERRY_LIKELY (!ecma_is_value_array_hole (values_p[int_value])))
            {
              return ecma_fast_copy_value (values_p[int_value]);
            }
          }
        }

        property_name_p = (ecma_string_t *) ECMA_CREATE_DIRECT_STRING (ECMA_DIRECT_STRING_UINT,
                                                                       (uintptr_t) int_value);
      }
    }
    else if (ecma_is_value_string (property))
    {
      property_name_p = ecma_get_string_from_value (property);
    }

#if JERRY_ESNEXT
    if (ecma_is_value_symbol (property))
    {
      property_name_p = ecma_get_symbol_from_value (property);
    }
#endif /* JERRY_ESNEXT */

    if (property_name_p != NULL)
    {
#if JERRY_LCACHE
      ecma_property_t *property_p = ecma_lcache_lookup (object_p, property_name_p);

      if (property_p != NULL && (*property_p & ECMA_PROPERTY_FLAG_DATA))
      {
        JERRY_ASSERT (!ECMA_PROPERTY_IS_INTERNAL (*property_p));
        return ecma_fast_copy_value (ECMA_PROPERTY_VALUE_PTR (property_p)->value);
      }
#endif /* JERRY_LCACHE */

      /* There is no need to free the name. */
      return ecma_op_object_get (object_p, property_name_p);
    }
  }

  if (JERRY_UNLIKELY (ecma_is_value_undefined (object) || ecma_is_value_null (object)))
  {
#if JERRY_ERROR_MESSAGES
    ecma_value_t error_value = ecma_raise_standard_error_with_format (JERRY_ERROR_TYPE,
                                                                      "Cannot read property '%' of %",
                                                                      property,
                                                                      object);
#else /* !JERRY_ERROR_MESSAGES */
    ecma_value_t error_value = ecma_raise_type_error (NULL);
#endif /* JERRY_ERROR_MESSAGES */
    return error_value;
  }

  ecma_string_t *property_name_p = ecma_op_to_property_key (property);

  if (property_name_p == NULL)
  {
    return ECMA_VALUE_ERROR;
  }

  ecma_value_t get_value_result = ecma_op_get_value_object_base (object, property_name_p);

  ecma_deref_ecma_string (property_name_p);
  return get_value_result;
} /* vm_op_get_value */