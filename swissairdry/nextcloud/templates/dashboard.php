<div class="dashboard">
    <h2>Dashboard</h2>
    
    <div class="stats-overview">
        <div class="stat-card">
            <h3>Geräte</h3>
            <div class="stat-value" id="device-count">--</div>
            <div class="stat-details">
                <span class="active" id="active-devices">-- aktiv</span>
                <span class="offline" id="offline-devices">-- offline</span>
            </div>
        </div>
        
        <div class="stat-card">
            <h3>Sensoren</h3>
            <div class="stat-value" id="sensor-count">--</div>
            <div class="stat-details">
                <span class="ok" id="ok-sensors">-- OK</span>
                <span class="warning" id="warning-sensors">-- Warnung</span>
            </div>
        </div>
        
        <div class="stat-card">
            <h3>Aufträge</h3>
            <div class="stat-value" id="job-count">--</div>
            <div class="stat-details">
                <span class="active" id="active-jobs">-- aktiv</span>
                <span class="completed" id="completed-jobs">-- abgeschlossen</span>
            </div>
        </div>
    </div>
    
    <div class="charts-container">
        <div class="chart-card">
            <h3>Temperaturverlauf</h3>
            <div id="temp-chart" class="chart">
                <p class="loading">Daten werden geladen...</p>
            </div>
        </div>
        
        <div class="chart-card">
            <h3>Feuchtigkeitsverlauf</h3>
            <div id="humidity-chart" class="chart">
                <p class="loading">Daten werden geladen...</p>
            </div>
        </div>
    </div>
    
    <div class="alerts-container">
        <h3>Aktuelle Meldungen</h3>
        <ul id="alerts-list">
            <li class="alert-placeholder">Keine aktuellen Meldungen</li>
        </ul>
    </div>
</div>

<script>
// Dashboard-Daten laden
document.addEventListener('DOMContentLoaded', function() {
    // API-Token aus localStorage holen
    const token = localStorage.getItem('api_token');
    
    if (!token) {
        // Nicht angemeldet, zur Login-Seite weiterleiten
        window.location.href = '?action=login';
        return;
    }
    
    // Statistiken laden
    loadStatistics(token);
    
    // Diagramme laden
    loadCharts(token);
    
    // Meldungen laden
    loadAlerts(token);
    
    // Automatische Aktualisierung alle 60 Sekunden
    setInterval(function() {
        loadStatistics(token);
        loadAlerts(token);
    }, 60000);
});

// Statistiken laden
function loadStatistics(token) {
    // Geräte zählen
    fetch('?action=api&endpoint=devices', {
        headers: {
            'Authorization': 'Bearer ' + token
        }
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok' && data.data) {
            const devices = data.data;
            const activeDevices = devices.filter(d => d.status === 'active').length;
            const offlineDevices = devices.filter(d => d.status === 'offline').length;
            
            document.getElementById('device-count').textContent = devices.length;
            document.getElementById('active-devices').textContent = activeDevices + ' aktiv';
            document.getElementById('offline-devices').textContent = offlineDevices + ' offline';
        }
    })
    .catch(error => console.error('Fehler beim Laden der Geräte:', error));
    
    // Sensoren zählen
    fetch('?action=api&endpoint=sensors', {
        headers: {
            'Authorization': 'Bearer ' + token
        }
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok' && data.data) {
            const sensors = data.data;
            const okSensors = sensors.filter(s => s.status === 'ok').length;
            const warningSensors = sensors.filter(s => s.status === 'warning').length;
            
            document.getElementById('sensor-count').textContent = sensors.length;
            document.getElementById('ok-sensors').textContent = okSensors + ' OK';
            document.getElementById('warning-sensors').textContent = warningSensors + ' Warnung';
        }
    })
    .catch(error => console.error('Fehler beim Laden der Sensoren:', error));
    
    // Aufträge zählen
    fetch('?action=api&endpoint=jobs', {
        headers: {
            'Authorization': 'Bearer ' + token
        }
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'ok' && data.data) {
            const jobs = data.data;
            const activeJobs = jobs.filter(j => j.status === 'active').length;
            const completedJobs = jobs.filter(j => j.status === 'completed').length;
            
            document.getElementById('job-count').textContent = jobs.length;
            document.getElementById('active-jobs').textContent = activeJobs + ' aktiv';
            document.getElementById('completed-jobs').textContent = completedJobs + ' abgeschlossen';
        }
    })
    .catch(error => console.error('Fehler beim Laden der Aufträge:', error));
}

// Diagramme laden
function loadCharts(token) {
    // TODO: Diagramme mit Chart.js oder ähnlicher Bibliothek implementieren
    document.querySelectorAll('.chart .loading').forEach(el => {
        el.textContent = 'Diagramme werden in einer späteren Version implementiert';
    });
}

// Meldungen laden
function loadAlerts(token) {
    // Meldungen über die API laden
    // TODO: Implementieren, sobald die entsprechende API verfügbar ist
    const alertsList = document.getElementById('alerts-list');
    alertsList.innerHTML = '<li class="alert-placeholder">Meldungen werden in einer späteren Version implementiert</li>';
}
</script>