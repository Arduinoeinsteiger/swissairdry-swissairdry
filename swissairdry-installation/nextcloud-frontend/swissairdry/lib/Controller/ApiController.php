<?php
/**
 * SwissAirDry - API Controller
 *
 * Steuert die API-Weiterleitungen zwischen Nextcloud und dem SwissAirDry API-Server.
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
use OCP\IUserSession;
use OCP\Http\Client\IClientService;

class ApiController extends Controller {
    /** @var IConfig */
    private $config;
    
    /** @var IUserSession */
    private $userSession;
    
    /** @var IClientService */
    private $clientService;
    
    /** @var string */
    private $userId;

    /**
     * @param string $appName
     * @param IRequest $request
     * @param IConfig $config
     * @param IUserSession $userSession
     * @param IClientService $clientService
     * @param string $userId
     */
    public function __construct(
        $appName,
        IRequest $request,
        IConfig $config,
        IUserSession $userSession,
        IClientService $clientService,
        $userId
    ) {
        parent::__construct($appName, $request);
        $this->config = $config;
        $this->userSession = $userSession;
        $this->clientService = $clientService;
        $this->userId = $userId;
    }

    /**
     * Hilfsmethode, um die API-URL zu erhalten
     *
     * @return string|null
     */
    private function getApiUrl() {
        return $this->config->getAppValue('swissairdry', 'api_url', null);
    }

    /**
     * Hilfsmethode, um die Authentifizierungsheader für API-Anfragen zu erstellen
     *
     * @return array
     */
    private function getAuthHeaders() {
        $headers = [
            'X-Nextcloud-User' => $this->userId,
            'Content-Type' => 'application/json',
        ];
        
        // Optional: JWT-Token oder ähnliches hinzufügen
        $apiKey = $this->config->getAppValue('swissairdry', 'api_key', '');
        if (!empty($apiKey)) {
            $headers['Authorization'] = 'Bearer ' . $apiKey;
        }
        
        return $headers;
    }

    /**
     * @NoAdminRequired
     * @NoCSRFRequired
     *
     * @param string $path
     * @return JSONResponse
     */
    public function proxy($path) {
        $apiUrl = $this->getApiUrl();
        if (!$apiUrl) {
            return new JSONResponse(['error' => 'API-URL not configured'], Http::STATUS_INTERNAL_SERVER_ERROR);
        }
        
        try {
            $client = $this->clientService->newClient();
            $url = rtrim($apiUrl, '/') . '/' . ltrim($path, '/');
            
            $options = [
                'headers' => $this->getAuthHeaders(),
                'timeout' => 30,
            ];
            
            $response = $client->get($url, $options);
            $body = $response->getBody();
            $statusCode = $response->getStatusCode();
            
            return new JSONResponse(json_decode($body, true), $statusCode);
        } catch (\Exception $e) {
            return new JSONResponse(
                ['error' => $e->getMessage()],
                Http::STATUS_INTERNAL_SERVER_ERROR
            );
        }
    }

    /**
     * @NoAdminRequired
     * @NoCSRFRequired
     *
     * @param string $path
     * @return JSONResponse
     */
    public function proxyPost($path) {
        $apiUrl = $this->getApiUrl();
        if (!$apiUrl) {
            return new JSONResponse(['error' => 'API-URL not configured'], Http::STATUS_INTERNAL_SERVER_ERROR);
        }
        
        try {
            $client = $this->clientService->newClient();
            $url = rtrim($apiUrl, '/') . '/' . ltrim($path, '/');
            
            $body = file_get_contents('php://input');
            $options = [
                'headers' => $this->getAuthHeaders(),
                'body' => $body,
                'timeout' => 30,
            ];
            
            $response = $client->post($url, $options);
            $responseBody = $response->getBody();
            $statusCode = $response->getStatusCode();
            
            return new JSONResponse(json_decode($responseBody, true), $statusCode);
        } catch (\Exception $e) {
            return new JSONResponse(
                ['error' => $e->getMessage()],
                Http::STATUS_INTERNAL_SERVER_ERROR
            );
        }
    }

    /**
     * @NoAdminRequired
     * @NoCSRFRequired
     *
     * @param string $path
     * @return JSONResponse
     */
    public function proxyPut($path) {
        $apiUrl = $this->getApiUrl();
        if (!$apiUrl) {
            return new JSONResponse(['error' => 'API-URL not configured'], Http::STATUS_INTERNAL_SERVER_ERROR);
        }
        
        try {
            $client = $this->clientService->newClient();
            $url = rtrim($apiUrl, '/') . '/' . ltrim($path, '/');
            
            $body = file_get_contents('php://input');
            $options = [
                'headers' => $this->getAuthHeaders(),
                'body' => $body,
                'timeout' => 30,
            ];
            
            $response = $client->put($url, $options);
            $responseBody = $response->getBody();
            $statusCode = $response->getStatusCode();
            
            return new JSONResponse(json_decode($responseBody, true), $statusCode);
        } catch (\Exception $e) {
            return new JSONResponse(
                ['error' => $e->getMessage()],
                Http::STATUS_INTERNAL_SERVER_ERROR
            );
        }
    }

    /**
     * @NoAdminRequired
     * @NoCSRFRequired
     *
     * @param string $path
     * @return JSONResponse
     */
    public function proxyDelete($path) {
        $apiUrl = $this->getApiUrl();
        if (!$apiUrl) {
            return new JSONResponse(['error' => 'API-URL not configured'], Http::STATUS_INTERNAL_SERVER_ERROR);
        }
        
        try {
            $client = $this->clientService->newClient();
            $url = rtrim($apiUrl, '/') . '/' . ltrim($path, '/');
            
            $options = [
                'headers' => $this->getAuthHeaders(),
                'timeout' => 30,
            ];
            
            $response = $client->delete($url, $options);
            $responseBody = $response->getBody();
            $statusCode = $response->getStatusCode();
            
            return new JSONResponse(json_decode($responseBody, true), $statusCode);
        } catch (\Exception $e) {
            return new JSONResponse(
                ['error' => $e->getMessage()],
                Http::STATUS_INTERNAL_SERVER_ERROR
            );
        }
    }
}