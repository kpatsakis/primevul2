bool PrivateScriptRunner::runDOMAttributeSetter(
    ScriptState* scriptState,
    ScriptState* scriptStateInUserScript,
    const char* className,
    const char* attributeName,
    v8::Local<v8::Value> holder,
    v8::Local<v8::Value> v8Value) {
  v8::Isolate* isolate = scriptState->isolate();
  v8::Local<v8::Object> classObject =
      classObjectOfPrivateScript(scriptState, className);
  v8::Local<v8::Value> descriptor;
  if (!classObject
           ->GetOwnPropertyDescriptor(scriptState->context(),
                                      v8String(isolate, attributeName))
           .ToLocal(&descriptor) ||
      !descriptor->IsObject()) {
    LOG(FATAL)
        << "Private script error: Target DOM attribute setter was not found. "
           "(Class name = "
        << className << ", Attribute name = " << attributeName << ")";
  }
  v8::Local<v8::Value> setter;
  if (!v8::Local<v8::Object>::Cast(descriptor)
           ->Get(scriptState->context(), v8String(isolate, "set"))
           .ToLocal(&setter) ||
      !setter->IsFunction()) {
    LOG(FATAL) << "Private script error: Target DOM attribute setter was not "
                  "found. (Class name = "
               << className << ", Attribute name = " << attributeName << ")";
  }
  initializeHolderIfNeeded(scriptState, classObject, holder);
  v8::Local<v8::Value> argv[] = {v8Value};
  v8::TryCatch block(isolate);
  v8::Local<v8::Value> result;
  if (!V8ScriptRunner::callInternalFunction(
           v8::Local<v8::Function>::Cast(setter), holder,
           WTF_ARRAY_LENGTH(argv), argv, isolate)
           .ToLocal(&result)) {
    rethrowExceptionInPrivateScript(isolate, block, scriptStateInUserScript,
                                    ExceptionState::SetterContext,
                                    attributeName, className);
    block.ReThrow();
    return false;
  }
  return true;
}
