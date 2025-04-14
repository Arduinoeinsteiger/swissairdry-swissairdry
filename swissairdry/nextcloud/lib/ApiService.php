<?php
namespace SwissAirDry;

/**
 * API-Service für die SwissAirDry-App
 * 
 * Stellt Funktionen für die Kommunikation mit der SwissAirDry-API bereit.
 */
class ApiService {
    private $apiUrl;
    private $logger;
    private $token;

    /**
     * Konstruktor
     * 
     * @param \Psr\Log\LoggerInterface $logger Logger-Instanz
     */
    public function __construct(\Psr\Log\LoggerInterface $logger = null) {
        $this->logger = $logger;
        $this->apiUrl = getenv('API_URL') ?: 'http://localhost:5000';
        
        // API-URL normalisieren
        if (substr($this->apiUrl, -1) == '/') {
            $this->apiUrl = substr($this->apiUrl, 0, -1);
        }
    }

    /**
     * Mit Benutzernamen und Passwort anmelden
     * 
     * @param string $username Benutzername
     * @param string $password Passwort
     * @return bool Erfolgreich angemeldet?
     */
    public function login($username, $password) {
        $response = $this->request('/auth/token', 'POST', [
            'username' => $username,
            'password' => $password
        ]);
        
        if (isset($response['access_token'])) {
            $this->token = $response['access_token'];
            return true;
        }
        
        return false;
    }

    /**
     * Mit Token anmelden
     * 
     * @param string $token JWT-Token
     * @return bool Token gültig?
     */
    public function setToken($token) {
        $this->token = $token;
        
        // Token testen
        $response = $this->request('/auth/me');
        return isset($response['username']);
    }

    /**
     * Liste aller Geräte abrufen
     * 
     * @return array Geräteliste
     */
    public function getDevices() {
        return $this->request('/devices');
    }

    /**
     * Detailinformationen zu einem Gerät abrufen
     * 
     * @param string $deviceId Geräte-ID
     * @return array Geräteinformationen
     */
    public function getDevice($deviceId) {
        return $this->request('/devices/' . $deviceId);
    }

    /**
     * Liste aller Sensoren abrufen
     * 
     * @return array Sensorliste
     */
    public function getSensors() {
        return $this->request('/sensors');
    }

    /**
     * Detailinformationen zu einem Sensor abrufen
     * 
     * @param string $sensorId Sensor-ID
     * @return array Sensorinformationen
     */
    public function getSensor($sensorId) {
        return $this->request('/sensors/' . $sensorId);
    }

    /**
     * Liste aller Aufträge abrufen
     * 
     * @return array Auftragsliste
     */
    public function getJobs() {
        return $this->request('/jobs');
    }

    /**
     * Detailinformationen zu einem Auftrag abrufen
     * 
     * @param string $jobId Auftrags-ID
     * @return array Auftragsinformationen
     */
    public function getJob($jobId) {
        return $this->request('/jobs/' . $jobId);
    }

    /**
     * API-Anfrage senden
     * 
     * @param string $endpoint API-Endpunkt
     * @param string $method HTTP-Methode
     * @param array $data Anfragedaten
     * @return array Antwort
     */
    private function request($endpoint, $method = 'GET', $data = null) {
        $url = $this->apiUrl . '/api/v1' . $endpoint;
        
        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
        curl_setopt($ch, CURLOPT_TIMEOUT, 30);
        
        $headers = ['Accept: application/json'];
        
        if ($this->token) {
            $headers[] = 'Authorization: Bearer ' . $this->token;
        }
        
        if ($method == 'POST' || $method == 'PUT' || $method == 'PATCH') {
            curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $method);
            
            if ($data) {
                $json = json_encode($data);
                curl_setopt($ch, CURLOPT_POSTFIELDS, $json);
                $headers[] = 'Content-Type: application/json';
                $headers[] = 'Content-Length: ' . strlen($json);
            }
        }
        
        curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
        
        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        
        if ($this->logger) {
            $this->logger->info("API-Anfrage: $method $url, Status: $httpCode");
        }
        
        if (curl_errno($ch)) {
            $error = curl_error($ch);
            if ($this->logger) {
                $this->logger->error("API-Fehler: $error");
            }
            curl_close($ch);
            return ['error' => $error];
        }
        
        curl_close($ch);
        
        if ($httpCode >= 200 && $httpCode < 300) {
            return json_decode($response, true) ?: [];
        } else {
            $error = "HTTP-Fehler: $httpCode";
            if ($this->logger) {
                $this->logger->error($error);
            }
            return ['error' => $error, 'response' => $response];
        }
    }

    /**
     * API-Verbindung testen
     * 
     * @return bool Verbindung funktioniert?
     */
    public function testConnection() {
        $response = $this->request('/health');
        return isset($response['status']) && $response['status'] == 'ok';
    }
}