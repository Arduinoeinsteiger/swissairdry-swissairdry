<?php
/**
 * SwissAirDry - Admin Settings
 *
 * Implementiert die Administrationseinstellungen für die SwissAirDry-App.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

namespace OCA\SwissAirDry\Settings;

use OCP\AppFramework\Http\TemplateResponse;
use OCP\Settings\ISettings;
use OCP\IConfig;
use OCP\IL10N;
use OCP\IURLGenerator;

class Admin implements ISettings {
    /** @var IConfig */
    private $config;
    
    /** @var IL10N */
    private $l;
    
    /** @var IURLGenerator */
    private $urlGenerator;

    /**
     * @param IConfig $config
     * @param IL10N $l
     * @param IURLGenerator $urlGenerator
     */
    public function __construct(
        IConfig $config,
        IL10N $l,
        IURLGenerator $urlGenerator
    ) {
        $this->config = $config;
        $this->l = $l;
        $this->urlGenerator = $urlGenerator;
    }

    /**
     * @return TemplateResponse
     */
    public function getForm() {
        $parameters = [
            'api_url' => $this->config->getAppValue('swissairdry', 'api_url', ''),
            'api_key' => $this->config->getAppValue('swissairdry', 'api_key', ''),
            'connection_timeout' => intval($this->config->getAppValue('swissairdry', 'connection_timeout', '30')),
            'auto_sync_files' => boolval($this->config->getAppValue('swissairdry', 'auto_sync_files', 'true')),
            'default_file_path' => $this->config->getAppValue('swissairdry', 'default_file_path', '/SwissAirDry'),
        ];
        
        return new TemplateResponse('swissairdry', 'admin', $parameters, 'blank');
    }

    /**
     * @return string the section ID, e.g. 'sharing'
     */
    public function getSection() {
        return 'swissairdry';
    }

    /**
     * @return int whether the form should be rather on the top or bottom of
     * the admin section. The forms are arranged in ascending order of the
     * priority values. It is required to return a value between 0 and 100.
     */
    public function getPriority() {
        return 50;
    }
}