<?php
/**
 * SwissAirDry - Page Controller
 *
 * Steuert die Hauptseiten der SwissAirDry-App in Nextcloud.
 *
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */

namespace OCA\SwissAirDry\Controller;

use OCP\AppFramework\Controller;
use OCP\AppFramework\Http\TemplateResponse;
use OCP\IRequest;
use OCP\IConfig;
use OCP\IURLGenerator;
use OCP\IL10N;
use OCP\IUserSession;

class PageController extends Controller {
    /** @var IConfig */
    private $config;
    
    /** @var IL10N */
    private $l;
    
    /** @var IURLGenerator */
    private $urlGenerator;
    
    /** @var IUserSession */
    private $userSession;
    
    /** @var string */
    private $userId;

    /**
     * @param string $appName
     * @param IRequest $request
     * @param IConfig $config
     * @param IL10N $l
     * @param IURLGenerator $urlGenerator
     * @param IUserSession $userSession
     * @param string $userId
     */
    public function __construct(
        $appName,
        IRequest $request,
        IConfig $config,
        IL10N $l,
        IURLGenerator $urlGenerator,
        IUserSession $userSession,
        $userId
    ) {
        parent::__construct($appName, $request);
        $this->config = $config;
        $this->l = $l;
        $this->urlGenerator = $urlGenerator;
        $this->userSession = $userSession;
        $this->userId = $userId;
    }

    /**
     * CAUTION: the @Stuff annotations here are used for different
     * annotations depending on your app's setup
     *
     * @NoAdminRequired
     * @NoCSRFRequired
     */
    public function index() {
        $apiUrl = $this->config->getAppValue('swissairdry', 'api_url', '');
        
        $params = [
            'user_id' => $this->userId,
            'api_url' => $apiUrl,
            'nextcloud_url' => $this->urlGenerator->getBaseUrl(),
            'lang' => $this->l->getLanguageCode(),
        ];
        
        // Prüfen, ob API-URL konfiguriert ist
        if (empty($apiUrl) && $this->userSession->getUser()->isAdmin()) {
            // Wenn nicht konfiguriert und Admin, zur Einstellungsseite weiterleiten
            $params['setup_required'] = true;
            $params['admin_settings_url'] = $this->urlGenerator->linkToRoute('settings.PersonalSettings.index', ['section' => 'swissairdry']);
        }
        
        return new TemplateResponse('swissairdry', 'index', $params);
    }
}