package com.swissairdry.mobile.api

import android.content.Context
import android.content.SharedPreferences
import android.net.ConnectivityManager
import android.net.NetworkCapabilities
import android.util.Log
import com.swissairdry.mobile.BuildConfig
import okhttp3.HttpUrl.Companion.toHttpUrl
import okhttp3.Interceptor
import okhttp3.Request
import okhttp3.Response
import java.io.IOException
import java.net.SocketTimeoutException
import java.net.UnknownHostException
import java.util.concurrent.TimeUnit
import javax.inject.Inject
import javax.inject.Singleton

/**
 * Interceptor für OkHttp, der automatisch zwischen primärem und Backup-API-Server umschaltet,
 * wenn der primäre Server nicht erreichbar ist.
 */
@Singleton
class NetworkFailoverInterceptor @Inject constructor(
    private val context: Context,
    private val preferences: SharedPreferences
) : Interceptor {

    companion object {
        private const val TAG = "NetworkFailover"
        private const val PREF_KEY_ACTIVE_SERVER = "active_server_type"
        private const val PREF_KEY_LAST_PRIMARY_CHECK = "last_primary_server_check"
        private const val PRIMARY_CHECK_INTERVAL = 5 * 60 * 1000 // 5 Minuten in Millisekunden

        // Serverdefinitionen
        private const val SERVER_TYPE_PRIMARY = "primary"
        private const val SERVER_TYPE_BACKUP = "backup"

        // Primärer Server
        private val PRIMARY_API_HOST = BuildConfig.PRIMARY_API_HOST
        private val PRIMARY_API_SCHEME = BuildConfig.PRIMARY_API_SCHEME
        private val PRIMARY_API_PORT = BuildConfig.PRIMARY_API_PORT

        // Backup-Server
        private val BACKUP_API_HOST = BuildConfig.BACKUP_API_HOST
        private val BACKUP_API_SCHEME = BuildConfig.BACKUP_API_SCHEME
        private val BACKUP_API_PORT = BuildConfig.BACKUP_API_PORT
    }

    override fun intercept(chain: Interceptor.Chain): Response {
        val originalRequest = chain.request()
        
        // Prüfe, ob Netzwerk verfügbar ist
        if (!isNetworkAvailable()) {
            throw IOException("Keine Netzwerkverbindung verfügbar")
        }

        // Aktuelle Server-Konfiguration ermitteln
        val activeServerType = getActiveServerType()
        
        // Request mit aktuellem API-Server erstellen
        val modifiedRequest = when (activeServerType) {
            SERVER_TYPE_PRIMARY -> createRequestForServer(
                originalRequest, 
                PRIMARY_API_SCHEME, 
                PRIMARY_API_HOST, 
                PRIMARY_API_PORT
            )
            SERVER_TYPE_BACKUP -> createRequestForServer(
                originalRequest, 
                BACKUP_API_SCHEME, 
                BACKUP_API_HOST, 
                BACKUP_API_PORT
            )
            else -> originalRequest // Sollte nie vorkommen
        }

        return try {
            // Versuche den Request mit dem aktiven Server
            chain.proceed(modifiedRequest)
        } catch (e: Exception) {
            when (e) {
                is SocketTimeoutException, is UnknownHostException, is IOException -> {
                    // Bei Verbindungsproblemen, versuche den alternativen Server
                    Log.w(TAG, "Verbindungsfehler mit $activeServerType Server: ${e.message}")
                    
                    if (activeServerType == SERVER_TYPE_PRIMARY) {
                        // Wenn der primäre Server nicht erreichbar ist, wechsle zum Backup
                        Log.i(TAG, "Wechsel zum Backup-Server")
                        setActiveServerType(SERVER_TYPE_BACKUP)
                        
                        // Neuen Request mit Backup-Server erstellen
                        val backupRequest = createRequestForServer(
                            originalRequest, 
                            BACKUP_API_SCHEME, 
                            BACKUP_API_HOST, 
                            BACKUP_API_PORT
                        )
                        
                        return chain.proceed(backupRequest)
                    } else {
                        // Wenn wir bereits den Backup-Server verwenden und dieser auch nicht
                        // erreichbar ist, prüfe den primären Server, falls genug Zeit vergangen ist
                        val lastPrimaryCheck = preferences.getLong(PREF_KEY_LAST_PRIMARY_CHECK, 0L)
                        val now = System.currentTimeMillis()
                        
                        if (now - lastPrimaryCheck > PRIMARY_CHECK_INTERVAL) {
                            Log.i(TAG, "Versuche Rückkehr zum primären Server")
                            preferences.edit().putLong(PREF_KEY_LAST_PRIMARY_CHECK, now).apply()
                            
                            try {
                                val checkRequest = createRequestForServer(
                                    originalRequest, 
                                    PRIMARY_API_SCHEME, 
                                    PRIMARY_API_HOST, 
                                    PRIMARY_API_PORT
                                )
                                
                                val response = chain.proceed(checkRequest)
                                
                                // Wenn der primäre Server wieder erreichbar ist, wechsle zurück
                                if (response.isSuccessful) {
                                    Log.i(TAG, "Primärer Server wieder erreichbar, wechsle zurück")
                                    setActiveServerType(SERVER_TYPE_PRIMARY)
                                }
                                
                                return response
                            } catch (checkEx: Exception) {
                                // Primärer Server immer noch nicht erreichbar
                                Log.w(TAG, "Primärer Server immer noch nicht erreichbar: ${checkEx.message}")
                                throw e // Original-Exception weitergeben
                            }
                        } else {
                            throw e // Original-Exception weitergeben, wenn noch nicht genug Zeit vergangen ist
                        }
                    }
                }
                else -> throw e // Andere Exceptions weitergeben
            }
        }
    }

    /**
     * Erstellt einen neuen Request für den angegebenen Server.
     */
    private fun createRequestForServer(
        originalRequest: Request,
        scheme: String,
        host: String,
        port: Int
    ): Request {
        val originalUrl = originalRequest.url
        val newUrl = originalUrl.newBuilder()
            .scheme(scheme)
            .host(host)
            .port(port)
            .build()
            
        return originalRequest.newBuilder()
            .url(newUrl)
            .build()
    }

    /**
     * Prüft, ob eine Netzwerkverbindung verfügbar ist.
     */
    private fun isNetworkAvailable(): Boolean {
        val connectivityManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        val network = connectivityManager.activeNetwork ?: return false
        val capabilities = connectivityManager.getNetworkCapabilities(network) ?: return false
        
        return capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
    }

    /**
     * Gibt den aktuell aktiven Server-Typ zurück.
     */
    private fun getActiveServerType(): String {
        return preferences.getString(PREF_KEY_ACTIVE_SERVER, SERVER_TYPE_PRIMARY) ?: SERVER_TYPE_PRIMARY
    }

    /**
     * Setzt den aktiven Server-Typ.
     */
    private fun setActiveServerType(serverType: String) {
        preferences.edit().putString(PREF_KEY_ACTIVE_SERVER, serverType).apply()
    }
}