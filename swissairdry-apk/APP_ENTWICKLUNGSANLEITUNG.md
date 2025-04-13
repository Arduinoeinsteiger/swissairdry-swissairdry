# SwissAirDry App Entwicklungsanleitung

Diese Anleitung beschreibt detailliert, wie Sie die SwissAirDry-App erweitern und anpassen können.

## 1. Aktueller Entwicklungsstand

Die App ist bereits mit folgenden Funktionen implementiert:

- **Grundgerüst**: MVVM-Architektur, Dependency Injection, API-Anbindung
- **Navigation**: Drawer und Bottom-Navigation für alle Hauptbereiche
- **Dashboard**: Übersicht mit Geräte-, Auftrags- und Kundenzahlen
- **Dunkles Thema**: Unterstützung für Tag/Nacht-Design
- **Grundlegende UI-Komponenten**: Layouts für wesentliche Screens

Der Screenshot zeigt den aktuellen Stand der Home-Ansicht:
- Gerätemonitor mit Status-Anzeige (77%)
- Temperaturanzeige
- Zeitstempel der letzten Aktualisierung
- Aktualisieren-Button
- Bottom-Navigation (Home, Dashboard, Benachrichtigungen)

## 2. Architektur der App

Die App folgt dem MVVM-Muster und Clean Architecture Prinzipien:

```
                    UI (Activities/Fragments)
                             ↑ ↓
                          ViewModels
                             ↑ ↓
                        Repositories
                         ↗     ↖
                API Service    Local Storage
```

### Wichtige Pakete und Klassen

- **com.swissairdry.mobile.ui**: UI-Komponenten, Aktivitäten und Fragmente
- **com.swissairdry.mobile.data.model**: Datenmodelle
- **com.swissairdry.mobile.data.repository**: Repository-Implementierungen
- **com.swissairdry.mobile.api**: API-Service und Netzwerkkommunikation
- **com.swissairdry.mobile.di**: Dependency Injection Module (Hilt)

## 3. Weitere Entwicklungsschritte

### 3.1 UI-Komponenten implementieren

Um die Benutzeroberfläche zu erweitern, erstellen Sie neue Fragmente und ViewModels:

1. **Fragment erstellen**:
   ```kotlin
   // DeviceDetailFragment.kt
   @AndroidEntryPoint
   class DeviceDetailFragment : Fragment() {
       private var _binding: FragmentDeviceDetailBinding? = null
       private val binding get() = _binding!!
       private val viewModel: DeviceDetailViewModel by viewModels()
       
       override fun onCreateView(...): View {
           _binding = FragmentDeviceDetailBinding.inflate(inflater, container, false)
           return binding.root
       }
       
       override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
           super.onViewCreated(view, savedInstanceState)
           setupObservers()
           setupListeners()
           
           // Device-ID aus Argumenten holen und Daten laden
           viewModel.loadDeviceData(args.deviceId)
       }
       
       private fun setupObservers() {
           viewModel.deviceData.observe(viewLifecycleOwner) { device ->
               // UI aktualisieren
               binding.deviceName.text = device.name
               binding.temperatureValue.text = "${device.temperature}°C"
               binding.humidityValue.text = "${device.humidity}%"
               updateStatusIndicator(device.status)
           }
       }
   }
   ```

2. **Layout erstellen**:
   ```xml
   <!-- fragment_device_detail.xml -->
   <androidx.constraintlayout.widget.ConstraintLayout
       xmlns:android="http://schemas.android.com/apk/res/android"
       xmlns:app="http://schemas.android.com/apk/res-auto"
       android:layout_width="match_parent"
       android:layout_height="match_parent"
       android:padding="16dp">
       
       <TextView
           android:id="@+id/device_name"
           android:layout_width="wrap_content"
           android:layout_height="wrap_content"
           android:textSize="20sp"
           android:textStyle="bold"
           app:layout_constraintTop_toTopOf="parent"
           app:layout_constraintStart_toStartOf="parent" />
           
       <!-- Weitere UI-Elemente hier einfügen -->
       
   </androidx.constraintlayout.widget.ConstraintLayout>
   ```

3. **ViewModel implementieren**:
   ```kotlin
   // DeviceDetailViewModel.kt
   @HiltViewModel
   class DeviceDetailViewModel @Inject constructor(
       private val deviceRepository: DeviceRepository
   ) : ViewModel() {
       private val _deviceData = MutableLiveData<Device>()
       val deviceData: LiveData<Device> = _deviceData
       
       fun loadDeviceData(deviceId: String) {
           viewModelScope.launch {
               try {
                   val device = deviceRepository.getDeviceById(deviceId)
                   _deviceData.value = device
               } catch (e: Exception) {
                   Timber.e(e, "Fehler beim Laden der Gerätedaten")
               }
           }
       }
   }
   ```

