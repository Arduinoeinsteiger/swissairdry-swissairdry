package com.swissairdry.mobile.ui

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.view.GravityCompat
import androidx.drawerlayout.widget.DrawerLayout
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.NavController
import androidx.navigation.fragment.NavHostFragment
import androidx.navigation.ui.AppBarConfiguration
import androidx.navigation.ui.navigateUp
import androidx.navigation.ui.setupActionBarWithNavController
import androidx.navigation.ui.setupWithNavController
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.android.material.navigation.NavigationView
import com.google.android.material.snackbar.Snackbar
import com.swissairdry.mobile.R
import com.swissairdry.mobile.databinding.ActivityMainBinding
import com.swissairdry.mobile.utils.NetworkUtils
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber

/**
 * MainActivity - Hauptaktivität der App
 *
 * Diese Aktivität enthält die Navigation (Drawer und Bottom Navigation)
 * sowie den NavHostFragment für die verschiedenen Fragmente.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@AndroidEntryPoint
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var navController: NavController
    private lateinit var appBarConfiguration: AppBarConfiguration
    private lateinit var mainViewModel: MainViewModel

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // View Binding initialisieren
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        // ViewModel initialisieren
        mainViewModel = ViewModelProvider(this)[MainViewModel::class.java]
        
        // Toolbar einrichten
        val toolbar = binding.appBarMain.toolbar
        setSupportActionBar(toolbar)
        
        // Navigation einrichten
        setupNavigation()
        
        // Netzwerkstatus überwachen
        observeNetworkStatus()
        
        // User-Daten laden
        mainViewModel.loadUserData()
        
        // Beobachter für UI-Updates einrichten
        observeViewModel()
    }

    private fun setupNavigation() {
        // Navigation Controller holen
        val navHostFragment = supportFragmentManager
            .findFragmentById(R.id.nav_host_fragment) as NavHostFragment
        navController = navHostFragment.navController
        
        // Drawer Layout konfigurieren
        val drawerLayout: DrawerLayout = binding.drawerLayout
        val navView: NavigationView = binding.navView
        
        // Bottom Navigation konfigurieren
        val bottomNav: BottomNavigationView = binding.appBarMain.bottomNavView
        
        // Top-Level-Destinations konfigurieren (zeigen Hamburger-Menü statt Zurück-Pfeil)
        appBarConfiguration = AppBarConfiguration(
            setOf(
                R.id.nav_home, R.id.nav_devices, R.id.nav_jobs, 
                R.id.nav_customers, R.id.nav_reports
            ), 
            drawerLayout
        )
        
        // Toolbar mit NavController verbinden
        setupActionBarWithNavController(navController, appBarConfiguration)
        
        // NavigationView mit NavController verbinden
        navView.setupWithNavController(navController)
        
        // Bottom Navigation mit NavController verbinden
        bottomNav.setupWithNavController(navController)
        
        // Navigationsansichtswechsel abhängig vom Ziel
        navController.addOnDestinationChangedListener { _, destination, _ ->
            // Nur bei den Hauptdestinationen Bottom-Navigation anzeigen
            val isTopLevelDestination = appBarConfiguration.topLevelDestinations.contains(destination.id)
            bottomNav.visibility = if (isTopLevelDestination) View.VISIBLE else View.GONE
            
            Timber.d("Navigation zu: %s", destination.label)
        }
        
        // Drawer-Navigation Klicklistener
        navView.setNavigationItemSelectedListener { menuItem ->
            when (menuItem.itemId) {
                R.id.nav_home, R.id.nav_devices, R.id.nav_jobs, 
                R.id.nav_customers, R.id.nav_reports, R.id.nav_settings -> {
                    // Standard-Navigation
                    drawerLayout.closeDrawer(GravityCompat.START)
                    navController.navigate(menuItem.itemId)
                    true
                }
                R.id.nav_logout -> {
                    // Logout-Funktion
                    mainViewModel.logout()
                    true
                }
                else -> false
            }
        }
    }
    
    private fun observeViewModel() {
        // Benutzerinformationen beobachten
        mainViewModel.userData.observe(this) { user ->
            // Benutzerinfos im NavigationView Header aktualisieren
            val headerView = binding.navView.getHeaderView(0)
            val userNameTextView = headerView.findViewById<View>(R.id.user_name)
            val userRoleTextView = headerView.findViewById<View>(R.id.user_role)
            
            // TODO: Benutzerinformationen setzen
            // userName.text = user.displayName
            // userRole.text = user.role
        }
        
        // Logout-Status beobachten
        mainViewModel.logoutStatus.observe(this) { success ->
            if (success) {
                // Zur Login-Activity navigieren
                // Intent(this, LoginActivity::class.java).also {
                //    it.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
                //    startActivity(it)
                //    finish()
                // }
            }
        }
    }
    
    private fun observeNetworkStatus() {
        // Netzwerkstatus beobachten und anzeigen
        NetworkUtils.isNetworkAvailable.observe(this) { isAvailable ->
            if (!isAvailable) {
                Snackbar.make(
                    binding.root,
                    R.string.no_internet_connection,
                    Snackbar.LENGTH_LONG
                ).show()
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Menü in der Toolbar anzeigen
        menuInflater.inflate(R.menu.main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_refresh -> {
                // Aktuelles Fragment neu laden
                true
            }
            R.id.action_notifications -> {
                // Benachrichtigungen anzeigen
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        // Für die Zurück-Navigation in der Toolbar
        return navController.navigateUp(appBarConfiguration) || super.onSupportNavigateUp()
    }
    
    override fun onBackPressed() {
        // Schließt den Drawer falls offen, sonst normales Zurück-Verhalten
        if (binding.drawerLayout.isDrawerOpen(GravityCompat.START)) {
            binding.drawerLayout.closeDrawer(GravityCompat.START)
        } else {
            super.onBackPressed()
        }
    }
}