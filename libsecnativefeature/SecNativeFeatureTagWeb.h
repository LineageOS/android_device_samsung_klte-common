#ifndef SEC_NATIVE_FEATURE_TAG_WEB_H
#define SEC_NATIVE_FEATURE_TAG_WEB_H


// Note
// The string must be same as the string in CSCFeatureTagWeb.java
// Because one feature coulde be implemented both in java layer and in native layer

// The default values for each types
// This means the specified TAG is not defined
#define CSCFeatureTagWeb_Str_NoTag ""
#define CSCFeatureTagWeb_Bool_NoTag false
#define CSCFeatureTagWeb_Int_NoTag 0

// To define custom UserAgent string
// If this value is not defined, the default Android UserAgent will be sent to server
#define CSCFeatureTagWeb_SetUserAgent	"CscFeature_Web_SetUserAgent"


// To define UAProfile string
// If this value is not defined, UAProfile will not be sent to server
#define CSCFeatureTagWeb_SetUAProfile	"CscFeature_Web_SetUAProfile"

// Change homepage_base regardless language and APN
#define CscFeatureTagWeb_SetHomepageURL "CscFeature_Web_SetHomepageURL"
// Disable RSS button on the url bar
#define CscFeatureTagWeb_DisableRSS "CscFeature_Web_DisableRSS"
// Remove voicesearch button on the url bar
#define CscFeatureTagWeb_DisableVoiceSearch "CscFeature_Web_DisableVoiceSearch"
// Disable showing activity chooser on redirecting in case when 3rd party browser also installed
#define CscFeatureTagWeb_DisableRedirectionChooser "CscFeature_Web_DisableRedirectionChooser"

// Enable UAProfile in Header
#define CscFeatureTagWeb_Bool_EnableUAProfile "CscFeature_Web_Bool_EnableUAProfile"

// To Enable the GateConfig Logs(USA STA requirement for stability test)
#define CscFeatureTagWeb_Bool_EnableGateConfig "CscFeature_Web_EnableLogStabililtyTest"

//add for Handling Operator UA
#define CscFeature_Web_OverrideUserAgent "CscFeature_Web_OverrideUserAgent"

// add menu clear today history
#define CscFeatureTagWeb_EnableDeletingTodayHistory "CscFeature_Web_EnableDeletingTodayHistory"

 // To enable bookmark overwrite
#define CscFeatureTagWeb_EnableOverwritingBookmark "CscFeature_Web_EnableOverwritingBookmark"

// To enable downloaded folder notification
#define CscFeatureTagWeb_EnableDownloadedFolderInNotificationBar "CscFeature_Web_EnableDownloadedFolderInNotificationBar"

  //Enable DeviceID at Header(USA ATT Requirement)
#define CscFeatureTag_Web_Bool_DeviceID "CscFeature_Web_UseDeviceIdInHeader"

// support uploading contacts in VCard format (China-Telecom requirement)
#define CscFeatureTagWeb_SupportVcfUpload "CscFeature_Web_SupportVcfUpload"
//Parse XHTML document as HTML (ignore parse error. China-Telecom requirement)
#define CscFeatureTagWeb_ParseXHtmlToHtml "CscFeature_Web_ParseXHtmlToHtml"

   //Block SD & CD download ( NAGSM common requirement)
#define  CscFeatureTag_Web_Bool_BlockSDCDDownload "CscFeature_Web_BlockSDCDDownload"

// display download progress on notification bar (China-Telecom requirement)
#define CscFeatureTagWeb_ShowDownloadProgressOnNotification "CscFeature_Web_ShowDownloadProgressOnNotification"

// show browser version in browser settings (China-Telecom requirement)
#define CscFeatureTagWeb_ShowVersionInSetting "CscFeature_Web_ShowVersionInSetting"

// support "exit browser" option (China-Telecom requirement)
#define CscFeatureTagWeb_AddOptionToTerminate "CscFeature_Web_AddOptionToTerminate"
// Add "Delete-all" function at Download list (China-Telecom requirement)
#define CscFeatureTagWeb_EnableDeleteAllOnDownloadList "CscFeature_Web_EnableDeleteAllOnDownloadList"
// Support save-as function and default folder by mime-type (China-Telecom requirement)
#define CscFeatureTagWeb_SupportDownloadSaveAs "CscFeature_Web_SupportDownloadSaveAs"
// support delete-all option in bookmarks page (China-Telecom requirement)
#define CscFeatureTagWeb_EnableDeleteAllBookmarks "CscFeature_Web_EnableDeleteAllBookmarks"
// Add "FullHandwriting IME issue "(China-Common requirement)
#define CscFeatureTagWeb_EnableOptionEditTextDuringFullHwr "CscFeature_Web_EnableOptionEditTextDuringFullHwr"
// support offline-startup page includes bookmarks, history, search dialog (China-Telecom requirement)
#define CscFeatureTagWeb_SupportOfflineStartupPage "CscFeature_Web_SupportOfflineStartupPage"