4. **Navigation aktualisieren**:
   In der `navigation/mobile_navigation.xml` Datei:
   ```xml
   <fragment
       android:id="@+id/deviceDetailFragment"
       android:name="com.swissairdry.mobile.ui.devices.DeviceDetailFragment"
       android:label="@string/device_details"
       tools:layout="@layout/fragment_device_detail">
       <argument
           android:name="deviceId"
           app:argType="string" />
   </fragment>
   ```

### 3.2 Repository-Implementierungen

Implementieren Sie die fehlenden Repository-Klassen für die Datenquellen:

```kotlin
// DeviceRepositoryImpl.kt
@Singleton
class DeviceRepositoryImpl @Inject constructor(
    private val apiService: ApiService,
    private val deviceDao: DeviceDao
) : DeviceRepository {

    override suspend fun getDevices(): List<Device> {
        // Versuche zuerst, Daten von der API zu laden
        return try {
            val response = apiService.getDevices()
            if (response.isSuccessful) {
                val devices = response.body()?.items?.map { it.toDevice() } ?: emptyList()
                // Cache in lokaler Datenbank aktualisieren
                deviceDao.insertAll(devices.map { it.toDeviceEntity() })
                devices
            } else {
                // Bei API-Fehler, lokale Daten verwenden
                deviceDao.getAllDevices().map { it.toDevice() }
            }
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden der Geräte")
            // Bei Netzwerkfehlern, lokale Daten verwenden
            deviceDao.getAllDevices().map { it.toDevice() }
        }
    }

    override suspend fun getDeviceById(deviceId: String): Device {
        return try {
            val response = apiService.getDeviceById(deviceId)
            if (response.isSuccessful) {
                val device = response.body()?.toDevice()
                    ?: throw Exception("Gerät nicht gefunden")
                // In lokaler DB aktualisieren
                deviceDao.insert(device.toDeviceEntity())
                device
            } else {
                // Bei API-Fehler, lokale Daten verwenden
                deviceDao.getDeviceById(deviceId)?.toDevice()
                    ?: throw Exception("Gerät nicht gefunden")
            }
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Laden des Geräts")
            // Bei Netzwerkfehlern, lokale Daten verwenden
            deviceDao.getDeviceById(deviceId)?.toDevice()
                ?: throw Exception("Gerät nicht gefunden")
        }
    }
}
```

### 3.3 Lokale Datenpersistenz

Implementieren Sie die Room-Datenbank für Offline-Funktionalität:

1. **Entity-Klassen erstellen**:
   ```kotlin
   // DeviceEntity.kt
   @Entity(tableName = "devices")
   data class DeviceEntity(
       @PrimaryKey val id: String,
       val name: String,
       val type: String,
       val status: String,
       val lastSeen: String?,
       val temperature: Double?,
       val humidity: Double?
   )
   ```

2. **DAO-Interfaces implementieren**:
   ```kotlin
   // DeviceDao.kt
   @Dao
   interface DeviceDao {
       @Query("SELECT * FROM devices")
       suspend fun getAllDevices(): List<DeviceEntity>
       
       @Query("SELECT * FROM devices WHERE id = :deviceId")
       suspend fun getDeviceById(deviceId: String): DeviceEntity?
       
       @Insert(onConflict = OnConflictStrategy.REPLACE)
       suspend fun insert(device: DeviceEntity)
       
       @Insert(onConflict = OnConflictStrategy.REPLACE)
       suspend fun insertAll(devices: List<DeviceEntity>)
   }
   ```

3. **Datenbank-Klasse definieren**:
   ```kotlin
   // AppDatabase.kt
   @Database(
       entities = [DeviceEntity::class, JobEntity::class, CustomerEntity::class],
       version = 1,
       exportSchema = false
   )
   abstract class AppDatabase : RoomDatabase() {
       abstract fun deviceDao(): DeviceDao
       abstract fun jobDao(): JobDao
       abstract fun customerDao(): CustomerDao
   }
   ```

