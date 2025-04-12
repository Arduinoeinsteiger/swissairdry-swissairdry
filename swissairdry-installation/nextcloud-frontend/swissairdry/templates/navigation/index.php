<?php
/**
 * SwissAirDry - Navigationsmenü Template
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
?>

<ul id="swissairdry-navigation">
    <li class="nav-item" data-id="dashboard">
        <a href="#dashboard">
            <img src="<?php print_unescaped(image_path('swissairdry', 'dashboard.svg')); ?>" alt="">
            <?php p($l->t('Dashboard')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="auftraege">
        <a href="#auftraege">
            <img src="<?php print_unescaped(image_path('swissairdry', 'jobs.svg')); ?>" alt="">
            <?php p($l->t('Aufträge')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="geraete">
        <a href="#geraete">
            <img src="<?php print_unescaped(image_path('swissairdry', 'devices.svg')); ?>" alt="">
            <?php p($l->t('Geräte')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="messungen">
        <a href="#messungen">
            <img src="<?php print_unescaped(image_path('swissairdry', 'measurements.svg')); ?>" alt="">
            <?php p($l->t('Messungen')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="kunden">
        <a href="#kunden">
            <img src="<?php print_unescaped(image_path('swissairdry', 'customers.svg')); ?>" alt="">
            <?php p($l->t('Kunden')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="berichte">
        <a href="#berichte">
            <img src="<?php print_unescaped(image_path('swissairdry', 'reports.svg')); ?>" alt="">
            <?php p($l->t('Berichte')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="logistik">
        <a href="#logistik">
            <img src="<?php print_unescaped(image_path('swissairdry', 'logistics.svg')); ?>" alt="">
            <?php p($l->t('Logistik')); ?>
        </a>
    </li>
    <li class="nav-item" data-id="administration">
        <a href="#administration">
            <img src="<?php print_unescaped(image_path('swissairdry', 'admin.svg')); ?>" alt="">
            <?php p($l->t('Administration')); ?>
        </a>
    </li>
</ul>

<!-- Navigations-Utilities -->
<div id="app-settings">
    <div id="app-settings-header">
        <button class="settings-button" data-apps-slide-toggle="#app-settings-content">
            <?php p($l->t('Einstellungen')); ?>
        </button>
    </div>
    <div id="app-settings-content">
        <ul>
            <li>
                <a href="#" id="toggle-dark-mode">
                    <span id="dark-mode-text"><?php p($l->t('Dark Mode aktivieren')); ?></span>
                </a>
            </li>
            <li>
                <a href="#help">
                    <?php p($l->t('Hilfe')); ?>
                </a>
            </li>
            <li>
                <a href="<?php p(\OC::$server->getURLGenerator()->linkToRoute('settings.PersonalSettings.index', ['section' => 'swissairdry'])); ?>">
                    <?php p($l->t('Einstellungen')); ?>
                </a>
            </li>
        </ul>
    </div>
</div>