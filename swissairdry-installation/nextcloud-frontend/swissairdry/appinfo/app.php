<?php
/**
 * SwissAirDry - App-Initialisierung
 *
 * Registriert die SwissAirDry-App bei Nextcloud.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

namespace OCA\SwissAirDry\AppInfo;

use OCP\AppFramework\App;
use OCP\INavigationManager;
use OCP\IURLGenerator;
use OCP\Util;

$app = new App('swissairdry');
$container = $app->getContainer();

$container->query(INavigationManager::class)->add(function () use ($container) {
    $urlGenerator = $container->query(IURLGenerator::class);
    return [
        'id' => 'swissairdry',
        'order' => 77,
        'href' => $urlGenerator->linkToRoute('swissairdry.page.index'),
        'icon' => $urlGenerator->imagePath('swissairdry', 'app-dark.svg'),
        'name' => $container->query(\OCP\IL10N::class)->t('SwissAirDry'),
    ];
});

// JavaScript für die Hauptansicht laden
Util::addScript('swissairdry', 'swissairdry-main');

// CSS-Stile laden
Util::addStyle('swissairdry', 'style');