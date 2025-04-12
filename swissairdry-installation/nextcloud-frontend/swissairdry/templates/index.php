<?php
/**
 * SwissAirDry - Hauptansicht Template
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

script('swissairdry', 'swissairdry-main');
style('swissairdry', 'style');
?>

<div id="content" class="app-swissairdry">
    <div id="app-navigation">
        <?php print_unescaped($this->inc('navigation/index')); ?>
    </div>

    <div id="app-content">
        <div id="loading-screen" class="icon-loading"></div>
        <div id="app-content-wrapper">
            <?php if (isset($_['setup_required']) && $_['setup_required']): ?>
                <div class="setup-required-message">
                    <h2><?php p($l->t('Einrichtung erforderlich')); ?></h2>
                    <p>
                        <?php p($l->t('Bitte konfigurieren Sie zuerst die API-Verbindung in den Einstellungen.')); ?>
                    </p>
                    <a href="<?php p($_['admin_settings_url']); ?>" class="button primary">
                        <?php p($l->t('Zu den Einstellungen')); ?>
                    </a>
                </div>
            <?php else: ?>
                <!-- SwissAirDry App Container -->
                <div id="swissairdry-app" 
                     data-api-url="<?php p($_['api_url']); ?>" 
                     data-nextcloud-url="<?php p($_['nextcloud_url']); ?>" 
                     data-user-id="<?php p($_['user_id']); ?>"
                     data-lang="<?php p($_['lang']); ?>">
                    <div class="initial-loading">
                        <h2><?php p($l->t('SwissAirDry wird geladen...')); ?></h2>
                        <div class="icon-loading"></div>
                    </div>
                </div>
            <?php endif; ?>
        </div>
    </div>
</div>