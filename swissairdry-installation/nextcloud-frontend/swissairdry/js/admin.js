/**
 * SwissAirDry - Admin JS
 *
 * JavaScript für die Admin-Einstellungen der SwissAirDry Nextcloud-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

(function(OC, window, $, undefined) {
    'use strict';

    $(document).ready(function() {
        // Variablen für DOM-Elemente
        var $form = $('#swissairdry-admin-form');
        var $testConnectionBtn = $('#swissairdry-test-connection');
        var $testResult = $('#connection-test-result');
        var $saveMessage = $('#save-message');
        var $apiUrl = $('#swissairdry-api-url');
        var $apiKey = $('#swissairdry-api-key');

        /**
         * API-Verbindung testen
         */
        $testConnectionBtn.click(function(e) {
            e.preventDefault();
            
            var url = $apiUrl.val();
            if (!url) {
                showTestResult(false, t('swissairdry', 'Bitte geben Sie eine API-URL ein'));
                return;
            }
            
            // Ladesymbol anzeigen
            $testResult.removeClass('success error').text(t('swissairdry', 'Teste Verbindung...'));
            
            $.ajax({
                url: OC.generateUrl('/apps/swissairdry/settings/test-connection'),
                type: 'POST',
                data: {
                    api_url: url,
                    api_key: $apiKey.val()
                },
                success: function(response) {
                    if (response.success) {
                        showTestResult(true, t('swissairdry', 'Verbindung erfolgreich!'));
                    } else {
                        showTestResult(false, response.message || t('swissairdry', 'Verbindungsfehler'));
                    }
                },
                error: function(xhr) {
                    var message = '';
                    try {
                        var response = JSON.parse(xhr.responseText);
                        message = response.message || '';
                    } catch(e) {
                        message = t('swissairdry', 'Verbindungsfehler');
                    }
                    
                    showTestResult(false, message);
                }
            });
        });

        /**
         * Einstellungen speichern
         */
        $form.submit(function(e) {
            e.preventDefault();
            
            var data = $form.serializeArray().reduce(function(obj, item) {
                if (item.name === 'auto_sync_files') {
                    obj[item.name] = true;
                } else {
                    obj[item.name] = item.value;
                }
                return obj;
            }, {});
            
            // Prüfen, ob auto_sync_files nicht im serialisierten Formular enthalten ist
            // (passiert, wenn die Checkbox nicht aktiviert ist)
            if (!data.hasOwnProperty('auto_sync_files')) {
                data.auto_sync_files = false;
            }
            
            // Ladesymbol anzeigen
            $saveMessage.removeClass('success error visible')
                .addClass('visible')
                .text(t('swissairdry', 'Speichere...'));
            
            $.ajax({
                url: OC.generateUrl('/apps/swissairdry/settings'),
                method: 'POST',
                data: data,
                success: function(response) {
                    showSaveMessage(true, t('swissairdry', 'Einstellungen wurden gespeichert!'));
                    
                    // Wenn das API-Key Feld leer ist, zeigt es an, dass der bestehende Schlüssel beibehalten wurde
                    // Setzen wir es daher auf einen Platzhalter zurück
                    if (!$apiKey.val()) {
                        $apiKey.val('');
                    }
                },
                error: function() {
                    showSaveMessage(false, t('swissairdry', 'Fehler beim Speichern der Einstellungen'));
                }
            });
        });

        /**
         * Zeigt das Ergebnis des Verbindungstests an
         */
        function showTestResult(success, message) {
            $testResult.removeClass('success error');
            if (success) {
                $testResult.addClass('success');
            } else {
                $testResult.addClass('error');
            }
            $testResult.text(message);
        }

        /**
         * Zeigt eine Speichernachricht an
         */
        function showSaveMessage(success, message) {
            $saveMessage.removeClass('success error').addClass('visible');
            if (success) {
                $saveMessage.addClass('success');
            } else {
                $saveMessage.addClass('error');
            }
            $saveMessage.text(message);
            
            // Ausblenden nach einigen Sekunden
            setTimeout(function() {
                $saveMessage.removeClass('visible');
            }, 5000);
        }

        /**
         * Hilfsfunktion für Übersetzungen
         */
        function t(app, string) {
            return OC.L10N.translate(app, string);
        }
    });

})(OC, window, jQuery);