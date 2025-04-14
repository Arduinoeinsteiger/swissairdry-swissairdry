<?php
namespace SwissAirDry;

/**
 * Logger für die SwissAirDry-App
 * 
 * Implementiert die PSR-3 LoggerInterface.
 */
class Logger implements \Psr\Log\LoggerInterface {
    private $logFile;
    private $logLevel;
    
    const EMERGENCY = 'emergency';
    const ALERT     = 'alert';
    const CRITICAL  = 'critical';
    const ERROR     = 'error';
    const WARNING   = 'warning';
    const NOTICE    = 'notice';
    const INFO      = 'info';
    const DEBUG     = 'debug';
    
    /**
     * Konstruktor
     * 
     * @param string $logFile Pfad zur Log-Datei
     * @param string $logLevel Minimales Log-Level
     */
    public function __construct($logFile = null, $logLevel = self::INFO) {
        $this->logFile = $logFile ?: __DIR__ . '/../logs/swissairdry.log';
        $this->logLevel = $logLevel;
        
        // Log-Verzeichnis erstellen, falls es nicht existiert
        $logDir = dirname($this->logFile);
        if (!is_dir($logDir)) {
            mkdir($logDir, 0755, true);
        }
    }
    
    /**
     * Log-Meldung schreiben
     * 
     * @param string $level Log-Level
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function log($level, $message, array $context = array()) {
        $levels = [
            self::EMERGENCY => 0,
            self::ALERT     => 1,
            self::CRITICAL  => 2,
            self::ERROR     => 3,
            self::WARNING   => 4,
            self::NOTICE    => 5,
            self::INFO      => 6,
            self::DEBUG     => 7,
        ];
        
        // Prüfen, ob das Level geloggt werden soll
        if ($levels[$level] > $levels[$this->logLevel]) {
            return;
        }
        
        // Kontext-Platzhalter ersetzen
        $replace = [];
        foreach ($context as $key => $val) {
            if (is_string($val) || is_numeric($val) || is_bool($val)) {
                $replace['{' . $key . '}'] = $val;
            } elseif (is_null($val)) {
                $replace['{' . $key . '}'] = 'null';
            } elseif (is_array($val) || is_object($val)) {
                $replace['{' . $key . '}'] = json_encode($val);
            }
        }
        
        $message = strtr($message, $replace);
        
        // Log-Eintrag formatieren
        $datetime = new \DateTime();
        $logEntry = sprintf(
            "[%s] %s: %s\n",
            $datetime->format('Y-m-d H:i:s'),
            strtoupper($level),
            $message
        );
        
        // In Datei schreiben
        file_put_contents($this->logFile, $logEntry, FILE_APPEND);
    }
    
    /**
     * System ist unbenutzbar
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function emergency($message, array $context = array()) {
        $this->log(self::EMERGENCY, $message, $context);
    }
    
    /**
     * Sofortige Maßnahmen erforderlich
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function alert($message, array $context = array()) {
        $this->log(self::ALERT, $message, $context);
    }
    
    /**
     * Kritischer Zustand
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function critical($message, array $context = array()) {
        $this->log(self::CRITICAL, $message, $context);
    }
    
    /**
     * Laufzeitfehler
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function error($message, array $context = array()) {
        $this->log(self::ERROR, $message, $context);
    }
    
    /**
     * Warnung
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function warning($message, array $context = array()) {
        $this->log(self::WARNING, $message, $context);
    }
    
    /**
     * Normaler, aber wichtiger Zustand
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function notice($message, array $context = array()) {
        $this->log(self::NOTICE, $message, $context);
    }
    
    /**
     * Informationen
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function info($message, array $context = array()) {
        $this->log(self::INFO, $message, $context);
    }
    
    /**
     * Debug-Meldungen
     * 
     * @param string $message Meldung
     * @param array $context Kontext
     */
    public function debug($message, array $context = array()) {
        $this->log(self::DEBUG, $message, $context);
    }
}