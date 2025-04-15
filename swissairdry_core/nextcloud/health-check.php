<?php
/**
 * Gesundheitscheck für die SwissAirDry-App
 * 
 * Prüft, ob die App korrekt funktioniert und alle Abhängigkeiten erreichbar sind.
 */

// Prüfen, ob die MQTT-Verbindung funktioniert
function checkMqtt() {
    $broker = getenv('MQTT_BROKER') ?: 'localhost';
    $port = (int)(getenv('MQTT_PORT') ?: 1883);
    
    $client = new Mosquitto\Client('health-check');
    try {
        $client->connect($broker, $port, 5);
        return true;
    } catch (Exception $e) {
        echo "MQTT-Fehler: " . $e->getMessage() . "\n";
        return false;
    }
}

// Prüfen, ob die API erreichbar ist
function checkApi() {
    $apiUrl = getenv('API_URL') ?: 'http://localhost:5000';
    $apiUrl .= '/health';
    
    $ch = curl_init($apiUrl);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_TIMEOUT, 5);
    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);
    
    if ($httpCode == 200) {
        return true;
    } else {
        echo "API-Fehler: HTTP-Code $httpCode\n";
        return false;
    }
}

// Ausführung der Prüfungen
$mqttOk = checkMqtt();
$apiOk = checkApi();

// Gesamtergebnis
if ($mqttOk && $apiOk) {
    echo "Gesundheitscheck erfolgreich!\n";
    exit(0);
} else {
    echo "Gesundheitscheck fehlgeschlagen!\n";
    exit(1);
}