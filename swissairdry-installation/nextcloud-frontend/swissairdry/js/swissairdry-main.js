/**
 * SwissAirDry - Hauptdatei für JavaScript
 *
 * JavaScript für die Hauptansicht der SwissAirDry Nextcloud-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

(function(OC, window, $, undefined) {
    'use strict';

    $(document).ready(function() {
        // App Container
        var $appContainer = $('#swissairdry-app');
        
        // Konfiguration aus Datenattributen lesen
        var config = {
            apiUrl: $appContainer.data('api-url'),
            nextcloudUrl: $appContainer.data('nextcloud-url'),
            userId: $appContainer.data('user-id'),
            lang: $appContainer.data('lang')
        };
        
        // Navigationselement
        var $navigation = $('#swissairdry-navigation');
        var $navItems = $navigation.find('.nav-item');
        
        // Dunkelmodus-Toggle
        var $darkModeToggle = $('#toggle-dark-mode');
        var $darkModeText = $('#dark-mode-text');
        
        // Prüfen, ob die App bereits initialisiert wurde
        if ($appContainer.data('initialized')) {
            return;
        }
        
        // App als initialisiert markieren
        $appContainer.data('initialized', true);
        
        /**
         * Navigation initialisieren
         */
        function initNavigation() {
            // Aktiven Menüpunkt basierend auf Hash setzen
            setActiveNavItem();
            
            // Event-Listener für Hash-Änderungen
            $(window).on('hashchange', function() {
                setActiveNavItem();
                loadContent();
            });
            
            // Klick-Handler für Menüpunkte
            $navItems.on('click', function(e) {
                var $item = $(this);
                var navId = $item.data('id');
                
                // Hash setzen
                window.location.hash = navId;
                
                // Inhalt laden
                loadContent();
            });
        }
        
        /**
         * Aktiven Navigationspunkt basierend auf URL-Hash setzen
         */
        function setActiveNavItem() {
            var hash = window.location.hash.substr(1) || 'dashboard';
            
            // Alle aktiven Klassen entfernen
            $navItems.find('a').removeClass('active');
            
            // Aktive Klasse für aktuellen Menüpunkt setzen
            $navItems.filter('[data-id="' + hash + '"]').find('a').addClass('active');
        }
        
        /**
         * Inhalt basierend auf aktuellem Hash laden
         */
        function loadContent() {
            // Loading anzeigen
            showLoading();
            
            var hash = window.location.hash.substr(1) || 'dashboard';
            var moduleUrl = 'api/dashboard/modules/' + hash;
            
            $.ajax({
                url: OC.generateUrl('/apps/swissairdry/' + moduleUrl),
                type: 'GET',
                contentType: 'application/json',
                success: function(response) {
                    renderContent(response);
                    hideLoading();
                },
                error: function(xhr, status, error) {
                    console.error('Fehler beim Laden des Moduls:', error);
                    showError(t('swissairdry', 'Fehler beim Laden des Moduls'));
                    hideLoading();
                }
            });
        }
        
        /**
         * Inhalts-Container mit empfangenen Daten rendern
         */
        function renderContent(data) {
            var $contentWrapper = $('#app-content-wrapper');
            
            // Bestehenden Inhalt leeren (außer Loading-Screen)
            $contentWrapper.find(':not(#loading-screen)').remove();
            
            // HTML-Container für Inhalt erstellen
            var $content = $('<div>').attr('id', 'module-content').html(data.html || '');
            
            // Module an Container anhängen
            $contentWrapper.append($content);
            
            // Event-Handling für interaktive Elemente einrichten
            setupInteractiveElements();
        }
        
        /**
         * Loading-Anzeige einblenden
         */
        function showLoading() {
            $('#loading-screen').show();
        }
        
        /**
         * Loading-Anzeige ausblenden
         */
        function hideLoading() {
            $('#loading-screen').hide();
        }
        
        /**
         * Fehlermeldung anzeigen
         */
        function showError(message) {
            var $errorContainer = $('<div>').addClass('error-container')
                .append($('<div>').addClass('icon-error'))
                .append($('<p>').text(message));
            
            $('#app-content-wrapper').append($errorContainer);
        }
        
        /**
         * Event-Handling für interaktive Elemente einrichten
         */
        function setupInteractiveElements() {
            // Diese Funktion wird aufgerufen, nachdem neuer Inhalt geladen wurde
            // Hier können Event-Handler für die dynamisch erzeugten Elemente gesetzt werden
        }
        
        /**
         * Dark Mode initialisieren
         */
        function initDarkMode() {
            // Prüfen, ob Dark Mode bereits aktiviert ist
            var darkModeEnabled = localStorage.getItem('swissairdry_dark_mode') === 'true';
            
            // Initialen Zustand setzen
            setDarkMode(darkModeEnabled);
            
            // Toggle-Button Event Handler
            $darkModeToggle.on('click', function(e) {
                e.preventDefault();
                toggleDarkMode();
            });
        }
        
        /**
         * Dark Mode ein- oder ausschalten
         */
        function setDarkMode(enabled) {
            if (enabled) {
                $('body').addClass('theme--dark');
                $darkModeText.text(t('swissairdry', 'Dark Mode deaktivieren'));
            } else {
                $('body').removeClass('theme--dark');
                $darkModeText.text(t('swissairdry', 'Dark Mode aktivieren'));
            }
            
            // Einstellung speichern
            localStorage.setItem('swissairdry_dark_mode', enabled);
        }
        
        /**
         * Dark Mode umschalten
         */
        function toggleDarkMode() {
            var currentMode = localStorage.getItem('swissairdry_dark_mode') === 'true';
            setDarkMode(!currentMode);
        }
        
        /**
         * Hilfsfunktion für Übersetzungen
         */
        function t(app, string) {
            return OC.L10N.translate(app, string);
        }
        
        // Initialisierung
        initNavigation();
        initDarkMode();
        
        // Initiale Inhaltsladung
        loadContent();
    });

})(OC, window, jQuery);