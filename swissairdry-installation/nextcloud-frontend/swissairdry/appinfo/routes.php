<?php
/**
 * SwissAirDry - Routes-Datei
 *
 * Definiert die Routen für die SwissAirDry Nextcloud-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

return [
    'routes' => [
        // Hauptseite
        ['name' => 'page#index', 'url' => '/', 'verb' => 'GET'],
        
        // API-Weiterleitungen
        ['name' => 'api#proxy', 'url' => '/api/{path}', 'verb' => 'GET', 'requirements' => ['path' => '.+']],
        ['name' => 'api#proxyPost', 'url' => '/api/{path}', 'verb' => 'POST', 'requirements' => ['path' => '.+']],
        ['name' => 'api#proxyPut', 'url' => '/api/{path}', 'verb' => 'PUT', 'requirements' => ['path' => '.+']],
        ['name' => 'api#proxyDelete', 'url' => '/api/{path}', 'verb' => 'DELETE', 'requirements' => ['path' => '.+']],
        
        // Einstellungen
        ['name' => 'settings#getApiUrl', 'url' => '/settings/api-url', 'verb' => 'GET'],
        ['name' => 'settings#setApiUrl', 'url' => '/settings/api-url', 'verb' => 'POST'],
        
        // Dateiintegrationen
        ['name' => 'files#saveToNextcloud', 'url' => '/files/save', 'verb' => 'POST'],
        ['name' => 'files#getFileLink', 'url' => '/files/link/{id}', 'verb' => 'GET'],
        
        // Dashboard-Module
        ['name' => 'dashboard#getModules', 'url' => '/dashboard/modules', 'verb' => 'GET'],
        
        // Benutzermodule
        ['name' => 'users#getCurrentUser', 'url' => '/users/current', 'verb' => 'GET'],
        ['name' => 'users#getPermissions', 'url' => '/users/permissions', 'verb' => 'GET'],
    ],
    'resources' => [
        'settings' => ['url' => '/settings'],
    ],
];