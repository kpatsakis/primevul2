void PasswordAutofillAgent::OnDynamicFormsSeen() {
  SendPasswordForms(false /* only_visible */);
}
