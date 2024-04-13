void AddPrintingStrings(content::WebUIDataSource* html_source) {
  LocalizedString localized_strings[] = {
    {"printingPageTitle", IDS_SETTINGS_PRINTING},
    {"printingCloudPrintLearnMoreLabel",
     IDS_SETTINGS_PRINTING_CLOUD_PRINT_LEARN_MORE_LABEL},
    {"printingNotificationsLabel", IDS_SETTINGS_PRINTING_NOTIFICATIONS_LABEL},
    {"printingManageCloudPrintDevices",
     IDS_SETTINGS_PRINTING_MANAGE_CLOUD_PRINT_DEVICES},
    {"cloudPrintersTitle", IDS_SETTINGS_PRINTING_CLOUD_PRINTERS},
#if defined(OS_CHROMEOS)
    {"cupsPrintersTitle", IDS_SETTINGS_PRINTING_CUPS_PRINTERS},
    {"cupsPrintersLearnMoreLabel",
     IDS_SETTINGS_PRINTING_CUPS_PRINTERS_LEARN_MORE_LABEL},
    {"addCupsPrinter", IDS_SETTINGS_PRINTING_CUPS_PRINTERS_ADD_PRINTER},
    {"editPrinter", IDS_SETTINGS_PRINTING_CUPS_PRINTERS_EDIT},
    {"removePrinter", IDS_SETTINGS_PRINTING_CUPS_PRINTERS_REMOVE},
    {"searchLabel", IDS_SETTINGS_PRINTING_CUPS_SEARCH_LABEL},
    {"noSearchResults", IDS_SEARCH_NO_RESULTS},
    {"printerDetailsTitle", IDS_SETTINGS_PRINTING_CUPS_PRINTER_DETAILS_TITLE},
    {"printerName", IDS_SETTINGS_PRINTING_CUPS_PRINTER_DETAILS_NAME},
    {"printerModel", IDS_SETTINGS_PRINTING_CUPS_PRINTER_DETAILS_MODEL},
    {"printerQueue", IDS_SETTINGS_PRINTING_CUPS_PRINTER_DETAILS_QUEUE},
    {"addPrintersNearbyTitle",
     IDS_SETTINGS_PRINTING_CUPS_ADD_PRINTERS_NEARBY_TITLE},
    {"addPrintersManuallyTitle",
     IDS_SETTINGS_PRINTING_CUPS_ADD_PRINTERS_MANUALLY_TITLE},
    {"selectManufacturerAndModelTitle",
     IDS_SETTINGS_PRINTING_CUPS_SELECT_MANUFACTURER_AND_MODEL_TITLE},
    {"addPrinterButtonText", IDS_SETTINGS_PRINTING_CUPS_ADD_PRINTER_BUTTON_ADD},
    {"printerDetailsAdvanced", IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADVANCED},
    {"printerDetailsA11yLabel",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADVANCED_ACCESSIBILITY_LABEL},
    {"printerAddress", IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADVANCED_ADDRESS},
    {"printerProtocol", IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADVANCED_PROTOCOL},
    {"printerURI", IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADVANCED_URI},
    {"manuallyAddPrinterButtonText",
     IDS_SETTINGS_PRINTING_CUPS_ADD_PRINTER_BUTTON_MANUAL_ADD},
    {"discoverPrintersButtonText",
     IDS_SETTINGS_PRINTING_CUPS_ADD_PRINTER_BUTTON_DISCOVER_PRINTERS},
    {"printerProtocolIpp", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_IPP},
    {"printerProtocolIpps", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_IPPS},
    {"printerProtocolHttp", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_HTTP},
    {"printerProtocolHttps", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_HTTPS},
    {"printerProtocolAppSocket",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_APP_SOCKET},
    {"printerProtocolLpd", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_LPD},
    {"printerProtocolUsb", IDS_SETTINGS_PRINTING_CUPS_PRINTER_PROTOCOL_USB},
    {"printerConfiguringMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_CONFIGURING_MESSAGE},
    {"printerManufacturer", IDS_SETTINGS_PRINTING_CUPS_PRINTER_MANUFACTURER},
    {"selectDriver", IDS_SETTINGS_PRINTING_CUPS_PRINTER_SELECT_DRIVER},
    {"selectDriverButtonText",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_BUTTON_SELECT_DRIVER},
    {"selectDriverErrorMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_INVALID_DRIVER},
    {"printerAddedSuccessfulMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_DONE_MESSAGE},
    {"noPrinterNearbyMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_NO_PRINTER_NEARBY},
    {"searchingNearbyPrinters",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_SEARCHING_NEARBY_PRINTER},
    {"printerAddedFailedMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_ERROR_MESSAGE},
    {"printerAddedFatalErrorMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_FATAL_ERROR_MESSAGE},
    {"printerAddedUnreachableMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_PRINTER_UNREACHABLE_MESSAGE},
    {"printerAddedPpdTooLargeMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_PPD_TOO_LARGE_MESSAGE},
    {"printerAddedInvalidPpdMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_INVALID_PPD_MESSAGE},
    {"printerAddedPpdNotFoundMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_PPD_NOT_FOUND},
    {"printerAddedPpdUnretrievableMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_ADDED_PRINTER_PPD_UNRETRIEVABLE},
    {"requireNetworkMessage",
     IDS_SETTINGS_PRINTING_CUPS_PRINTER_REQUIRE_INTERNET_MESSAGE},
    {"editPrinterDialogTitle",
     IDS_SETTINGS_PRINTING_CUPS_EDIT_PRINTER_DIALOG_TITLE},
    {"editPrinterButtonText", IDS_SETTINGS_PRINTING_CUPS_EDIT_PRINTER_BUTTON},
#else
    {"localPrintersTitle", IDS_SETTINGS_PRINTING_LOCAL_PRINTERS_TITLE},
#endif
  };
  AddLocalizedStringsBulk(html_source, localized_strings,
                          arraysize(localized_strings));

  html_source->AddString("devicesUrl", chrome::kChromeUIDevicesURL);
  html_source->AddString("printingCloudPrintLearnMoreUrl",
                         chrome::kCloudPrintLearnMoreURL);

#if defined(OS_CHROMEOS)
  html_source->AddString("printingCUPSPrintLearnMoreUrl",
                         GetHelpUrlWithBoard(chrome::kCupsPrintLearnMoreURL));
#endif
}