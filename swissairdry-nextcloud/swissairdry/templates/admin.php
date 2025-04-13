<?php
/**
 * SwissAirDry - Admin Template
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

script('swissairdry', 'admin');
style('swissairdry', 'admin');
?>

<div id="swissairdry-admin" class="section">
    <h2><?php p($l->t('SwissAirDry Einstellungen')); ?></h2>
    
    <form id="swissairdry-admin-form">
        <div class="admin-settings-block">
            <h3><?php p($l->t('API-Verbindung')); ?></h3>
            <p class="settings-hint">
                <?php p($l->t('Konfigurieren Sie hier die Verbindung zum SwissAirDry API-Server.')); ?>
            </p>
            
            <div class="form-group">
                <label for="swissairdry-api-url"><?php p($l->t('API-URL')); ?></label>
                <input type="text" id="swissairdry-api-url" name="api_url"
                       value="<?php p($_['api_url']); ?>"
                       placeholder="https://swissairdry-api.example.com"
                       required />
                <p class="hint">
                    <?php p($l->t('Die vollständige URL zum SwissAirDry API-Server, z.B. https://swissairdry-api.example.com')); ?>
                </p>
            </div>
            
            <div class="form-group">
                <label for="swissairdry-api-key"><?php p($l->t('API-Schlüssel')); ?></label>
                <input type="password" id="swissairdry-api-key" name="api_key"
                       value="<?php p($_['api_key']); ?>"
                       placeholder="<?php p($l->t('API-Schlüssel eingeben')); ?>" />
                <p class="hint">
                    <?php p($l->t('Der Authentifizierungsschlüssel für den API-Zugriff. Lassen Sie dieses Feld leer, um den aktuellen Schlüssel beizubehalten.')); ?>
                </p>
            </div>
            
            <div class="form-group">
                <label for="swissairdry-connection-timeout"><?php p($l->t('Verbindungs-Timeout')); ?></label>
                <input type="number" id="swissairdry-connection-timeout" name="connection_timeout"
                       value="<?php p($_['connection_timeout']); ?>"
                       min="5" max="120" />
                <span class="unit">s</span>
                <p class="hint">
                    <?php p($l->t('Timeout für API-Anfragen in Sekunden (5-120).')); ?>
                </p>
            </div>
            
            <div class="form-group">
                <input type="button" id="swissairdry-test-connection" class="button"
                       value="<?php p($l->t('Verbindung testen')); ?>" />
                <span id="connection-test-result"></span>
            </div>
        </div>
        
        <div class="admin-settings-block">
            <h3><?php p($l->t('Nextcloud-Integration')); ?></h3>
            
            <div class="form-group">
                <input type="checkbox" id="swissairdry-auto-sync-files" name="auto_sync_files"
                       class="checkbox" <?php if ($_['auto_sync_files']) echo 'checked'; ?> />
                <label for="swissairdry-auto-sync-files"><?php p($l->t('Automatische Dateisynchronisation')); ?></label>
                <p class="hint">
                    <?php p($l->t('Berichte und Fotos automatisch mit Nextcloud synchronisieren.')); ?>
                </p>
            </div>
            
            <div class="form-group">
                <label for="swissairdry-default-file-path"><?php p($l->t('Standard-Dateipfad')); ?></label>
                <input type="text" id="swissairdry-default-file-path" name="default_file_path"
                       value="<?php p($_['default_file_path']); ?>"
                       placeholder="/SwissAirDry" />
                <p class="hint">
                    <?php p($l->t('Der Standardpfad in Nextcloud, in dem SwissAirDry-Dateien gespeichert werden.')); ?>
                </p>
            </div>
        </div>
        
        <div class="admin-settings-block">
            <input type="submit" id="swissairdry-admin-submit" 
                   class="button primary" value="<?php p($l->t('Einstellungen speichern')); ?>" />
            <span id="save-message" class="msg"></span>
        </div>
    </form>
</div>