4. **DI-Modul für die Datenbank**:
   ```kotlin
   // DatabaseModule.kt
   @Module
   @InstallIn(SingletonComponent::class)
   object DatabaseModule {
       @Singleton
       @Provides
       fun provideDatabase(@ApplicationContext appContext: Context): AppDatabase {
           return Room.databaseBuilder(
               appContext,
               AppDatabase::class.java,
               "swissairdry.db"
           ).build()
       }
       
       @Singleton
       @Provides
       fun provideDeviceDao(database: AppDatabase): DeviceDao {
           return database.deviceDao()
       }
       
       // Weitere DAOs bereitstellen
   }
   ```

### 3.4 MQTT-Integration

Um Echtzeitdaten von Geräten zu empfangen, implementieren Sie einen MQTT-Client:

```kotlin
// MqttClientManager.kt
@Singleton
class MqttClientManager @Inject constructor(
    @ApplicationContext private val context: Context,
    private val userPreferencesRepository: UserPreferencesRepository
) {
    private var mqttClient: MqttAsyncClient? = null
    private val clientId = "SwissAirDry_${System.currentTimeMillis()}"
    
    private val connectionCallback = object : MqttCallbackExtended {
        override fun connectComplete(reconnect: Boolean, serverURI: String) {
            Timber.i("MQTT Verbindung hergestellt: ${if(reconnect) "Wiederverbunden" else "Neu"}")
            subscribeToTopics()
        }
        
        override fun messageArrived(topic: String, message: MqttMessage) {
            val payload = String(message.payload)
            Timber.d("MQTT Nachricht: $topic - $payload")
            
            // Nachricht verarbeiten und an entsprechende Komponenten weiterleiten
            val deviceId = extractDeviceIdFromTopic(topic)
            if (deviceId != null) {
                val data = parsePayload(payload)
                // Event an Observer senden
                _deviceUpdateEvents.value = DeviceUpdateEvent(deviceId, data)
            }
        }
        
        // Weitere Callback-Methoden implementieren
    }
    
    fun connect() {
        try {
            val brokerUrl = BuildConfig.MQTT_BROKER_URL
            val options = MqttConnectOptions().apply {
                isAutomaticReconnect = true
                isCleanSession = true
                connectionTimeout = 10
                
                // Optional: Authentifizierung hinzufügen
                val token = userPreferencesRepository.getAuthToken()
                if (token.isNotBlank()) {
                    userName = "jwt"
                    password = token.toCharArray()
                }
            }
            
            mqttClient = MqttAsyncClient(brokerUrl, clientId, MemoryPersistence()).apply {
                setCallback(connectionCallback)
                connect(options)
            }
        } catch (e: Exception) {
            Timber.e(e, "MQTT Verbindungsfehler")
        }
    }
    
    private fun subscribeToTopics() {
        try {
            mqttClient?.subscribe("swissairdry/devices/+/data", 1)
            mqttClient?.subscribe("swissairdry/devices/+/status", 1)
            
            Timber.i("MQTT Topics abonniert")
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Abonnieren von MQTT-Topics")
        }
    }
    
    // Event für Geräte-Updates
    private val _deviceUpdateEvents = MutableLiveData<DeviceUpdateEvent>()
    val deviceUpdateEvents: LiveData<DeviceUpdateEvent> = _deviceUpdateEvents
    
    data class DeviceUpdateEvent(val deviceId: String, val data: Map<String, Any>)
}
```

## 4. Testen der App

### Unit-Tests

Beispiel eines ViewModelTests:

```kotlin
@RunWith(AndroidJUnit4::class)
class HomeViewModelTest {
    
    @get:Rule
    val instantTaskExecutorRule = InstantTaskExecutorRule()
    
    @get:Rule
    val coroutineRule = MainCoroutineRule()
    
    private lateinit var viewModel: HomeViewModel
    private lateinit var userRepository: FakeUserRepository
    private lateinit var deviceRepository: FakeDeviceRepository
    private lateinit var jobRepository: FakeJobRepository
    private lateinit var customerRepository: FakeCustomerRepository
    
    @Before
    fun setup() {
        userRepository = FakeUserRepository()
        deviceRepository = FakeDeviceRepository()
        jobRepository = FakeJobRepository()
        customerRepository = FakeCustomerRepository()
        
        viewModel = HomeViewModel(
            userRepository,
            deviceRepository,
            jobRepository,
            customerRepository
        )
    }
    
    @Test
    fun `loadDashboardData lädt Daten erfolgreich`() = runBlockingTest {
        // Given - Testdaten in Repositories vorbereiten
        userRepository.setCurrentUser(testUser)
        deviceRepository.setDevices(testDevices)
        jobRepository.setJobs(testJobs)
        customerRepository.setCustomers(testCustomers)
        
        // When - ViewModel-Methode aufrufen
        viewModel.loadDashboardData()
        
        // Then - LiveData-Werte überprüfen
        assertThat(viewModel.deviceStats.value?.total).isEqualTo(3)
        assertThat(viewModel.deviceStats.value?.online).isEqualTo(2)
        assertThat(viewModel.jobStats.value?.active).isEqualTo(1)
        assertThat(viewModel.customerCount.value).isEqualTo(2)
        assertThat(viewModel.error.value).isNull()
    }
}
```

