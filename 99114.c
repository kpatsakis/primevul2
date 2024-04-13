ExprResolveMaskLookup(struct xkb_context *ctx, const ExprDef *expr,
                      unsigned int *val_rtrn, IdentLookupFunc lookup,
                      const void *lookupPriv)
{
    bool ok = false;
    unsigned int l = 0, r = 0;
    int v;
    ExprDef *left, *right;
    const char *bogus = NULL;

    switch (expr->expr.op) {
    case EXPR_VALUE:
        if (expr->expr.value_type != EXPR_TYPE_INT) {
            log_err(ctx,
                    "Found constant of type %s where a mask was expected\n",
                    expr_value_type_to_string(expr->expr.value_type));
            return false;
        }
        *val_rtrn = (unsigned int) expr->integer.ival;
        return true;

    case EXPR_IDENT:
        ok = lookup(ctx, lookupPriv, expr->ident.ident, EXPR_TYPE_INT,
                    val_rtrn);
        if (!ok)
            log_err(ctx, "Identifier \"%s\" of type int is unknown\n",
                    xkb_atom_text(ctx, expr->ident.ident));
        return ok;

    case EXPR_FIELD_REF:
        log_err(ctx, "Default \"%s.%s\" of type int is unknown\n",
                xkb_atom_text(ctx, expr->field_ref.element),
                xkb_atom_text(ctx, expr->field_ref.field));
        return false;

    case EXPR_ARRAY_REF:
        bogus = "array reference";
	/* fallthrough */
    case EXPR_ACTION_DECL:
        if (bogus == NULL)
            bogus = "function use";
        log_err(ctx,
                "Unexpected %s in mask expression; Expression Ignored\n",
                bogus);
        return false;

    case EXPR_ADD:
    case EXPR_SUBTRACT:
    case EXPR_MULTIPLY:
    case EXPR_DIVIDE:
        left = expr->binary.left;
        right = expr->binary.right;
        if (!ExprResolveMaskLookup(ctx, left, &l, lookup, lookupPriv) ||
            !ExprResolveMaskLookup(ctx, right, &r, lookup, lookupPriv))
            return false;

        switch (expr->expr.op) {
        case EXPR_ADD:
            *val_rtrn = l | r;
            break;
        case EXPR_SUBTRACT:
            *val_rtrn = l & (~r);
            break;
        case EXPR_MULTIPLY:
        case EXPR_DIVIDE:
            log_err(ctx, "Cannot %s masks; Illegal operation ignored\n",
                    (expr->expr.op == EXPR_DIVIDE ? "divide" : "multiply"));
            return false;
        default:
            break;
        }

        return true;

    case EXPR_ASSIGN:
        log_wsgo(ctx, "Assignment operator not implemented yet\n");
        break;

    case EXPR_INVERT:
        left = expr->unary.child;
        if (!ExprResolveIntegerLookup(ctx, left, &v, lookup, lookupPriv))
            return false;

        *val_rtrn = ~v;
        return true;

    case EXPR_UNARY_PLUS:
    case EXPR_NEGATE:
    case EXPR_NOT:
        left = expr->unary.child;
        if (!ExprResolveIntegerLookup(ctx, left, &v, lookup, lookupPriv))
            log_err(ctx, "The %s operator cannot be used with a mask\n",
                    (expr->expr.op == EXPR_NEGATE ? "-" : "!"));
        return false;

    default:
        log_wsgo(ctx, "Unknown operator %d in ResolveMask\n",
                 expr->expr.op);
        break;
    }

    return false;
}
