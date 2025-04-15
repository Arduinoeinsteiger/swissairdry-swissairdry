/**
 * SwissAirDry - Web App
 * 
 * Hauptskript für die SwissAirDry-Webanwendung
 */

document.addEventListener('DOMContentLoaded', function() {
  // Initialisierung
  initApp();
  
  // Event-Listener registrieren
  registerEventListeners();
  
  // API-Status prüfen
  checkApiStatus();
  
  // Beispieldaten laden
  loadDashboardData();
});

/**
 * App initialisieren
 */
function initApp() {
  // Login-Status prüfen
  const token = localStorage.getItem('api_token');
  
  if (!token) {
    // Nicht eingeloggt, zur Login-Seite weiterleiten
    console.log('Nicht eingeloggt, Weiterleitung zur Login-Seite...');
    // In einer echten App: window.location.href = 'login.html';
  }
  
  // Aktuelle Seite aus URL-Hash ermitteln und anzeigen
  const hash = window.location.hash || '#dashboard';
  showSection(hash.substring(1));
}

/**
 * Event-Listener registrieren
 */
function registerEventListeners() {
  // Navigation
  document.querySelectorAll('nav a').forEach(link => {
    link.addEventListener('click', function(e) {
      const section = this.getAttribute('href').substring(1);
      showSection(section);
    });
  });
  
  // Logout-Button
  const logoutBtn = document.getElementById('logout-btn');
  if (logoutBtn) {
    logoutBtn.addEventListener('click', function() {
      localStorage.removeItem('api_token');
      console.log('Abgemeldet, Weiterleitung zur Login-Seite...');
      // In einer echten App: window.location.href = 'login.html';
    });
  }
}

/**
 * Abschnitt anzeigen
 */
function showSection(sectionId) {
  // Alle Abschnitte ausblenden
  document.querySelectorAll('main section').forEach(section => {
    section.classList.remove('active-section');
  });
  
  // Gewählten Abschnitt einblenden
  const activeSection = document.getElementById(sectionId);
  if (activeSection) {
    activeSection.classList.add('active-section');
  }
  
  // URL-Hash aktualisieren
  window.location.hash = '#' + sectionId;
}

/**
 * API-Status prüfen
 */
function checkApiStatus() {
  // Beispiel für API-Status-Prüfung
  fetch('/api/health')
    .then(response => {
      if (response.ok) {
        updateApiStatus('online');
      } else {
        updateApiStatus('offline');
      }
    })
    .catch(error => {
      console.error('API-Status-Prüfung fehlgeschlagen:', error);
      updateApiStatus('offline');
    });
  
  // Status alle 60 Sekunden aktualisieren
  setTimeout(checkApiStatus, 60000);
}

/**
 * API-Status aktualisieren
 */
function updateApiStatus(status) {
  const statusIndicator = document.querySelector('.status-indicator');
  
  if (statusIndicator) {
    statusIndicator.className = 'status-indicator ' + status;
    statusIndicator.textContent = status === 'online' ? 'Online' : 'Offline';
  }
}

/**
 * Dashboard-Daten laden
 */
function loadDashboardData() {
  // In einer echten App würden hier Daten von der API geladen werden
  console.log('Dashboard-Daten werden geladen...');
}