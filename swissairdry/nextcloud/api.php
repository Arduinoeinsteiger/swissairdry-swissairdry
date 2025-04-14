<?php
/**
 * SwissAirDry - API-Endpunkte
 * 
 * Stellt eine REST-API für die Nextcloud-Integration bereit.
 * 
 * @copyright 2023-2025 Swiss Air Dry Team
 * @license AGPL-3.0-or-later
 */

// Nur JSON-Anfragen akzeptieren
header('Content-Type: application/json');

// Prüfen, ob die API-Services verfügbar sind
if (!isset($apiService) || !isset($mqttService) || !isset($logger)) {
    echo json_encode(['error' => 'API-Services nicht verfügbar']);
    exit;
}

// HTTP-Methode ermitteln
$method = $_SERVER['REQUEST_METHOD'];

// Endpunkt aus URL-Parameter ermitteln
$endpoint = $_GET['endpoint'] ?? '';

// Antwort vorbereiten
$response = ['status' => 'error', 'message' => 'Ungültiger Endpunkt'];

// Anfragedaten aus POST-Body oder GET-Parametern lesen
$data = [];
if ($method === 'POST' || $method === 'PUT' || $method === 'PATCH') {
    $input = file_get_contents('php://input');
    $data = json_decode($input, true) ?: [];
} else {
    $data = $_GET;
}

// Authentifizierung prüfen
$authenticated = false;
$token = null;

// Token aus HTTP-Header lesen
$authHeader = $_SERVER['HTTP_AUTHORIZATION'] ?? '';
if (preg_match('/Bearer\s+(.*)$/i', $authHeader, $matches)) {
    $token = $matches[1];
    if ($apiService->setToken($token)) {
        $authenticated = true;
    }
}

// API-Endpunkte verarbeiten
switch ($endpoint) {
    case 'health':
        // Gesundheitscheck - keine Authentifizierung erforderlich
        $response = [
            'status' => 'ok',
            'api' => $apiService->testConnection(),
            'mqtt' => $mqttService->isConnected(),
            'timestamp' => time()
        ];
        break;
        
    case 'login':
        // Anmeldung - keine Authentifizierung erforderlich
        if ($method === 'POST' && isset($data['username']) && isset($data['password'])) {
            if ($apiService->login($data['username'], $data['password'])) {
                $response = [
                    'status' => 'ok',
                    'message' => 'Anmeldung erfolgreich'
                ];
            } else {
                $response = [
                    'status' => 'error',
                    'message' => 'Ungültige Anmeldedaten'
                ];
            }
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Ungültige Anfrage'
            ];
        }
        break;
        
    case 'devices':
        // Geräteliste abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET') {
            $devices = $apiService->getDevices();
            $response = [
                'status' => 'ok',
                'data' => $devices
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Methode nicht erlaubt'
            ];
        }
        break;
        
    case 'device':
        // Geräteinformationen abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET' && isset($data['id'])) {
            $device = $apiService->getDevice($data['id']);
            $response = [
                'status' => 'ok',
                'data' => $device
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Ungültige Anfrage'
            ];
        }
        break;
        
    case 'sensors':
        // Sensorliste abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET') {
            $sensors = $apiService->getSensors();
            $response = [
                'status' => 'ok',
                'data' => $sensors
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Methode nicht erlaubt'
            ];
        }
        break;
        
    case 'sensor':
        // Sensorinformationen abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET' && isset($data['id'])) {
            $sensor = $apiService->getSensor($data['id']);
            $response = [
                'status' => 'ok',
                'data' => $sensor
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Ungültige Anfrage'
            ];
        }
        break;
        
    case 'jobs':
        // Auftragsliste abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET') {
            $jobs = $apiService->getJobs();
            $response = [
                'status' => 'ok',
                'data' => $jobs
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Methode nicht erlaubt'
            ];
        }
        break;
        
    case 'job':
        // Auftragsinformationen abrufen - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'GET' && isset($data['id'])) {
            $job = $apiService->getJob($data['id']);
            $response = [
                'status' => 'ok',
                'data' => $job
            ];
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Ungültige Anfrage'
            ];
        }
        break;
        
    case 'mqtt-publish':
        // MQTT-Nachricht senden - Authentifizierung erforderlich
        if (!$authenticated) {
            $response = ['status' => 'error', 'message' => 'Nicht authentifiziert'];
            break;
        }
        
        if ($method === 'POST' && isset($data['topic']) && isset($data['message'])) {
            $qos = $data['qos'] ?? 1;
            $retain = $data['retain'] ?? false;
            
            if ($mqttService->publish($data['topic'], $data['message'], $qos, $retain)) {
                $response = [
                    'status' => 'ok',
                    'message' => 'Nachricht gesendet'
                ];
            } else {
                $response = [
                    'status' => 'error',
                    'message' => 'Fehler beim Senden der Nachricht'
                ];
            }
        } else {
            $response = [
                'status' => 'error',
                'message' => 'Ungültige Anfrage'
            ];
        }
        break;
}

// Antwort ausgeben
echo json_encode($response);