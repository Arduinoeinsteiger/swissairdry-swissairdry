package com.swissairdry.mobile.ui.home

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.lifecycle.lifecycleScope
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import com.swissairdry.mobile.R
import com.swissairdry.mobile.databinding.FragmentHomeBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * HomeFragment - Hauptdashboard der Anwendung
 *
 * Dieses Fragment zeigt eine Übersicht über aktuelle Aufträge, Gerätestatus
 * und anstehende Aufgaben an. Es dient als zentrale Anlaufstelle in der App.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@AndroidEntryPoint
class HomeFragment : Fragment() {

    private var _binding: FragmentHomeBinding? = null
    private val binding get() = _binding!!
    
    private val viewModel: HomeViewModel by viewModels()
    
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentHomeBinding.inflate(inflater, container, false)
        return binding.root
    }
    
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        
        setupObservers()
        setupListeners()
        
        // Daten laden
        viewModel.loadDashboardData()
    }
    
    private fun setupObservers() {
        lifecycleScope.launch {
            viewModel.uiState.collectLatest { state ->
                when (state) {
                    is HomeUiState.Loading -> showLoading()
                    is HomeUiState.Error -> showError(state.message)
                    is HomeUiState.Success -> updateDashboard(state)
                }
            }
        }
    }
    
    private fun setupListeners() {
        // Navigation zu den Geräten
        binding.cardDevices.setOnClickListener {
            findNavController().navigate(R.id.nav_devices)
        }
        
        // Navigation zu den Aufträgen
        binding.cardJobs.setOnClickListener {
            findNavController().navigate(R.id.nav_jobs)
        }
        
        // Navigation zu den Kunden
        binding.cardCustomers.setOnClickListener {
            findNavController().navigate(R.id.nav_customers)
        }
        
        // Aktualisieren der Daten per Pull-to-Refresh
        binding.swipeRefreshLayout.setOnRefreshListener {
            viewModel.loadDashboardData()
        }
    }
    
    private fun showLoading() {
        binding.progressBar.visibility = View.VISIBLE
        binding.errorView.visibility = View.GONE
        binding.contentView.visibility = View.GONE
    }
    
    private fun showError(message: String) {
        binding.progressBar.visibility = View.GONE
        binding.errorView.visibility = View.VISIBLE
        binding.contentView.visibility = View.GONE
        binding.swipeRefreshLayout.isRefreshing = false
        
        binding.errorMessage.text = message
        binding.retryButton.setOnClickListener {
            viewModel.loadDashboardData()
        }
        
        Timber.e("Fehler beim Laden der Dashboard-Daten: $message")
    }
    
    private fun updateDashboard(state: HomeUiState.Success) {
        binding.progressBar.visibility = View.GONE
        binding.errorView.visibility = View.GONE
        binding.contentView.visibility = View.VISIBLE
        binding.swipeRefreshLayout.isRefreshing = false
        
        // Gerätestatistiken aktualisieren
        binding.deviceCount.text = state.deviceStats.totalCount.toString()
        binding.deviceOnlineCount.text = state.deviceStats.onlineCount.toString()
        binding.deviceWarningCount.text = state.deviceStats.warningCount.toString()
        binding.deviceErrorCount.text = state.deviceStats.errorCount.toString()
        
        // Auftragsstatistiken aktualisieren
        binding.activeJobsCount.text = state.jobStats.activeCount.toString()
        binding.completedJobsCount.text = state.jobStats.completedCount.toString()
        binding.pendingJobsCount.text = state.jobStats.pendingCount.toString()
        
        // Kundenstatistiken aktualisieren
        binding.customersCount.text = state.customerCount.toString()
    }
    
    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}