// support not to set factory-reset-homepage to PREF_HOMEPAGE
#define CscFeatureTagWeb_Bool_DisableSetFactoryResetHomeToPrefHome "CscFeature_Web_Bool_DisableSetFactoryResetHomeToPrefHome"

// Show roaming dialog (ATT Requirement)
#define CscFeatureTagWeb_Bool_ShowRoamingDialog "CscFeature_Web_EnableRoamingDialog"

// set download folder by mime-type for China-Telecom requirement
#define CscFeatureTagWeb_SetDownloadFolderNameByMimeType "CscFeature_Web_SetDownloadFolderNameByMimeType"

// Disable showing activity chooser for defined string
#define CscFeature_Web_DisableChooser4  "CscFeature_Web_DisableChooser4"

// Enable download hebrew filename
#define CscFeature_Web_SupportHebrewFileName "CscFeature_Web_SupportHebrewFileName"

// remove google in search engine list
#define CscFeatureTagWeb_DisableGoogleInBrowserSearchEngine "CscFeature_Web_DisableGoogleInBrowserSearchEngine"

// set off overview mode as default (CTC Requirement)
#define CscFeatureTagWeb_SetOffOverviewModeAsDefault "CscFeature_Web_SetOffOverviewModeAsDefault"

// Disable setting homepage as it is set in APN when SIM changed
#define CscFeature_Web_EnableAutoSimHomeUrlInProfile  "CscFeature_Web_EnableAutoSimHomeUrlInProfile"

// Add download file name decode feature for China region
#define CscFeatureTagWeb_SupportDownloadedFileNameInChineseChar "CscFeature_Web_SupportDownloadedFileNameInChineseChar"

// support multiAPN (CMCC Requirement)
#define CscFeatureTagWeb_EnableMultipleApn4 "CscFeature_Web_EnableMultipleApn4"

// Enable Ask to exit on back (CMCC Requirement)
#define CscFeatureTagWeb_EnablePromptToExit "CscFeature_Web_EnablePromptToExit"


// Enable EMOJI for JPN
#define CscFeatureTagWeb_EnableEmoji "CscFeature_Web_Bool_EnableEmoji"

// Show Wifi AP List when WIFI switch is on (CMCC Requirement)
#define CscFeatureTagWeb_ShowWifiAPList "CscFeature_Web_ShowWifiAPList"

 // Enable Operator's toolbar (ATT Requirement)
#define CscFeatureTagWeb_EnableOperatorToolbar "CscFeature_Web_EnableOperatorToolbar"

// Set TCP Connection timout (China Requirement)
#define CscFeatureTagWeb_SetTcpConnTimeoutAs "CscFeature_Web_SetTcpConnTimeoutAs"

// Add WML mime type to Accept Header (CMCC Requirement)
#define CscFeatureTagWeb_AddWmlToHttpAcceptHeader4 "CscFeature_Web_AddWmlToHttpAcceptHeader4"

// change block zoom method to position based touch block zoom (Australia Requirement)
#define CscFeature_Web_BlockZoomBaseOnTouchPosition "CscFeature_Web_BlockZoomBaseOnTouchPosition"

// Remove Partial View During Horizontal Scroll in Nav Screen (Australia Requirement)
#define CscFeature_Web_RemovePartialViewDuringHorizontalScroll "CscFeature_Web_RemovePartialViewDuringHorizontalScroll"

// Show Popup for MaxLength  reached during Url input (CMCC Requirement)
#define CscFeatureTagWeb_EnablePopup4MaxLengthReachedDuringUrlInput "CscFeature_Web_EnablePopup4MaxLengthReachedDuringUrlInput"

// Support Tel Number in the page to go to dialing app (KOR Requirement)
#define CscFeature_Web_RecognizeTelNumber "CscFeature_Web_RecognizeTelNumber"

// Large file transfer in Internet download (VZW Requirement)
#define CscFeature_Web_EnableWifiOption4LargeFileDownload "CscFeature_Web_EnableWifiOption4LargeFileDownload"

// Add additional accept charset  to Accept Header (CTC Requirement)
#define CscFeature_Web_AddCharSetToHttpHeader "CscFeature_Web_AddCharSetToHttpHeader"

// max connection per host for performance
#define CscFeature_Web_MaxConnectionPerHost "CscFeature_Web_MaxConnectionPerHost"
#endif // SEC_NATIVE_FEATURE_TAG_WEB_H
