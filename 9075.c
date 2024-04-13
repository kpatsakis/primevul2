vm_get_implicit_this_value (ecma_value_t *this_value_p) /**< [in,out] this value */
{
  if (ecma_is_value_object (*this_value_p))
  {
    ecma_object_t *this_obj_p = ecma_get_object_from_value (*this_value_p);

    if (ecma_is_lexical_environment (this_obj_p))
    {
      ecma_value_t completion_value = ecma_op_implicit_this_value (this_obj_p);

      JERRY_ASSERT (!ECMA_IS_VALUE_ERROR (completion_value));

      *this_value_p = completion_value;
      return true;
    }
  }
  return false;
} /* vm_get_implicit_this_value */