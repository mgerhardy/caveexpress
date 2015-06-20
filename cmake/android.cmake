option(HD_VERSION "Build the HD versions of the games" OFF)
set(PERMISSIONS)
set(META_DATA)
set(ANDROID_REFERENCED_LIBS android.library.reference.1=google-play-services_lib)
set(UNITTESTS OFF)
set(TOOLS OFF)

list(APPEND PERMISSIONS "<uses-permission android:name=\"com.android.vending.BILLING\" />")

if (NOT HD_VERSION)
list(APPEND META_DATA "<meta-data android:value=\"true\" android:name=\"ADMOB_ALLOW_LOCATION_FOR_ADS\" />")
endif()
list(APPEND META_DATA "<meta-data android:name=\"com.google.android.gms.games.APP_ID\" android:value=\"@string/app_id\" />")
list(APPEND META_DATA "<meta-data android:name=\"com.google.android.gms.appstate.APP_ID\" android:value=\"@string/app_id\" />")
list(APPEND META_DATA "<meta-data android:name=\"com.google.android.gms.version\" android:value=\"@integer/google_play_services_version\" />")

