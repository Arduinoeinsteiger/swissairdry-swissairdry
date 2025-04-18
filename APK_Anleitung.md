# Anleitung zur Verbindung der ESP-Geräte mit dem Smartphone-Hotspot

## Vorbereitung der Android-App

Für die SwissAirDry-App muss eine Anpassung vorgenommen werden, damit die ESP-Geräte sich mit dem Smartphone-Hotspot verbinden können:

### 1. Hotspot-Funktionalität in der App implementieren

1. Die App muss die Android-Hotspot-API nutzen (benötigt entsprechende Berechtigungen).
2. Fügen Sie folgenden Code in die App ein, um den Hotspot zu aktivieren:

```java
private void startHotspot() {
    // Überprüfe, ob die Berechtigung für WLAN-Hotspot vorhanden ist
    if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_WIFI_STATE) != PackageManager.PERMISSION_GRANTED ||
        ContextCompat.checkSelfPermission(this, Manifest.permission.CHANGE_WIFI_STATE) != PackageManager.PERMISSION_GRANTED) {
        ActivityCompat.requestPermissions(this, 
                new String[]{Manifest.permission.ACCESS_WIFI_STATE, Manifest.permission.CHANGE_WIFI_STATE}, 
                PERMISSION_REQUEST_CODE);
        return;
    }
    
    // Feste Konfiguration für den ESP-Verbindungsmodus
    String ssid = "SwissAirDry-Mobile";
    String password = "12345678";
    
    // Zeige Hotspot-Informationen an
    TextView infoTextView = findViewById(R.id.hotspotInfoText);
    infoTextView.setText("WLAN-Name: " + ssid + "\nPasswort: " + password + 
                         "\n\nBitte starten Sie jetzt Ihr SwissAirDry-Gerät");
    
    // Hotspot aktivieren
    WifiManager wifiManager = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
    
    // Prüfen, ob WLAN aktiviert ist und ggf. deaktivieren
    if (wifiManager.isWifiEnabled()) {
        wifiManager.setWifiEnabled(false);
    }
    
    // Hotspot aktivieren (benötigt ggf. zusätzliche Bibliotheken oder Root-Zugriff)
    try {
        Method method = wifiManager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
        WifiConfiguration config = new WifiConfiguration();
        config.SSID = ssid;
        config.preSharedKey = password;
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        method.invoke(wifiManager, config, true);
        
        Toast.makeText(this, "Hotspot aktiviert", Toast.LENGTH_SHORT).show();
    } catch (Exception e) {
        e.printStackTrace();
        Toast.makeText(this, "Fehler beim Aktivieren des Hotspots: " + e.getMessage(), 
                      Toast.LENGTH_LONG).show();
    }
}
```

3. Fügen Sie einen Button zur App-Oberfläche hinzu, um den Hotspot zu starten.

### 2. ESP-Gerät-Erkennung

Fügen Sie Code hinzu, um verbundene ESP-Geräte zu erkennen:

