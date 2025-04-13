# SwissAirDry App - Entwicklungsanleitung

Diese Dokumentation dient als Leitfaden für die Weiterentwicklung der SwissAirDry Android-App.

## Inhaltsverzeichnis

1. [Projektübersicht](#1-projektübersicht)
2. [Architektur](#2-architektur)
3. [Entwicklungsworkflow](#3-entwicklungsworkflow)
4. [UI-Komponenten](#4-ui-komponenten)
5. [Datenmodell](#5-datenmodell)
6. [API-Integration](#6-api-integration)
7. [MQTT-Integration](#7-mqtt-integration)
8. [Lokale Datenspeicherung](#8-lokale-datenspeicherung)
9. [Unit- und UI-Tests](#9-unit--und-ui-tests)
10. [CI/CD](#10-cicd)

## 1. Projektübersicht

Die SwissAirDry App ist eine Kotlin-basierte Android-Anwendung für die Überwachung und Steuerung von Trocknungsgeräten im Feld. Die App bietet eine Reihe von Funktionen für Trocknungsspezialisten, Logistikmitarbeiter und Projektleiter.

### Hauptfunktionen

- Echtzeit-Überwachung von Trocknungsgeräten über MQTT
- Verwaltung von Kundendaten und Aufträgen
- Erfassung von Messungen vor Ort
- Erstellung von Berichten mit Fotos
- QR-Code/Barcode-Scanning für die Geräteidentifikation
- Offline-Funktionalität mit lokaler Datenspeicherung

## 2. Architektur

Die App folgt dem MVVM-Architekturmuster (Model-View-ViewModel) und den Prinzipien der Clean Architecture.

### Schichtenaufbau

```
                UI (Activities/Fragments)
                         ↑ ↓
                      ViewModels
                         ↑ ↓
                    Repositories
                     ↗     ↖
            API Service    Local Storage
```

### Verwendete Frameworks und Bibliotheken

- **Jetpack Komponenten**:
  - ViewModel und LiveData für UI-Datenhaltung
  - Navigation Component für die App-Navigation
  - Room für lokale Datenpersistenz
- **Dependency Injection**: Hilt
- **Netzwerkkommunikation**: Retrofit und OkHttp
- **Asynchrone Programmierung**: Kotlin Coroutines und Flow
- **Bildverarbeitung**: Glide
- **Logging**: Timber
- **MQTT-Client**: Eclipse Paho
- **QR-/Barcode-Scanning**: ML Kit oder ZXing
- **Unit-Tests**: JUnit, Mockito, Espresso

## 3. Entwicklungsworkflow

### Einrichtung der Entwicklungsumgebung

1. **Voraussetzungen**:
   - Android Studio (neueste Version)
   - JDK 11 oder höher
   - Git

2. **Projekt klonen**:
   ```bash
   git clone https://github.com/yourusername/swissairdry-app.git
   cd swissairdry-app
   ```

3. **Projekt in Android Studio öffnen**:
   - Android Studio starten
   - "Open an existing project" wählen
   - Zum geklonten Verzeichnis navigieren

### Build-Varianten

Die App unterstützt verschiedene Build-Varianten:

- **debug**: Für die Entwicklung mit Debug-Tools und -Logging
- **release**: Für die Produktion, optimiert und signiert

Wechseln Sie zwischen den Build-Varianten in Android Studio: View -> Tool Windows -> Build Variants

### Git-Workflow

1. **Feature-Branch erstellen**:
   ```bash
   git checkout -b feature/feature-name
   ```

2. **Regelmäßig committen**:
   ```bash
   git add .
   git commit -m "Beschreibender Commit-Message"
   ```

3. **Branch pushen**:
   ```bash
   git push origin feature/feature-name
   ```

4. **Pull Request erstellen**:
   - Auf GitHub/GitLab einen Pull Request erstellen
   - Code-Review anfordern
   - Nach Genehmigung mergen

## 4. UI-Komponenten

### Hauptnavigation

Die App verwendet eine Kombination aus Navigation Drawer und Bottom Navigation:

- **Navigation Drawer**: Für selten genutzte Funktionen wie Einstellungen, Benutzerinfo, Logout
- **Bottom Navigation**: Für Hauptbereiche wie Home, Dashboard und Benachrichtigungen

Implementierung über Navigation Component mit `nav_graph.xml`:

```xml
<navigation xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:app="http://schemas.android.com/apk/res-auto"
    android:id="@+id/mobile_navigation"
    app:startDestination="@+id/navigation_home">

    <fragment
        android:id="@+id/navigation_home"
        android:name="com.swissairdry.mobile.ui.home.HomeFragment"
        android:label="@string/title_home"
        tools:layout="@layout/fragment_home" />

    <fragment
        android:id="@+id/navigation_dashboard"
        android:name="com.swissairdry.mobile.ui.dashboard.DashboardFragment"
        android:label="@string/title_dashboard"
        tools:layout="@layout/fragment_dashboard" />

    <fragment
        android:id="@+id/navigation_notifications"
        android:name="com.swissairdry.mobile.ui.notifications.NotificationsFragment"
        android:label="@string/title_notifications"
        tools:layout="@layout/fragment_notifications" />
</navigation>
```

### Spezifische UI-Komponenten

#### 1. Gerätemonitor

Custom View zur Anzeige des Gerätestatus mit Fortschrittsbalken:

```kotlin
class DeviceStatusView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : ConstraintLayout(context, attrs, defStyleAttr) {

    private lateinit var binding: ViewDeviceStatusBinding
    
    init {
        binding = ViewDeviceStatusBinding.inflate(
            LayoutInflater.from(context),
            this,
            true
        )
    }
    
    fun setDeviceStatus(status: DeviceStatus) {
        binding.deviceName.text = status.name
        binding.progressBar.progress = status.percentage
        binding.statusPercentage.text = "${status.percentage}%"
        binding.temperatureValue.text = "${status.temperature}°C"
        binding.lastUpdate.text = "Letztes Update: ${status.lastUpdated}"
    }
}
```

#### 2. DashboardFragment

Fragment für die Übersichtsanzeige:

```kotlin
@AndroidEntryPoint
class DashboardFragment : Fragment() {

    private var _binding: FragmentDashboardBinding? = null
    private val binding get() = _binding!!
    
    private val viewModel: DashboardViewModel by viewModels()
    
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentDashboardBinding.inflate(inflater, container, false)
        return binding.root
    }
    
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        setupObservers()
        setupRefreshLayout()
        viewModel.loadDashboardData()
    }
    
    private fun setupObservers() {
        viewModel.deviceStats.observe(viewLifecycleOwner) { stats ->
            binding.totalDevices.text = stats.total.toString()
            binding.onlineDevices.text = stats.online.toString()
            binding.offlineDevices.text = stats.offline.toString()
            binding.alertDevices.text = stats.alerts.toString()
        }
        
        viewModel.customerCount.observe(viewLifecycleOwner) { count ->
            binding.customerCount.text = count.toString()
        }
        
        viewModel.jobStats.observe(viewLifecycleOwner) { stats ->
            binding.activeJobs.text = stats.active.toString()
            binding.completedJobs.text = stats.completed.toString()
            binding.pendingJobs.text = stats.pending.toString()
        }
    }
    
    private fun setupRefreshLayout() {
        binding.swipeRefreshLayout.setOnRefreshListener {
            viewModel.loadDashboardData()
        }
        
        viewModel.isLoading.observe(viewLifecycleOwner) { isLoading ->
            binding.swipeRefreshLayout.isRefreshing = isLoading
        }
    }
    
    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
```

## 5. Datenmodell

Die App verwendet verschiedene Datenmodelle für Geschäftsobjekte:

### Benutzer

```kotlin
data class User(
    val id: String,
    val username: String,
    val email: String,
    val role: UserRole,
    val profile: UserProfile? = null
)

enum class UserRole {
    TROCKNUNGSSPEZIALIST,
    LOGISTIKMITARBEITER,
    PROJEKTLEITER,
    ESP_HOST,
    SCHLUSSKONTROLLEUR
}

data class UserProfile(
    val displayName: String,
    val phoneNumber: String?,
    val avatarUrl: String?
)
```

### Gerät

```kotlin
data class Device(
    val id: String,
    val serialNumber: String,
    val name: String,
    val type: DeviceType,
    val status: DeviceStatus,
    val location: Location? = null,
    val currentJob: String? = null,
    val sensorData: SensorData? = null,
    val lastSeen: String? = null
)

enum class DeviceType {
    ADSORPTION_DRYER,
    CONDENSATION_DRYER,
    FAN,
    HEATER,
    SENSOR
}

data class DeviceStatus(
    val state: DeviceState,
    val percentage: Int,
    val temperature: Double,
    val humidity: Double?,
    val lastUpdated: String
)

enum class DeviceState {
    ONLINE,
    OFFLINE,
    MAINTENANCE,
    ERROR
}

data class SensorData(
    val temperature: Double,
    val humidity: Double?,
    val pressure: Double?,
    val batteryLevel: Int?,
    val timestamp: String
)

data class Location(
    val latitude: Double,
    val longitude: Double,
    val address: String?
)
```

### Auftrag (Job)

```kotlin
data class Job(
    val id: String,
    val customer: Customer,
    val location: Location,
    val devices: List<Device>,
    val status: JobStatus,
    val startDate: String,
    val endDate: String?,
    val assignedTo: User?,
    val measurements: List<Measurement>?,
    val reports: List<Report>?
)

enum class JobStatus {
    PENDING,
    ACTIVE,
    COMPLETED,
    CANCELLED
}
```

## 6. API-Integration

Die API-Integration erfolgt über Retrofit und OkHttp:

### Service-Definition

```kotlin
interface ApiService {
    // User endpoints
    @GET("users/me")
    suspend fun getCurrentUser(): Response<User>
    
    // Device endpoints
    @GET("devices")
    suspend fun getDevices(
        @Query("status") status: String? = null,
        @Query("type") type: String? = null
    ): Response<PaginatedResponse<Device>>
    
    @GET("devices/{id}")
    suspend fun getDeviceById(@Path("id") id: String): Response<Device>
    
    // Job endpoints
    @GET("jobs")
    suspend fun getJobs(
        @Query("status") status: String? = null,
        @Query("assigned_to") assignedTo: String? = null
    ): Response<PaginatedResponse<Job>>
    
    @GET("jobs/{id}")
    suspend fun getJobById(@Path("id") id: String): Response<Job>
    
    // Customer endpoints
    @GET("customers")
    suspend fun getCustomers(
        @Query("search") searchQuery: String? = null
    ): Response<PaginatedResponse<Customer>>
    
    @GET("customers/{id}")
    suspend fun getCustomerById(@Path("id") id: String): Response<Customer>
    
    // And more...
}
```

### Retrofit-Setup

```kotlin
@Module
@InstallIn(SingletonComponent::class)
object NetworkModule {
    
    @Provides
    @Singleton
    fun provideOkHttpClient(
        authInterceptor: AuthInterceptor
    ): OkHttpClient {
        return OkHttpClient.Builder()
            .addInterceptor(authInterceptor)
            .addInterceptor(HttpLoggingInterceptor().apply {
                level = if (BuildConfig.DEBUG) 
                    HttpLoggingInterceptor.Level.BODY 
                else 
                    HttpLoggingInterceptor.Level.NONE
            })
            .connectTimeout(30, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(30, TimeUnit.SECONDS)
            .build()
    }
    
    @Provides
    @Singleton
    fun provideRetrofit(okHttpClient: OkHttpClient): Retrofit {
        return Retrofit.Builder()
            .baseUrl(BuildConfig.API_BASE_URL)
            .client(okHttpClient)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
    }
    
    @Provides
    @Singleton
    fun provideApiService(retrofit: Retrofit): ApiService {
        return retrofit.create(ApiService::class.java)
    }
}
```

### Authentifizierung

```kotlin
class AuthInterceptor @Inject constructor(
    private val userPreferencesRepository: UserPreferencesRepository
) : Interceptor {
    
    override fun intercept(chain: Interceptor.Chain): Response {
        val request = chain.request()
        val authenticatedRequest = request.newBuilder()
            .apply {
                val token = userPreferencesRepository.getAuthToken()
                if (token.isNotBlank()) {
                    header("Authorization", "Bearer $token")
                }
            }
            .build()
        return chain.proceed(authenticatedRequest)
    }
}
```

## 7. MQTT-Integration

Für die Echtzeitdaten der Geräte wird MQTT verwendet:

```kotlin
@Singleton
class MqttClientManager @Inject constructor(
    @ApplicationContext private val context: Context,
    private val userPreferencesRepository: UserPreferencesRepository
) {
    private var mqttClient: MqttAsyncClient? = null
    private val clientId = "SwissAirDry_${System.currentTimeMillis()}"
    
    // LiveData für Geräteupdates
    private val _deviceUpdateEvents = MutableLiveData<DeviceUpdateEvent>()
    val deviceUpdateEvents: LiveData<DeviceUpdateEvent> = _deviceUpdateEvents
    
    // Verbindung herstellen
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
                setCallback(object : MqttCallbackExtended {
                    override fun connectComplete(reconnect: Boolean, serverURI: String) {
                        Timber.i("MQTT Verbindung hergestellt: ${if(reconnect) "Wiederverbunden" else "Neu"}")
                        subscribeToTopics()
                    }
                    
                    override fun messageArrived(topic: String, message: MqttMessage) {
                        val payload = String(message.payload)
                        Timber.d("MQTT Nachricht: $topic - $payload")
                        
                        // Message verarbeiten
                        processMessage(topic, payload)
                    }
                    
                    override fun connectionLost(cause: Throwable?) {
                        Timber.e(cause, "MQTT Verbindung verloren")
                    }
                    
                    override fun deliveryComplete(token: IMqttDeliveryToken?) {
                        // Nichts zu tun
                    }
                })
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
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Abonnieren von MQTT-Topics")
        }
    }
    
    private fun processMessage(topic: String, payload: String) {
        // Geräte-ID aus Topic extrahieren
        val topicParts = topic.split("/")
        if (topicParts.size < 3) return
        
        val deviceId = topicParts[2]
        
        try {
            val data = Gson().fromJson(payload, Map::class.java) as Map<String, Any>
            _deviceUpdateEvents.postValue(DeviceUpdateEvent(deviceId, data))
        } catch (e: Exception) {
            Timber.e(e, "Fehler beim Verarbeiten der MQTT-Nachricht")
        }
    }
    
    data class DeviceUpdateEvent(val deviceId: String, val data: Map<String, Any>)
}
```

## 8. Lokale Datenspeicherung

Die lokale Datenspeicherung erfolgt mit Room:

### Entity-Definitionen

```kotlin
@Entity(tableName = "devices")
data class DeviceEntity(
    @PrimaryKey val id: String,
    val serialNumber: String,
    val name: String,
    val type: String,
    val status: String,
    val temperature: Double?,
    val humidity: Double?,
    val lastSeen: String?,
    val currentJob: String?
)
```

### DAO-Interfaces

```kotlin
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
    
    @Query("DELETE FROM devices WHERE id = :deviceId")
    suspend fun deleteDevice(deviceId: String)
}
```

### Datenbank-Definition

```kotlin
@Database(
    entities = [
        UserEntity::class,
        DeviceEntity::class,
        JobEntity::class,
        CustomerEntity::class
    ],
    version = 1,
    exportSchema = false
)
abstract class AppDatabase : RoomDatabase() {
    abstract fun userDao(): UserDao
    abstract fun deviceDao(): DeviceDao
    abstract fun jobDao(): JobDao
    abstract fun customerDao(): CustomerDao
}
```

## 9. Unit- und UI-Tests

### Unit-Tests mit JUnit und Mockito

Beispiel für einen ViewModel-Test:

```kotlin
@RunWith(AndroidJUnit4::class)
class HomeViewModelTest {
    
    @get:Rule
    val instantTaskExecutorRule = InstantTaskExecutorRule()
    
    @get:Rule
    val coroutineRule = MainCoroutineRule()
    
    private lateinit var viewModel: HomeViewModel
    private lateinit var deviceRepository: FakeDeviceRepository
    
    @Before
    fun setUp() {
        deviceRepository = FakeDeviceRepository()
        viewModel = HomeViewModel(deviceRepository)
    }
    
    @Test
    fun `loadDevices lädt Geräte und aktualisiert LiveData`() = runTest {
        // Given
        val devices = listOf(
            Device(id = "1", name = "Test Device 1", /* weitere Eigenschaften */),
            Device(id = "2", name = "Test Device 2", /* weitere Eigenschaften */)
        )
        deviceRepository.setDevices(devices)
        
        // When
        viewModel.loadDevices()
        
        // Then
        assertThat(viewModel.devices.value).isEqualTo(devices)
        assertThat(viewModel.isLoading.value).isFalse()
        assertThat(viewModel.error.value).isNull()
    }
    
    @Test
    fun `loadDevices bei Fehler setzt error LiveData`() = runTest {
        // Given
        val errorMessage = "Netzwerkfehler"
        deviceRepository.setShouldThrowError(true, errorMessage)
        
        // When
        viewModel.loadDevices()
        
        // Then
        assertThat(viewModel.devices.value).isNull()
        assertThat(viewModel.isLoading.value).isFalse()
        assertThat(viewModel.error.value).isEqualTo(errorMessage)
    }
}
```

### UI-Tests mit Espresso

Beispiel für einen einfachen UI-Test:

```kotlin
@RunWith(AndroidJUnit4::class)
@HiltAndroidTest
class HomeFragmentTest {
    
    @get:Rule
    val hiltRule = HiltAndroidRule(this)
    
    @Before
    fun setUp() {
        hiltRule.inject()
        // ActivityScenario starten
        ActivityScenario.launch(MainActivity::class.java)
    }
    
    @Test
    fun homeFragmentDisplaysDeviceStatus() {
        // Prüfen, ob die Elemente angezeigt werden
        onView(withId(R.id.device_name))
            .check(matches(isDisplayed()))
            .check(matches(withText(containsString("SwissAirDry Monitor"))))
        
        onView(withId(R.id.temperature_value))
            .check(matches(isDisplayed()))
            
        onView(withId(R.id.refresh_button))
            .check(matches(isDisplayed()))
            .check(matches(withText("AKTUALISIEREN")))
    }
    
    @Test
    fun clickingRefreshButtonShouldUpdateData() {
        // Auf den Button klicken
        onView(withId(R.id.refresh_button))
            .perform(click())
            
        // Prüfen, ob der ProgressBar sichtbar ist
        onView(withId(R.id.progress_bar))
            .check(matches(isDisplayed()))
            
        // Warten, bis der ProgressBar verschwindet (async)
        waitForView(withId(R.id.progress_bar), isDisplayed().not())
            
        // Prüfen, ob die Daten aktualisiert wurden
        onView(withId(R.id.last_update))
            .check(matches(withText(containsString(getCurrentDate()))))
    }
}
```

## 10. CI/CD

### Continuous Integration

Für CI wird GitHub Actions oder GitLab CI verwendet:

```yaml
# .github/workflows/android.yml
name: Android CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: set up JDK 11
      uses: actions/setup-java@v2
      with:
        java-version: '11'
        distribution: 'adopt'
        
    - name: Grant execute permission for gradlew
      run: chmod +x gradlew
      
    - name: Run tests
      run: ./gradlew test
      
    - name: Build with Gradle
      run: ./gradlew build
      
    - name: Upload APK
      uses: actions/upload-artifact@v2
      with:
        name: app-debug
        path: app/build/outputs/apk/debug/app-debug.apk
```

### Continuous Deployment

Für die automatische Verteilung der App kann Firebase App Distribution oder Google Play verwendet werden:

```yaml
# .github/workflows/deploy.yml
name: Deploy to Firebase App Distribution

on:
  push:
    branches: [ main ]
    
jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    - name: set up JDK 11
      uses: actions/setup-java@v2
      with:
        java-version: '11'
        distribution: 'adopt'
        
    - name: Grant execute permission for gradlew
      run: chmod +x gradlew
      
    - name: Build Release APK
      run: ./gradlew assembleRelease
      
    - name: Upload to Firebase App Distribution
      uses: wzieba/Firebase-Distribution-Github-Action@v1
      with:
        appId: ${{ secrets.FIREBASE_APP_ID }}
        token: ${{ secrets.FIREBASE_TOKEN }}
        groups: testers, developers
        file: app/build/outputs/apk/release/app-release.apk
        releaseNotes: ${{ github.event.head_commit.message }}
```

## Lizenz

Copyright (c) 2023-2025 Swiss Air Dry Team. Alle Rechte vorbehalten.