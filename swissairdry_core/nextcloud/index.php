<?php
/**
 * SwissAirDry - Nextcloud App
 * 
 * Haupteinstiegspunkt für die SwissAirDry-App
 * 
 * @copyright 2023-2025 Swiss Air Dry Team
 * @license AGPL-3.0-or-later
 */

// Autoloader laden
require_once __DIR__ . '/vendor/autoload.php';

// Konfiguration laden
$configFile = __DIR__ . '/config.json';
$config = [];
if (file_exists($configFile)) {
    $config = json_decode(file_get_contents($configFile), true) ?: [];
}

// Logger initialisieren
$logger = new SwissAirDry\Logger();
$logger->info('SwissAirDry-App gestartet');

// API-Service initialisieren
$apiService = new SwissAirDry\ApiService($logger);

// MQTT-Service initialisieren
$mqttService = new SwissAirDry\MQTTService($logger);

// Aktion aus URL-Parameter ermitteln
$action = $_GET['action'] ?? 'index';

// HTML-Header ausgeben
header('Content-Type: text/html; charset=UTF-8');
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SwissAirDry</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <header>
        <h1>SwissAirDry</h1>
        <nav>
            <ul>
                <li><a href="?action=dashboard">Dashboard</a></li>
                <li><a href="?action=devices">Geräte</a></li>
                <li><a href="?action=sensors">Sensoren</a></li>
                <li><a href="?action=jobs">Aufträge</a></li>
                <li><a href="?action=settings">Einstellungen</a></li>
            </ul>
        </nav>
    </header>
    
    <main>
        <?php
        // Je nach Aktion unterschiedlichen Inhalt anzeigen
        switch ($action) {
            case 'dashboard':
                include __DIR__ . '/templates/dashboard.php';
                break;
                
            case 'devices':
                include __DIR__ . '/templates/devices.php';
                break;
                
            case 'sensors':
                include __DIR__ . '/templates/sensors.php';
                break;
                
            case 'jobs':
                include __DIR__ . '/templates/jobs.php';
                break;
                
            case 'settings':
                include __DIR__ . '/templates/settings.php';
                break;
                
            case 'api':
                // API-Endpunkt
                header('Content-Type: application/json');
                include __DIR__ . '/api.php';
                exit;
                
            default:
                // Standardseite
                include __DIR__ . '/templates/dashboard.php';
        }
        ?>
    </main>
    
    <footer>
        <p>&copy; 2023-2025 Swiss Air Dry Team</p>
    </footer>
    
    <script src="js/script.js"></script>
</body>
</html>