```java
private void scanForDevices() {
    // Prüfe, ob Hotspot aktiv ist
    if (!isHotspotActive()) {
        Toast.makeText(this, "Bitte aktivieren Sie zuerst den Hotspot", Toast.LENGTH_SHORT).show();
        return;
    }
    
    // IP des Hotspots ist normalerweise 192.168.43.1 (Standard für Android-Hotspots)
    String baseIp = "192.168.43.";
    
    // Suche nach ESP-Geräten im Netzwerk
    new Thread(() -> {
        List<String> foundDevices = new ArrayList<>();
        
        // UI aktualisieren, dass die Suche läuft
        runOnUiThread(() -> {
            TextView statusText = findViewById(R.id.statusText);
            statusText.setText("Suche nach Geräten...");
        });
        
        // Netzwerk scannen (IP-Bereich 2-254)
        for (int i = 2; i < 255; i++) {
            String host = baseIp + i;
            try {
                // Ping-Timeout auf 100ms setzen für schnelle Überprüfung
                if (InetAddress.getByName(host).isReachable(100)) {
                    // Prüfe, ob es ein SwissAirDry-Gerät ist, indem HTTP-Anfrage an Port 80 gesendet wird
                    URL url = new URL("http://" + host + "/");
                    HttpURLConnection connection = (HttpURLConnection) url.openConnection();
                    connection.setConnectTimeout(200);
                    connection.setReadTimeout(200);
                    connection.setRequestMethod("GET");
                    
                    if (connection.getResponseCode() == 200) {
                        // Prüfe, ob es sich um ein SwissAirDry-Gerät handelt
                        BufferedReader reader = new BufferedReader(
                                new InputStreamReader(connection.getInputStream()));
                        String line;
                        boolean isSwissAirDry = false;
                        while ((line = reader.readLine()) != null) {
                            if (line.contains("SwissAirDry")) {
                                isSwissAirDry = true;
                                break;
                            }
                        }
                        reader.close();
                        
                        if (isSwissAirDry) {
                            foundDevices.add(host);
                        }
                    }
                }
            } catch (Exception e) {
                // Ignoriere nicht erreichbare Hosts
            }
        }
        
        // UI aktualisieren mit den gefundenen Geräten
        runOnUiThread(() -> {
            if (foundDevices.isEmpty()) {
                TextView statusText = findViewById(R.id.statusText);
                statusText.setText("Keine Geräte gefunden. Stellen Sie sicher, dass Ihr ESP-Gerät " +
                                  "mit dem Hotspot verbunden ist.");
            } else {
                // ListView mit gefundenen Geräten füllen
                ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                        android.R.layout.simple_list_item_1, foundDevices);
                ListView deviceList = findViewById(R.id.deviceList);
                deviceList.setAdapter(adapter);
                
                TextView statusText = findViewById(R.id.statusText);
                statusText.setText(foundDevices.size() + " Gerät(e) gefunden. Bitte wählen Sie ein Gerät aus.");
            }
        });
    }).start();
}
```

## Konfiguration der ESP-Geräte

Die ESP-Geräte müssen so konfiguriert werden, dass sie sich mit dem Smartphone-Hotspot verbinden können:

### 1. Standardmäßige WLAN-Konfiguration

Aktualisieren Sie den ESP-Firmware-Code, um standardmäßig nach dem Smartphone-Hotspot zu suchen:

```cpp
// Standardmäßig nach diesem Hotspot suchen
const char* defaultSsid = "SwissAirDry-Mobile";
const char* defaultPassword = "12345678";
```

### 2. Automatische Verbindung mit Hotspot

Wenn keine gespeicherte Konfiguration vorhanden ist oder die Verbindung zum konfigurierten WLAN fehlschlägt, sollte der ESP automatisch versuchen, sich mit dem Standard-Hotspot zu verbinden:

```cpp
bool connectWiFi() {
  // Zuerst mit gespeicherter Konfiguration verbinden
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  
  // Wenn keine Verbindung, mit Standard-Hotspot verbinden
  Serial.println("Verbinde mit Standard-Hotspot...");
  WiFi.begin(defaultSsid, defaultPassword);
  
  attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  return WiFi.status() == WL_CONNECTED;
}
```

## Verwendung in der Praxis

1. Der Benutzer öffnet die SwissAirDry-App auf dem Smartphone.
2. Er wählt "Gerät verbinden" und aktiviert den Hotspot über die App.
3. Der Benutzer schaltet das ESP-Gerät ein oder startet es neu.
4. Das ESP-Gerät versucht, sich mit dem Standard-Hotspot zu verbinden.
5. Die App scannt das Netzwerk und zeigt gefundene Geräte an.
6. Der Benutzer wählt das gefundene Gerät aus und kann es dann konfigurieren.

## Wichtige Hinweise

1. Die Standard-WLAN-Einstellungen (SSID "SwissAirDry-Mobile" und Passwort "12345678") sollten in allen ESP-Geräten und in der App gleich sein.
2. Wenn ein ESP-Gerät bereits mit einem anderen WLAN verbunden ist, muss es zurückgesetzt werden, damit es den Standard-Hotspot sucht.
3. Die Hotspot-Funktionalität in modernen Android-Versionen kann Einschränkungen haben und erfordert möglicherweise zusätzliche Berechtigungen.
4. Für eine verbesserte Benutzererfahrung könnte ein QR-Code für die WLAN-Konfiguration verwendet werden.

Mit dieser Konfiguration können sich die ESP-Geräte automatisch mit dem Smartphone-Hotspot verbinden, was die Ersteinrichtung und Konfiguration deutlich erleichtert.