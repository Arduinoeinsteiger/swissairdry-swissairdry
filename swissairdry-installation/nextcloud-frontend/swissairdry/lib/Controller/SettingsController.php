<?php
/**
 * SwissAirDry - Settings Controller
 *
 * Steuert die Verwaltung von Einstellungen für die SwissAirDry-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

namespace OCA\SwissAirDry\Controller;

use OCP\AppFramework\Controller;
use OCP\AppFramework\Http;
use OCP\AppFramework\Http\JSONResponse;
use OCP\IRequest;
use OCP\IConfig;
use OCP\IL10N;
use OCP\Http\Client\IClientService;

class SettingsController extends Controller {
    /** @var IConfig */
    private $config;
    
    /** @var IL10N */
    private $l;
    
    /** @var IClientService */
    private $clientService;

    /**
     * @param string $appName
     * @param IRequest $request
     * @param IConfig $config
     * @param IL10N $l
     * @param IClientService $clientService
     */
    public function __construct(
        $appName,
        IRequest $request,
        IConfig $config,
        IL10N $l,
        IClientService $clientService
    ) {
        parent::__construct($appName, $request);
        $this->config = $config;
        $this->l = $l;
        $this->clientService = $clientService;
    }

    /**
     * @NoAdminRequired
     */
    public function getApiUrl() {
        return new JSONResponse([
            'api_url' => $this->config->getAppValue('swissairdry', 'api_url', '')
        ]);
    }

    /**
     * @param string $api_url
     * @return JSONResponse
     */
    public function setApiUrl($api_url) {
        $this->config->setAppValue('swissairdry', 'api_url', $api_url);
        
        return new JSONResponse([
            'success' => true,
            'api_url' => $api_url
        ]);
    }

    /**
     * API-Verbindung testen
     *
     * @NoAdminRequired
     * @param string $api_url
     * @param string $api_key
     * @return JSONResponse
     */
    public function testConnection($api_url, $api_key = '') {
        if (empty($api_url)) {
            return new JSONResponse([
                'success' => false,
                'message' => $this->l->t('API-URL darf nicht leer sein')
            ], Http::STATUS_BAD_REQUEST);
        }
        
        try {
            $client = $this->clientService->newClient();
            $headers = [];
            
            if (!empty($api_key)) {
                $headers['Authorization'] = 'Bearer ' . $api_key;
            }
            
            $options = [
                'headers' => $headers,
                'timeout' => 10
            ];
            
            // Health-Check-Endpunkt aufrufen
            $url = rtrim($api_url, '/') . '/health';
            $response = $client->get($url, $options);
            
            $body = json_decode($response->getBody(), true);
            
            if ($response->getStatusCode() === 200 && isset($body['status']) && $body['status'] === 'ok') {
                return new JSONResponse([
                    'success' => true,
                    'message' => $this->l->t('Verbindung zum API-Server erfolgreich hergestellt'),
                    'api_version' => $body['version'] ?? $this->l->t('Unbekannt')
                ]);
            } else {
                return new JSONResponse([
                    'success' => false,
                    'message' => $this->l->t('Unerwartete Antwort vom API-Server: %s', [$body['message'] ?? 'Unbekannter Fehler'])
                ], Http::STATUS_BAD_REQUEST);
            }
        } catch (\Exception $e) {
            return new JSONResponse([
                'success' => false,
                'message' => $this->l->t('Verbindungsfehler: %s', [$e->getMessage()])
            ], Http::STATUS_BAD_REQUEST);
        }
    }

    /**
     * Einstellungen speichern
     *
     * @param string $api_url
     * @param string $api_key
     * @param int $connection_timeout
     * @param bool $auto_sync_files
     * @param string $default_file_path
     * @return JSONResponse
     */
    public function index($api_url, $api_key = null, $connection_timeout = 30, $auto_sync_files = true, $default_file_path = '/SwissAirDry') {
        if (empty($api_url)) {
            return new JSONResponse([
                'success' => false,
                'message' => $this->l->t('API-URL darf nicht leer sein')
            ], Http::STATUS_BAD_REQUEST);
        }
        
        // API-URL speichern
        $this->config->setAppValue('swissairdry', 'api_url', $api_url);
        
        // API-Key nur speichern, wenn er angegeben wurde (nicht null)
        if ($api_key !== null && $api_key !== '') {
            $this->config->setAppValue('swissairdry', 'api_key', $api_key);
        }
        
        // Verbindungs-Timeout speichern (min 5, max 120)
        $timeout = max(5, min(120, intval($connection_timeout)));
        $this->config->setAppValue('swissairdry', 'connection_timeout', $timeout);
        
        // Datei-Synchronisationsoptionen speichern
        $this->config->setAppValue('swissairdry', 'auto_sync_files', $auto_sync_files ? 'true' : 'false');
        $this->config->setAppValue('swissairdry', 'default_file_path', $default_file_path);
        
        return new JSONResponse([
            'success' => true,
            'message' => $this->l->t('Einstellungen wurden gespeichert')
        ]);
    }
}