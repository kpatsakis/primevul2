AcpiDsDoImplicitReturn (
    ACPI_OPERAND_OBJECT     *ReturnDesc,
    ACPI_WALK_STATE         *WalkState,
    BOOLEAN                 AddReference)
{
    ACPI_FUNCTION_NAME (DsDoImplicitReturn);


    /*
     * Slack must be enabled for this feature, and we must
     * have a valid return object
     */
    if ((!AcpiGbl_EnableInterpreterSlack) ||
        (!ReturnDesc))
    {
        return (FALSE);
    }

    ACPI_DEBUG_PRINT ((ACPI_DB_DISPATCH,
        "Result %p will be implicitly returned; Prev=%p\n",
        ReturnDesc,
        WalkState->ImplicitReturnObj));

    /*
     * Delete any "stale" implicit return value first. However, in
     * complex statements, the implicit return value can be
     * bubbled up several levels, so we don't clear the value if it
     * is the same as the ReturnDesc.
     */
    if (WalkState->ImplicitReturnObj)
    {
        if (WalkState->ImplicitReturnObj == ReturnDesc)
        {
            return (TRUE);
        }
        AcpiDsClearImplicitReturn (WalkState);
    }

    /* Save the implicit return value, add a reference if requested */

    WalkState->ImplicitReturnObj = ReturnDesc;
    if (AddReference)
    {
        AcpiUtAddReference (ReturnDesc);
    }

    return (TRUE);
}