### UI-Tests

Beispiel eines Fragment-Tests:

```kotlin
@HiltAndroidTest
@RunWith(AndroidJUnit4::class)
class HomeFragmentTest {

    @get:Rule
    val hiltRule = HiltAndroidRule(this)
    
    @Before
    fun setup() {
        hiltRule.inject()
        
        // Mock-Daten für den Test vorbereiten
        val deviceStats = HomeViewModel.DeviceStats(5, 3, 1, 1)
        val jobStats = HomeViewModel.JobStats(2, 1, 3)
        
        // ViewModel mocken und bereitstellen
        val mockViewModel = mock<HomeViewModel> {
            on { deviceStats } doReturn MutableLiveData(deviceStats)
            on { jobStats } doReturn MutableLiveData(jobStats)
            on { customerCount } doReturn MutableLiveData(10)
            on { greeting } doReturn MutableLiveData("Guten Tag, Test!")
        }
        
        // ViewModel mit Hilt mocken
        launchFragmentInHiltContainer<HomeFragment> {
            // Mock-ViewModel injizieren
        }
    }
    
    @Test
    fun testDashboardDataDisplayed() {
        // Elemente prüfen
        onView(withId(R.id.greeting_text))
            .check(matches(withText("Guten Tag, Test!")))
            
        onView(withId(R.id.device_count))
            .check(matches(withText("5")))
            
        onView(withId(R.id.device_online_count))
            .check(matches(withText("3")))
            
        // Weitere UI-Elemente prüfen
    }
}
```

## 5. Erstellung einer APK

Um eine Debug-APK zu erstellen:

```bash
./gradlew assembleDebug
```

Für eine Release-Version:

```bash
./gradlew assembleRelease
```

Die APK-Dateien werden im Verzeichnis `app/build/outputs/apk/` erstellt.

## 6. App veröffentlichen

Zur Veröffentlichung im Google Play Store:

1. **App signieren**:
   ```bash
   ./gradlew bundleRelease
   ```

2. **App Bundle testen**:
   Das erzeugte App Bundle (`app/build/outputs/bundle/release/app-release.aab`) sollte mit dem Play Console Testing oder mit dem `bundletool` getestet werden.

3. **In Play Console hochladen**:
   Laden Sie das App Bundle in die Google Play Console hoch und durchlaufen Sie den Veröffentlichungsprozess.

## 7. Erweiterungsideen

- **QR-Code Scanner**: Implementierung zum schnellen Erfassen von Geräte-IDs
- **Offline-Modus**: Verbesserte Offline-Funktionalität für Außendiensteinsätze
- **Push-Benachrichtigungen**: Für wichtige Gerätewarnungen
- **Biometrische Authentifizierung**: Für sicheren und schnellen Login
- **Sprach-/Textsuche**: Zum schnellen Finden von Geräten und Aufträgen
- **Datenvisualisierung**: Grafiken und Charts für Messwerte
- **Standorterfassung**: Für eine bessere Übersicht der Gerätepositionen

## 8. Fehlerbehandlung

Häufige Fehler und Lösungen:

- **Netzwerkfehler**: Implementieren Sie eine zuverlässige Offline-Strategie mit lokaler Datenspeicherung
- **Rendering-Probleme**: Optimieren Sie Layouts für verschiedene Bildschirmgrößen
- **Speicherlecks**: Achten Sie auf korrekte Lifecycle-Verwaltung und Ressourcenfreigabe
- **Performance**: Implementieren Sie Paging für große Datenlisten und optimieren Sie Bildladezeiten

## 9. Support und Weiterentwicklung

Bei Fragen oder Problemen wenden Sie sich an das SwissAirDry-Entwicklerteam:
- E-Mail: development@swissairdry.com

## 10. Lizenz und rechtliche Hinweise

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.