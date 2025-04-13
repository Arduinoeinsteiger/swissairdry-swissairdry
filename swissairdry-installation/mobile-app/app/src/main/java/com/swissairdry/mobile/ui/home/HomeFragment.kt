package com.swissairdry.mobile.ui.home

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.view.isVisible
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.navigation.fragment.findNavController
import com.google.android.material.snackbar.Snackbar
import com.swissairdry.mobile.R
import com.swissairdry.mobile.databinding.FragmentHomeBinding
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber

/**
 * HomeFragment - Startseite der App
 *
 * Dieses Fragment zeigt eine Übersicht über Geräte, Aufträge und Kunden
 * und dient als Einstiegspunkt für die Navigation zu anderen Bereichen.
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
        
        setupListeners()
        setupObservers()
        
        // Daten laden
        viewModel.loadDashboardData()
    }

    private fun setupListeners() {
        // SwipeRefreshLayout konfigurieren
        binding.swipeRefreshLayout.setOnRefreshListener {
            viewModel.loadDashboardData()
        }
        
        // Retry-Button konfigurieren
        binding.retryButton.setOnClickListener {
            viewModel.loadDashboardData()
        }
        
        // Karten-Klick-Listener
        binding.cardDevices.setOnClickListener {
            findNavController().navigate(R.id.nav_devices)
        }
        
        binding.cardJobs.setOnClickListener {
            findNavController().navigate(R.id.nav_jobs)
        }
        
        binding.cardCustomers.setOnClickListener {
            findNavController().navigate(R.id.nav_customers)
        }
    }

    private fun setupObservers() {
        // Beobachter für den Ladezustand
        viewModel.isLoading.observe(viewLifecycleOwner) { isLoading ->
            binding.swipeRefreshLayout.isRefreshing = isLoading
            binding.progressBar.isVisible = isLoading && !binding.swipeRefreshLayout.isRefreshing
        }
        
        // Beobachter für Fehler
        viewModel.error.observe(viewLifecycleOwner) { errorMessage ->
            if (errorMessage != null) {
                binding.errorView.isVisible = true
                binding.contentView.isVisible = false
                binding.errorMessage.text = errorMessage
                
                Snackbar.make(binding.root, errorMessage, Snackbar.LENGTH_LONG).show()
                Timber.e("Fehler auf Dashboard: %s", errorMessage)
            } else {
                binding.errorView.isVisible = false
                binding.contentView.isVisible = true
            }
        }
        
        // Beobachter für Begrüßung
        viewModel.greeting.observe(viewLifecycleOwner) { greeting ->
            binding.greetingText.text = greeting
        }
        
        // Beobachter für Gerätezahlen
        viewModel.deviceStats.observe(viewLifecycleOwner) { stats ->
            binding.deviceCount.text = stats.total.toString()
            binding.deviceOnlineCount.text = stats.online.toString()
            binding.deviceWarningCount.text = stats.warning.toString()
            binding.deviceErrorCount.text = stats.error.toString()
        }
        
        // Beobachter für Auftragszahlen
        viewModel.jobStats.observe(viewLifecycleOwner) { stats ->
            binding.activeJobsCount.text = stats.active.toString()
            binding.pendingJobsCount.text = stats.pending.toString()
            binding.completedJobsCount.text = stats.completed.toString()
        }
        
        // Beobachter für Kundenzahlen
        viewModel.customerCount.observe(viewLifecycleOwner) { count ->
            binding.customersCount.text = count.toString()
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}