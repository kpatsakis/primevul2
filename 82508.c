static void V8LazyEventListenerToString(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8SetReturnValue(info, V8HiddenValue::getHiddenValue(info.GetIsolate(), info.Holder(), V8HiddenValue::toStringString(info.GetIsolate())));
}
