package com.swissairdry.mobile.ui

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.swissairdry.mobile.data.model.User
import com.swissairdry.mobile.data.preferences.UserPreferencesRepository
import com.swissairdry.mobile.data.repository.UserRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.launch
import timber.log.Timber
import javax.inject.Inject

/**
 * MainViewModel - ViewModel für die MainActivity
 *
 * Dieses ViewModel kümmert sich um Benutzerdaten, Logout-Funktionalität
 * und alle anderen UI-Daten, die für die MainActivity benötigt werden.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@HiltViewModel
class MainViewModel @Inject constructor(
    private val userRepository: UserRepository,
    private val userPreferencesRepository: UserPreferencesRepository
) : ViewModel() {

    private val _userData = MutableLiveData<User>()
    val userData: LiveData<User> = _userData

    private val _logoutStatus = MutableLiveData<Boolean>()
    val logoutStatus: LiveData<Boolean> = _logoutStatus

    /**
     * Lädt die Benutzerdaten aus dem Repository
     */
    fun loadUserData() {
        viewModelScope.launch {
            try {
                val user = userRepository.getCurrentUser()
                _userData.postValue(user)
            } catch (e: Exception) {
                Timber.e(e, "Fehler beim Laden der Benutzerdaten")
            }
        }
    }

    /**
     * Loggt den Benutzer aus und entfernt die Authentifizierungsdaten
     */
    fun logout() {
        viewModelScope.launch {
            try {
                // Token und Benutzerdaten löschen
                userPreferencesRepository.clearAuthToken()
                userRepository.logout()
                
                _logoutStatus.postValue(true)
                Timber.i("Benutzer erfolgreich ausgeloggt")
            } catch (e: Exception) {
                Timber.e(e, "Fehler beim Logout")
                _logoutStatus.postValue(false)
            }
        }
    }
}