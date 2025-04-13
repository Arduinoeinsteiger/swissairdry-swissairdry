/**
 * SwissAirDry - Dark Mode Toggle
 * 
 * Dieses Script steuert den Dark Mode der Anwendung.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

(function() {
  // DOM-Elemente
  const darkModeToggle = document.getElementById('darkModeToggle');
  
  // Bevorzugtes Farbschema des Benutzers prüfen
  const prefersDarkMode = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches;
  
  // Gespeicherte Einstellung aus localStorage abrufen oder Systemeinstellung verwenden
  const savedTheme = localStorage.getItem('swissairdry-theme');
  const initialTheme = savedTheme || (prefersDarkMode ? 'dark' : 'light');
  
  // Dark Mode initial setzen
  setTheme(initialTheme);
  
  // Event Listener für Toggle-Button
  if (darkModeToggle) {
    // Checkbox-Status basierend auf aktuellem Theme setzen
    darkModeToggle.checked = initialTheme === 'dark';
    
    // Event-Listener für Änderungen am Toggle-Button
    darkModeToggle.addEventListener('change', function() {
      const newTheme = this.checked ? 'dark' : 'light';
      setTheme(newTheme);
      
      // Einstellung im localStorage speichern
      localStorage.setItem('swissairdry-theme', newTheme);
    });
  }
  
  /**
   * Setzt das Theme der Anwendung.
   * 
   * @param {string} theme - 'light' oder 'dark'
   */
  function setTheme(theme) {
    if (theme === 'dark') {
      document.documentElement.setAttribute('data-theme', 'dark');
      if (darkModeToggle) darkModeToggle.checked = true;
      
      // Bootstrap-spezifische Klassen für Dark Mode
      document.body.classList.add('bg-dark');
      document.body.classList.add('text-light');
      
      // Navbar-Klassen anpassen
      const navbar = document.querySelector('.navbar');
      if (navbar) {
        navbar.classList.remove('navbar-light', 'bg-light');
        navbar.classList.add('navbar-dark', 'bg-dark');
      }
      
      // Footer-Klasse anpassen
      const footer = document.querySelector('.footer');
      if (footer) {
        footer.classList.remove('bg-light');
        footer.classList.add('bg-dark');
      }
    } else {
      document.documentElement.setAttribute('data-theme', 'light');
      if (darkModeToggle) darkModeToggle.checked = false;
      
      // Bootstrap-spezifische Klassen entfernen
      document.body.classList.remove('bg-dark');
      document.body.classList.remove('text-light');
      
      // Navbar-Klassen anpassen
      const navbar = document.querySelector('.navbar');
      if (navbar) {
        navbar.classList.remove('navbar-dark', 'bg-dark');
        navbar.classList.add('navbar-light', 'bg-light');
      }
      
      // Footer-Klasse anpassen
      const footer = document.querySelector('.footer');
      if (footer) {
        footer.classList.remove('bg-dark');
        footer.classList.add('bg-light');
      }
    }
  }
  
  // Event-Listener für Änderungen an den Systemeinstellungen
  window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', e => {
    // Nur anwenden, wenn keine gespeicherte Einstellung vorhanden ist
    if (!localStorage.getItem('swissairdry-theme')) {
      const newTheme = e.matches ? 'dark' : 'light';
      setTheme(newTheme);
    }
  });
})();