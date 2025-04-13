package com.swissairdry.mobile.ui

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavController
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import com.swissairdry.mobile.R
import com.swissairdry.mobile.data.preferences.UserPreferencesRepository
import com.swissairdry.mobile.databinding.ActivityMainBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import timber.log.Timber
import javax.inject.Inject

/**
 * Hauptaktivität für die SwissAirDry Anwendung
 * 
 * Diese Aktivität dient als Container für die verschiedenen Fragmente
 * und verwaltet die Navigation innerhalb der App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@AndroidEntryPoint
class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private lateinit var navController: NavController
    private lateinit var appBarConfiguration: AppBarConfiguration
    
    @Inject
    lateinit var userPreferencesRepository: UserPreferencesRepository
    
    override fun onCreate(savedInstanceState: Bundle?) {
        // Splash Screen zeigen
        installSplashScreen()
        
        super.onCreate(savedInstanceState)
        
        // View Binding initialisieren
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // Toolbar einrichten
        setSupportActionBar(binding.toolbar)
        
        // Navigation einrichten
        val navHostFragment = supportFragmentManager
            .findFragmentById(R.id.nav_host_fragment) as NavHostFragment
        navController = navHostFragment.navController
        
        // App Bar Konfiguration mit Top-Level-Zielen
        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.nav_home, 
                R.id.nav_devices,
                R.id.nav_customers,
                R.id.nav_jobs,
                R.id.nav_measurements,
                R.id.nav_reports,
                R.id.nav_settings
            ),
            binding.drawerLayout
        )
        
        // Navigation mit Action Bar verbinden
        setupActionBarWithNavController(navController, appBarConfiguration)
        
        // NavigationView mit NavController verbinden
        binding.navView.setupWithNavController(navController)
        
        // BottomNavigationView mit NavController verbinden
        binding.bottomNavView?.setupWithNavController(navController)
        
        // Benutzeranmeldestatus prüfen und ggf. zur Login-Seite navigieren
        lifecycleScope.launch {
            val isLoggedIn = userPreferencesRepository.isLoggedIn().first()
            if (!isLoggedIn) {
                navController.navigate(R.id.loginFragment)
            }
        }
        
        // Auf Navigation Events reagieren
        navController.addOnDestinationChangedListener { _, destination, _ ->
            Timber.d("Navigation zu: ${destination.label}")
        }
    }
    
    override fun onSupportNavigateUp(): Boolean {
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }
}