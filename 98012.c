AcpiDsClearOperands (
    ACPI_WALK_STATE         *WalkState)
{
    UINT32                  i;


    ACPI_FUNCTION_TRACE_PTR (DsClearOperands, WalkState);


    /* Remove a reference on each operand on the stack */

    for (i = 0; i < WalkState->NumOperands; i++)
    {
        /*
         * Remove a reference to all operands, including both
         * "Arguments" and "Targets".
         */
        AcpiUtRemoveReference (WalkState->Operands[i]);
        WalkState->Operands[i] = NULL;
    }

    WalkState->NumOperands = 0;
    return_VOID;
}
