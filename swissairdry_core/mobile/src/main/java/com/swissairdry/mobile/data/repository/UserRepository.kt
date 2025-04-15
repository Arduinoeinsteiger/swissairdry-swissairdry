package com.swissairdry.mobile.data.repository

import com.swissairdry.mobile.api.ApiService
import com.swissairdry.mobile.api.LoginRequest
import com.swissairdry.mobile.data.model.User
import com.swissairdry.mobile.data.preferences.UserPreferencesRepository
import timber.log.Timber
import javax.inject.Inject
import javax.inject.Singleton

/**
 * UserRepository - Repository für Benutzerfunktionen
 *
 * Dieses Repository verwaltet alle Operationen im Zusammenhang mit Benutzern,
 * einschließlich Authentifizierung, Benutzerprofilaktualisierungen und 
 * Berechtigungsverwaltung.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@Singleton
class UserRepository @Inject constructor(
    private val apiService: ApiService,
    private val userPreferencesRepository: UserPreferencesRepository
) {
    private var currentUser: User? = null

    /**
     * Loggt einen Benutzer ein und speichert die Authentifizierungsdaten
     */
    suspend fun login(username: String, password: String): User {
        val response = apiService.login(LoginRequest(username, password))
        
        if (!response.isSuccessful) {
            throw Exception("Login fehlgeschlagen: ${response.message()}")
        }
        
        val loginResponse = response.body() ?: throw Exception("Keine Daten von der API erhalten")
        
        // Token speichern
        userPreferencesRepository.saveAuthToken(loginResponse.token)
        userPreferencesRepository.saveRefreshToken(loginResponse.refreshToken)
        
        // Benutzer speichern
        currentUser = loginResponse.user
        
        Timber.i("Benutzer erfolgreich eingeloggt: ${loginResponse.user.username}")
        
        return loginResponse.user
    }

    /**
     * Loggt den aktuellen Benutzer aus
     */
    suspend fun logout() {
        try {
            // Bei der API abmelden
            val response = apiService.logout()
            
            if (!response.isSuccessful) {
                Timber.w("Logout-API-Aufruf fehlgeschlagen: ${response.message()}")
            }
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Logout-API-Aufruf")
        } finally {
            // Lokale Daten immer löschen, auch wenn API-Aufruf fehlschlägt
            userPreferencesRepository.clearAuthToken()
            userPreferencesRepository.clearRefreshToken()
            currentUser = null
        }
    }

    /**
     * Holt den aktuellen Benutzer, wenn nicht vorhanden, lädt er ihn von der API
     */
    suspend fun getCurrentUser(): User {
        // Wenn bereits geladen, verwende den Cache
        currentUser?.let { return it }
        
        // Sonst vom Server laden
        val response = apiService.getCurrentUser()
        
        if (!response.isSuccessful) {
            throw Exception("Fehler beim Laden des Benutzers: ${response.message()}")
        }
        
        val user = response.body() ?: throw Exception("Keine Benutzerdaten erhalten")
        
        // Im Cache speichern
        currentUser = user
        
        return user
    }

    /**
     * Holt einen bestimmten Benutzer anhand seiner ID
     */
    suspend fun getUserById(userId: Int): User {
        val response = apiService.getUserById(userId)
        
        if (!response.isSuccessful) {
            throw Exception("Fehler beim Laden des Benutzers: ${response.message()}")
        }
        
        return response.body() ?: throw Exception("Keine Benutzerdaten erhalten")
    }

    /**
     * Aktualisiert den Token, wenn er abgelaufen ist
     */
    suspend fun refreshToken(): Boolean {
        try {
            val response = apiService.refreshToken()
            
            if (!response.isSuccessful) {
                Timber.w("Token-Aktualisierung fehlgeschlagen: ${response.message()}")
                return false
            }
            
            val loginResponse = response.body() ?: return false
            
            // Neuen Token speichern
            userPreferencesRepository.saveAuthToken(loginResponse.token)
            userPreferencesRepository.saveRefreshToken(loginResponse.refreshToken)
            
            Timber.i("Token erfolgreich aktualisiert")
            
            return true
        } catch (e: Exception) {
            Timber.e(e, "Fehler bei der Token-Aktualisierung")
            return false
        }
    }
}