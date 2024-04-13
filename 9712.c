njs_vm_compile(njs_vm_t *vm, u_char **start, u_char *end)
{
    njs_int_t           ret;
    njs_str_t           ast;
    njs_chb_t           chain;
    njs_value_t         **global, **new;
    njs_lexer_t         lexer;
    njs_parser_t        parser;
    njs_vm_code_t       *code;
    njs_generator_t     generator;
    njs_parser_scope_t  *scope;

    njs_memzero(&parser, sizeof(njs_parser_t));

    parser.scope = vm->global_scope;

    if (parser.scope != NULL && vm->modules != NULL) {
        njs_module_reset(vm);
    }

    ret = njs_lexer_init(vm, &lexer, &vm->options.file, *start, end, 0);
    if (njs_slow_path(ret != NJS_OK)) {
        return NJS_ERROR;
    }

    parser.lexer = &lexer;

    ret = njs_parser(vm, &parser);
    if (njs_slow_path(ret != NJS_OK)) {
        return NJS_ERROR;
    }

    if (njs_slow_path(vm->options.ast)) {
        njs_chb_init(&chain, vm->mem_pool);
        ret = njs_parser_serialize_ast(parser.node, &chain);
        if (njs_slow_path(ret == NJS_ERROR)) {
            return ret;
        }

        if (njs_slow_path(njs_chb_join(&chain, &ast) != NJS_OK)) {
            return NJS_ERROR;
        }

        njs_print(ast.start, ast.length);

        njs_chb_destroy(&chain);
        njs_mp_free(vm->mem_pool, ast.start);
    }

    *start = lexer.start;
    scope = parser.scope;

    ret = njs_generator_init(&generator, 0, 0);
    if (njs_slow_path(ret != NJS_OK)) {
        njs_internal_error(vm, "njs_generator_init() failed");
        return NJS_ERROR;
    }

    code = njs_generate_scope(vm, &generator, scope, &njs_entry_main);
    if (njs_slow_path(code == NULL)) {
        if (!njs_is_error(&vm->retval)) {
            njs_internal_error(vm, "njs_generate_scope() failed");
        }

        return NJS_ERROR;
    }

    vm->global_scope = scope;

    if (scope->items > vm->global_items) {
        global = vm->levels[NJS_LEVEL_GLOBAL];

        new = njs_scope_make(vm, scope->items);
        if (njs_slow_path(new == NULL)) {
            return ret;
        }

        vm->levels[NJS_LEVEL_GLOBAL] = new;

        if (global != NULL) {
            while (vm->global_items != 0) {
                vm->global_items--;

                *new++ = *global++;
            }

            njs_mp_free(vm->mem_pool, global);
        }
    }

    /* globalThis and this */
    njs_scope_value_set(vm, njs_scope_global_this_index(), &vm->global_value);

    vm->start = generator.code_start;
    vm->variables_hash = &scope->variables;
    vm->global_items = scope->items;

    vm->levels[NJS_LEVEL_TEMP] = NULL;

    if (scope->temp != 0) {
        new = njs_scope_make(vm, scope->temp);
        if (njs_slow_path(new == NULL)) {
            return ret;
        }

        vm->levels[NJS_LEVEL_TEMP] = new;
    }

    if (vm->options.disassemble) {
        njs_disassembler(vm);
    }

    return NJS_OK;
}