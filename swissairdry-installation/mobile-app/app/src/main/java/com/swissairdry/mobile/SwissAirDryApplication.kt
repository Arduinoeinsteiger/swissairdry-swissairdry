package com.swissairdry.mobile

import android.app.Application
import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.preferencesDataStore
import dagger.hilt.android.HiltAndroidApp
import timber.log.Timber

/**
 * SwissAirDry Hauptanwendungsklasse
 * 
 * Diese Klasse initialisiert die Android-Anwendung und richtet die 
 * erforderlichen Abhängigkeiten und Komponenten ein.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@HiltAndroidApp
class SwissAirDryApplication : Application() {

    // DataStore für App-Präferenzen
    val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")

    override fun onCreate() {
        super.onCreate()
        
        // Logging nur im Debug-Modus initialisieren
        if (BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        }
        
        // Globaler Error-Handler
        Thread.setDefaultUncaughtExceptionHandler { _, throwable ->
            Timber.e(throwable, "Unbehandelter Fehler in der Anwendung")
        }
    }
}