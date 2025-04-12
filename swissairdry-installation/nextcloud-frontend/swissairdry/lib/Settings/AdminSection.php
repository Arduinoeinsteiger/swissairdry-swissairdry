<?php
/**
 * SwissAirDry - Admin Settings Section
 *
 * Implementiert die Administrationseinstellungssektion für die SwissAirDry-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

namespace OCA\SwissAirDry\Settings;

use OCP\IL10N;
use OCP\IURLGenerator;
use OCP\Settings\IIconSection;

class AdminSection implements IIconSection {
    /** @var IL10N */
    private $l;
    
    /** @var IURLGenerator */
    private $urlGenerator;

    /**
     * @param IL10N $l
     * @param IURLGenerator $urlGenerator
     */
    public function __construct(IL10N $l, IURLGenerator $urlGenerator) {
        $this->l = $l;
        $this->urlGenerator = $urlGenerator;
    }

    /**
     * Hinweis: Dieser Wert muss mit der ID in Admin::getSection() übereinstimmen
     *
     * @return string
     */
    public function getID() {
        return 'swissairdry';
    }

    /**
     * @return string
     */
    public function getName() {
        return $this->l->t('SwissAirDry');
    }

    /**
     * @return int
     */
    public function getPriority() {
        return 80;
    }

    /**
     * @return string
     */
    public function getIcon() {
        return $this->urlGenerator->imagePath('swissairdry', 'app-dark.svg');
    }
}