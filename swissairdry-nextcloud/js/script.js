/**
 * SwissAirDry - Hauptskript
 * 
 * Enthält die clientseitige Logik für die SwissAirDry-App.
 * 
 * @copyright 2023-2025 Swiss Air Dry Team
 * @license AGPL-3.0-or-later
 */

document.addEventListener('DOMContentLoaded', function() {
    // Initialisierung
    initApp();
    
    // Event-Listener registrieren
    registerEventListeners();
});

/**
 * App initialisieren
 */
function initApp() {
    // Prüfen, ob ein Token vorhanden ist
    const token = localStorage.getItem('api_token');
    
    // Aktuelle Seite ermitteln
    const urlParams = new URLSearchParams(window.location.search);
    const action = urlParams.get('action') || 'dashboard';
    
    // Wenn kein Token vorhanden ist und nicht auf der Login-Seite,
    // zur Login-Seite weiterleiten
    if (!token && action !== 'login') {
        window.location.href = '?action=login';
        return;
    }
    
    // API-Status prüfen
    checkApiStatus();
}

/**
 * Event-Listener registrieren
 */
function registerEventListeners() {
    // Login-Formular
    const loginForm = document.getElementById('login-form');
    if (loginForm) {
        loginForm.addEventListener('submit', handleLogin);
    }
    
    // Logout-Button
    const logoutButton = document.getElementById('logout-button');
    if (logoutButton) {
        logoutButton.addEventListener('click', handleLogout);
    }
    
    // API-Status aktualisieren
    const refreshStatusButton = document.getElementById('refresh-status');
    if (refreshStatusButton) {
        refreshStatusButton.addEventListener('click', checkApiStatus);
    }
}

/**
 * API-Status prüfen
 */
function checkApiStatus() {
    fetch('?action=api&endpoint=health')
        .then(response => response.json())
        .then(data => {
            // Status-Anzeige aktualisieren
            const statusIndicator = document.getElementById('api-status');
            if (statusIndicator) {
                if (data.status === 'ok') {
                    statusIndicator.textContent = 'Online';
                    statusIndicator.className = 'status-indicator online';
                } else {
                    statusIndicator.textContent = 'Offline';
                    statusIndicator.className = 'status-indicator offline';
                }
            }
            
            // MQTT-Status aktualisieren
            const mqttIndicator = document.getElementById('mqtt-status');
            if (mqttIndicator) {
                if (data.mqtt) {
                    mqttIndicator.textContent = 'Verbunden';
                    mqttIndicator.className = 'status-indicator online';
                } else {
                    mqttIndicator.textContent = 'Nicht verbunden';
                    mqttIndicator.className = 'status-indicator offline';
                }
            }
        })
        .catch(error => {
            console.error('Fehler beim Prüfen des API-Status:', error);
            
            // Status-Anzeigen auf Offline setzen
            const statusIndicator = document.getElementById('api-status');
            if (statusIndicator) {
                statusIndicator.textContent = 'Offline';
                statusIndicator.className = 'status-indicator offline';
            }
            
            const mqttIndicator = document.getElementById('mqtt-status');
            if (mqttIndicator) {
                mqttIndicator.textContent = 'Nicht verbunden';
                mqttIndicator.className = 'status-indicator offline';
            }
        });
}

/**
 * Login-Formular verarbeiten
 * 
 * @param {Event} event Formular-Submit-Event
 */
function handleLogin(event) {
    event.preventDefault();
    
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    
    if (!username || !password) {
        showMessage('Bitte geben Sie Benutzername und Passwort ein.', 'error');
        return;
    }
    
    fetch('?action=api&endpoint=login', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({
            username: username,
            password: password
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok') {
            // Token speichern
            localStorage.setItem('api_token', data.token);
            
            // Zur Dashboard-Seite weiterleiten
            window.location.href = '?action=dashboard';
        } else {
            showMessage(data.message || 'Anmeldung fehlgeschlagen.', 'error');
        }
    })
    .catch(error => {
        console.error('Fehler bei der Anmeldung:', error);
        showMessage('Verbindungsfehler. Bitte versuchen Sie es später erneut.', 'error');
    });
}

/**
 * Abmelden
 */
function handleLogout() {
    // Token entfernen
    localStorage.removeItem('api_token');
    
    // Zur Login-Seite weiterleiten
    window.location.href = '?action=login';
}

/**
 * Meldung anzeigen
 * 
 * @param {string} message Meldungstext
 * @param {string} type Meldungstyp (success, error, warning, info)
 */
function showMessage(message, type = 'info') {
    const messageContainer = document.getElementById('message-container');
    
    if (!messageContainer) {
        // Meldungscontainer erstellen, falls nicht vorhanden
        const container = document.createElement('div');
        container.id = 'message-container';
        document.body.appendChild(container);
    }
    
    // Meldung erstellen
    const messageElement = document.createElement('div');
    messageElement.className = `message message-${type}`;
    messageElement.textContent = message;
    
    // Schließen-Button hinzufügen
    const closeButton = document.createElement('button');
    closeButton.className = 'message-close';
    closeButton.innerHTML = '&times;';
    closeButton.addEventListener('click', function() {
        messageElement.remove();
    });
    messageElement.appendChild(closeButton);
    
    // Meldung anzeigen
    document.getElementById('message-container').appendChild(messageElement);
    
    // Meldung nach 5 Sekunden automatisch ausblenden
    setTimeout(function() {
        if (messageElement.parentNode) {
            messageElement.remove();
        }
    }, 5000);
}