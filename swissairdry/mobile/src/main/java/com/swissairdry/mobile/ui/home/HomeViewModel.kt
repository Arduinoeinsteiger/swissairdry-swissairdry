package com.swissairdry.mobile.ui.home

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.swissairdry.mobile.data.model.User
import com.swissairdry.mobile.data.repository.CustomerRepository
import com.swissairdry.mobile.data.repository.DeviceRepository
import com.swissairdry.mobile.data.repository.JobRepository
import com.swissairdry.mobile.data.repository.UserRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.async
import kotlinx.coroutines.launch
import timber.log.Timber
import java.text.SimpleDateFormat
import java.util.*
import javax.inject.Inject

/**
 * HomeViewModel - ViewModel für die Startseite
 *
 * Dieses ViewModel ist für die Datenbereitstellung der Startseite zuständig
 * und lädt die Übersichtsdaten für Geräte, Aufträge und Kunden.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@HiltViewModel
class HomeViewModel @Inject constructor(
    private val userRepository: UserRepository,
    private val deviceRepository: DeviceRepository,
    private val jobRepository: JobRepository,
    private val customerRepository: CustomerRepository
) : ViewModel() {

    private val _isLoading = MutableLiveData<Boolean>()
    val isLoading: LiveData<Boolean> = _isLoading

    private val _error = MutableLiveData<String?>()
    val error: LiveData<String?> = _error

    private val _greeting = MutableLiveData<String>()
    val greeting: LiveData<String> = _greeting

    private val _deviceStats = MutableLiveData<DeviceStats>()
    val deviceStats: LiveData<DeviceStats> = _deviceStats

    private val _jobStats = MutableLiveData<JobStats>()
    val jobStats: LiveData<JobStats> = _jobStats

    private val _customerCount = MutableLiveData<Int>()
    val customerCount: LiveData<Int> = _customerCount

    // Initialisieren mit Standardwerten
    init {
        _deviceStats.value = DeviceStats(0, 0, 0, 0)
        _jobStats.value = JobStats(0, 0, 0)
        _customerCount.value = 0
        _greeting.value = getGreetingByTime()
    }

    /**
     * Lädt alle Daten für das Dashboard
     */
    fun loadDashboardData() {
        viewModelScope.launch {
            try {
                _isLoading.value = true
                _error.value = null

                // Aktuelle Benutzerinformationen laden
                val currentUser = loadCurrentUser()
                
                // Personalisierten Gruß setzen
                _greeting.value = getPersonalizedGreeting(currentUser)

                // Parallele Datenladung
                val deviceStatsDeferred = async { loadDeviceStats() }
                val jobStatsDeferred = async { loadJobStats() }
                val customerCountDeferred = async { loadCustomerCount() }

                // Ergebnisse sammeln
                _deviceStats.value = deviceStatsDeferred.await()
                _jobStats.value = jobStatsDeferred.await()
                _customerCount.value = customerCountDeferred.await()

                _isLoading.value = false
            } catch (e: Exception) {
                Timber.e(e, "Fehler beim Laden der Dashboard-Daten")
                _error.value = "Daten konnten nicht geladen werden. Bitte versuchen Sie es später erneut."
                _isLoading.value = false
            }
        }
    }

    /**
     * Lädt den aktuellen Benutzer aus dem Repository
     */
    private suspend fun loadCurrentUser(): User? {
        return try {
            userRepository.getCurrentUser()
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden des Benutzers")
            null
        }
    }

    /**
     * Lädt die Gerätezahlen aus dem Repository
     */
    private suspend fun loadDeviceStats(): DeviceStats {
        return try {
            val devices = deviceRepository.getDevices()
            val total = devices.size
            val online = devices.count { it.status == "online" }
            val warning = devices.count { it.status == "warning" }
            val error = devices.count { it.status == "error" }
            
            DeviceStats(total, online, warning, error)
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden der Gerätezahlen")
            DeviceStats(0, 0, 0, 0)
        }
    }

    /**
     * Lädt die Auftragszahlen aus dem Repository
     */
    private suspend fun loadJobStats(): JobStats {
        return try {
            val jobs = jobRepository.getJobs()
            val active = jobs.count { it.status == "active" }
            val pending = jobs.count { it.status == "pending" }
            val completed = jobs.count { it.status == "completed" }
            
            JobStats(active, pending, completed)
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden der Auftragszahlen")
            JobStats(0, 0, 0)
        }
    }

    /**
     * Lädt die Kundenzahl aus dem Repository
     */
    private suspend fun loadCustomerCount(): Int {
        return try {
            customerRepository.getCustomers().size
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden der Kundenzahl")
            0
        }
    }

    /**
     * Erstellt einen personalisierten Gruß basierend auf dem aktuellen Benutzer
     */
    private fun getPersonalizedGreeting(user: User?): String {
        val baseGreeting = getGreetingByTime()
        return if (user != null) {
            "$baseGreeting, ${user.firstName}!"
        } else {
            "$baseGreeting!"
        }
    }

    /**
     * Erstellt einen Gruß basierend auf der Tageszeit
     */
    private fun getGreetingByTime(): String {
        val calendar = Calendar.getInstance()
        val hourOfDay = calendar.get(Calendar.HOUR_OF_DAY)
        
        return when {
            hourOfDay < 12 -> "Guten Morgen"
            hourOfDay < 18 -> "Guten Tag"
            else -> "Guten Abend"
        }
    }

    /**
     * Datenklasse für Gerätezahlen
     */
    data class DeviceStats(
        val total: Int,
        val online: Int,
        val warning: Int,
        val error: Int
    )

    /**
     * Datenklasse für Auftragszahlen
     */
    data class JobStats(
        val active: Int,
        val pending: Int,
        val completed: Int
    )
}