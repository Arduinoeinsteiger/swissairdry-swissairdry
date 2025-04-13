package com.swissairdry.mobile.ui.home

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.swissairdry.mobile.data.models.Device
import com.swissairdry.mobile.data.repositories.CustomerRepository
import com.swissairdry.mobile.data.repositories.DeviceRepository
import com.swissairdry.mobile.data.repositories.JobRepository
import com.swissairdry.mobile.utils.NetworkResult
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import timber.log.Timber
import javax.inject.Inject

/**
 * HomeViewModel - ViewModel für das Dashboard
 *
 * Dieses ViewModel verwaltet die Daten für das Dashboard und stellt sie
 * dem HomeFragment zur Verfügung.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@HiltViewModel
class HomeViewModel @Inject constructor(
    private val deviceRepository: DeviceRepository,
    private val jobRepository: JobRepository,
    private val customerRepository: CustomerRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow<HomeUiState>(HomeUiState.Loading)
    val uiState: StateFlow<HomeUiState> = _uiState

    fun loadDashboardData() {
        _uiState.value = HomeUiState.Loading
        
        viewModelScope.launch {
            try {
                // Geräte abrufen
                val devicesResult = deviceRepository.getDevices(page = 1, limit = 100)
                
                // Aufträge abrufen
                val jobsResult = jobRepository.getJobs(page = 1, limit = 100)
                
                // Kunden abrufen
                val customersResult = customerRepository.getCustomers(page = 1, limit = 100)
                
                if (devicesResult is NetworkResult.Success && 
                    jobsResult is NetworkResult.Success && 
                    customersResult is NetworkResult.Success) {
                    
                    // Gerätestatistiken berechnen
                    val devices = devicesResult.data?.items ?: emptyList()
                    val deviceStats = calculateDeviceStats(devices)
                    
                    // Auftragsstatistiken berechnen
                    val jobs = jobsResult.data?.items ?: emptyList()
                    val jobStats = JobStats(
                        activeCount = jobs.count { it.status == "active" },
                        completedCount = jobs.count { it.status == "completed" },
                        pendingCount = jobs.count { it.status == "pending" }
                    )
                    
                    // Kunden zählen
                    val customerCount = customersResult.data?.total ?: 0
                    
                    _uiState.update { 
                        HomeUiState.Success(
                            deviceStats = deviceStats,
                            jobStats = jobStats,
                            customerCount = customerCount
                        )
                    }
                } else {
                    val errorMessage = when {
                        devicesResult is NetworkResult.Error -> devicesResult.message
                        jobsResult is NetworkResult.Error -> jobsResult.message
                        customersResult is NetworkResult.Error -> customersResult.message
                        else -> "Fehler beim Abrufen der Daten"
                    }
                    
                    _uiState.update { HomeUiState.Error(errorMessage ?: "Unbekannter Fehler") }
                    Timber.e("Fehler beim Laden der Dashboard-Daten: $errorMessage")
                }
            } catch (e: Exception) {
                _uiState.update { HomeUiState.Error(e.localizedMessage ?: "Unbekannter Fehler") }
                Timber.e(e, "Ausnahme beim Laden der Dashboard-Daten")
            }
        }
    }
    
    private fun calculateDeviceStats(devices: List<Device>): DeviceStats {
        val onlineCount = devices.count { it.status == "online" }
        val warningCount = devices.count { it.status == "warning" }
        val errorCount = devices.count { it.status == "error" }
        
        return DeviceStats(
            totalCount = devices.size,
            onlineCount = onlineCount,
            warningCount = warningCount,
            errorCount = errorCount
        )
    }
}

/**
 * Datenklasse für Gerätestatistiken
 */
data class DeviceStats(
    val totalCount: Int,
    val onlineCount: Int,
    val warningCount: Int,
    val errorCount: Int
)

/**
 * Datenklasse für Auftragsstatistiken
 */
data class JobStats(
    val activeCount: Int,
    val completedCount: Int,
    val pendingCount: Int
)

/**
 * Sealed Class für den UI-Zustand
 */
sealed class HomeUiState {
    object Loading : HomeUiState()
    data class Error(val message: String) : HomeUiState()
    data class Success(
        val deviceStats: DeviceStats,
        val jobStats: JobStats,
        val customerCount: Int
    ) : HomeUiState()
}