<?php
namespace SwissAirDry;

/**
 * MQTT-Service für die SwissAirDry-App
 * 
 * Stellt Funktionen für die Kommunikation mit dem MQTT-Broker bereit.
 */
class MQTTService {
    private $mqtt;
    private $logger;
    private $connected = false;

    /**
     * Konstruktor
     * 
     * @param \Psr\Log\LoggerInterface $logger Logger-Instanz
     */
    public function __construct(\Psr\Log\LoggerInterface $logger = null) {
        $this->logger = $logger;
        
        try {
            $this->mqtt = new \Mosquitto\Client('swissairdry-' . uniqid());
            
            $broker = getenv('MQTT_BROKER') ?: 'localhost';
            $port = (int)(getenv('MQTT_PORT') ?: 1883);
            $username = getenv('MQTT_USERNAME') ?: null;
            $password = getenv('MQTT_PASSWORD') ?: null;
            
            if ($username && $password) {
                $this->mqtt->setCredentials($username, $password);
            }
            
            $this->mqtt->connect($broker, $port, 60);
            $this->connected = true;
            
            if ($this->logger) {
                $this->logger->info("MQTT-Verbindung hergestellt: $broker:$port");
            }
        } catch (\Exception $e) {
            $this->connected = false;
            if ($this->logger) {
                $this->logger->error("MQTT-Verbindungsfehler: " . $e->getMessage());
            }
        }
    }

    /**
     * Nachricht an ein Topic senden
     * 
     * @param string $topic Topic, an das gesendet werden soll
     * @param string $message Nachricht, die gesendet werden soll
     * @param int $qos Quality of Service (0, 1 oder 2)
     * @param bool $retain Soll die Nachricht gespeichert werden?
     * @return bool Erfolgreich gesendet?
     */
    public function publish($topic, $message, $qos = 1, $retain = false) {
        if (!$this->connected) {
            return false;
        }
        
        try {
            $this->mqtt->publish($topic, $message, $qos, $retain);
            return true;
        } catch (\Exception $e) {
            if ($this->logger) {
                $this->logger->error("MQTT-Fehler beim Senden: " . $e->getMessage());
            }
            return false;
        }
    }

    /**
     * Topic abonnieren
     * 
     * @param string $topic Topic, das abonniert werden soll
     * @param callable $callback Callback-Funktion, die bei Nachrichten aufgerufen wird
     * @param int $qos Quality of Service (0, 1 oder 2)
     * @return bool Erfolgreich abonniert?
     */
    public function subscribe($topic, $callback, $qos = 1) {
        if (!$this->connected) {
            return false;
        }
        
        try {
            $this->mqtt->subscribe($topic, $qos);
            $this->mqtt->onMessage(function($message) use ($callback) {
                $callback($message->topic, $message->payload);
            });
            return true;
        } catch (\Exception $e) {
            if ($this->logger) {
                $this->logger->error("MQTT-Fehler beim Abonnieren: " . $e->getMessage());
            }
            return false;
        }
    }

    /**
     * Verbindung trennen
     */
    public function disconnect() {
        if ($this->connected) {
            try {
                $this->mqtt->disconnect();
                $this->connected = false;
            } catch (\Exception $e) {
                // Ignorieren
            }
        }
    }

    /**
     * Ist die Verbindung hergestellt?
     * 
     * @return bool Verbindungsstatus
     */
    public function isConnected() {
        return $this->connected;
    }

    /**
     * Destruktor
     */
    public function __destruct() {
        $this->disconnect();
    }
}