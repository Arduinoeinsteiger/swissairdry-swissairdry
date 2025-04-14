package com.swissairdry.mobile

import android.app.Application
import dagger.hilt.android.HiltAndroidApp
import timber.log.Timber

/**
 * SwissAirDryApplication - Hauptanwendungsklasse
 *
 * Diese Klasse wird beim Starten der App initialisiert und dient als
 * Einstiegspunkt für die Hilt-Dependency-Injection.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@HiltAndroidApp
class SwissAirDryApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        
        // Logger initialisieren
        if (BuildConfig.DEBUG) {
            Timber.plant(Timber.DebugTree())
        }
        
        Timber.i("SwissAirDry Application gestartet")
    }
}