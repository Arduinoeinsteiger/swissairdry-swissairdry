# SwissAirDry Mobile App Entwicklungsanleitung

Diese Anleitung beschreibt, wie die SwissAirDry Mobile App für Android entwickelt werden kann. Die App ermöglicht Technikern und Projektleitern den mobilen Zugriff auf das SwissAirDry-System, um Geräte zu verwalten, Messungen durchzuführen und Aufträge zu bearbeiten.

## 1. Einrichtung der Entwicklungsumgebung

### Voraussetzungen

- [Android Studio](https://developer.android.com/studio) (neueste Version)
- JDK 11 oder höher
- Git für die Versionskontrolle
- Ein Android-Gerät oder Emulator für Tests

### Einrichtung

1. Android Studio installieren
2. Das SwissAirDry Mobile App Repository klonen:
   ```
   git clone https://github.com/swissairdry/mobile-app.git
   ```
3. Projekt in Android Studio öffnen
4. Gradle-Synchronisierung abwarten

## 2. Projektstruktur

Die App verwendet eine MVVM-Architektur (Model-View-ViewModel) mit folgender Struktur:

```
app/
  ├── src/
  │   ├── main/
  │   │   ├── java/com/swissairdry/mobile/
  │   │   │   ├── api/             # API-Zugriff und Retrofit-Schnittstellen
  │   │   │   ├── data/            # Datenmodelle und Repositories
  │   │   │   ├── di/              # Dependency Injection mit Hilt
  │   │   │   ├── ui/              # UI-Komponenten
  │   │   │   │   ├── auth/        # Authentifizierung 
  │   │   │   │   ├── dashboard/   # Dashboard
  │   │   │   │   ├── devices/     # Geräteverwaltung
  │   │   │   │   ├── jobs/        # Auftragsverwaltung
  │   │   │   │   ├── measurements/ # Messungen
  │   │   │   │   └── settings/    # Einstellungen
  │   │   │   ├── utils/           # Hilfsfunktionen
  │   │   │   └── SwissAirDryApp.kt # Hauptanwendungsklasse
  │   │   │
  │   │   ├── res/                 # Ressourcen (Layouts, Strings, usw.)
  │   │   └── AndroidManifest.xml
  │   │
  │   └── test/                    # Unit-Tests
  │
  ├── build.gradle                 # App-spezifische Gradle-Konfiguration
  └── proguard-rules.pro           # ProGuard-Regeln
```

## 3. Hauptabhängigkeiten

In der `build.gradle`-Datei des App-Moduls folgende Abhängigkeiten hinzufügen:

```gradle
dependencies {
    // Android-Kernbibliotheken
    implementation 'androidx.core:core-ktx:1.9.0'
    implementation 'androidx.appcompat:appcompat:1.6.1'
    implementation 'com.google.android.material:material:1.8.0'
    implementation 'androidx.constraintlayout:constraintlayout:2.1.4'
    
    // MVVM-Architektur 
    implementation 'androidx.lifecycle:lifecycle-viewmodel-ktx:2.5.1'
    implementation 'androidx.lifecycle:lifecycle-livedata-ktx:2.5.1'
    implementation 'androidx.navigation:navigation-fragment-ktx:2.5.3'
    implementation 'androidx.navigation:navigation-ui-ktx:2.5.3'
    
    // Dependency Injection
    implementation 'com.google.dagger:hilt-android:2.44'
    kapt 'com.google.dagger:hilt-android-compiler:2.44'
    
    // Netzwerkkommunikation
    implementation 'com.squareup.retrofit2:retrofit:2.9.0'
    implementation 'com.squareup.retrofit2:converter-gson:2.9.0'
    implementation 'com.squareup.okhttp3:logging-interceptor:4.10.0'
    
    // Bildverarbeitung
    implementation 'com.github.bumptech.glide:glide:4.15.0'
    
    // MQTT für IoT-Geräte
    implementation 'org.eclipse.paho:org.eclipse.paho.client.mqttv3:1.2.5'
    implementation 'org.eclipse.paho:org.eclipse.paho.android.service:1.1.1'
    
    // QR-Code-Scanner
    implementation 'com.journeyapps:zxing-android-embedded:4.3.0'
    
    // Biometrische Authentifizierung
    implementation 'androidx.biometric:biometric:1.2.0-alpha05'
    
    // Tests
    testImplementation 'junit:junit:4.13.2'
    androidTestImplementation 'androidx.test.ext:junit:1.1.5'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.5.1'
}
```

## 4. API-Integration

### API-Service einrichten

1. Erstellen Sie ein `ApiService.kt` Interface in der `api`-Package:

```kotlin
interface ApiService {
    // Authentifizierung
    @POST("auth/login")
    suspend fun login(@Body loginRequest: LoginRequest): Response<AuthResponse>
    
    // Geräte
    @GET("devices")
    suspend fun getDevices(): Response<List<Device>>
    
    @GET("devices/{id}")
    suspend fun getDevice(@Path("id") deviceId: String): Response<Device>
    
    @PUT("devices/{id}")
    suspend fun updateDevice(@Path("id") deviceId: String, @Body device: Device): Response<Device>
    
    // Aufträge
    @GET("jobs")
    suspend fun getJobs(): Response<List<Job>>
    
    @GET("jobs/{id}")
    suspend fun getJob(@Path("id") jobId: String): Response<Job>
    
    @POST("jobs")
    suspend fun createJob(@Body job: Job): Response<Job>
    
    @PUT("jobs/{id}")
    suspend fun updateJob(@Path("id") jobId: String, @Body job: Job): Response<Job>
    
    // Messungen
    @GET("measurements")
    suspend fun getMeasurements(@Query("device_id") deviceId: String? = null): Response<List<Measurement>>
    
    @POST("measurements")
    suspend fun createMeasurement(@Body measurement: Measurement): Response<Measurement>
    
    // MQTT-Konfiguration
    @GET("mqtt/config")
    suspend fun getMqttConfig(): Response<MqttConfig>
}
```

2. Erstellen Sie ein `RetrofitClient.kt` zur API-Initialisierung:

```kotlin
@Module
@InstallIn(SingletonComponent::class)
object NetworkModule {
    
    @Provides
    @Singleton
    fun provideOkHttpClient(authInterceptor: AuthInterceptor): OkHttpClient {
        return OkHttpClient.Builder()
            .addInterceptor(authInterceptor)
            .addInterceptor(HttpLoggingInterceptor().apply { 
                level = HttpLoggingInterceptor.Level.BODY 
            })
            .connectTimeout(30, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(30, TimeUnit.SECONDS)
            .build()
    }
    
    @Provides
    @Singleton
    fun provideRetrofit(okHttpClient: OkHttpClient, @Named("baseUrl") baseUrl: String): Retrofit {
        return Retrofit.Builder()
            .baseUrl(baseUrl)
            .client(okHttpClient)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
    }
    
    @Provides
    @Singleton
    fun provideApiService(retrofit: Retrofit): ApiService {
        return retrofit.create(ApiService::class.java)
    }
    
    @Provides
    @Named("baseUrl")
    fun provideBaseUrl(@ApplicationContext context: Context): String {
        val preferences = PreferenceManager.getDefaultSharedPreferences(context)
        return preferences.getString("api_url", "https://api.swissairdry.com/") ?: "https://api.swissairdry.com/"
    }
}
```

## 5. Datenmodelle

Erstellen Sie die Hauptdatenmodelle in der `data/models`-Package:

### Device.kt
```kotlin
data class Device(
    val id: String,
    val name: String,
    val type: String,
    val serialNumber: String,
    val status: String,
    val lastMaintenance: String?,
    val currentLocation: Location?,
    val assignedJob: String?,
    val sensorData: SensorData?
)

data class SensorData(
    val temperature: Float?,
    val humidity: Float?,
    val pressure: Float?,
    val batteryLevel: Int?,
    val lastUpdate: String
)

data class Location(
    val jobId: String?,
    val address: String?,
    val room: String?,
    val floor: String?,
    val coordinates: Coordinates?
)

data class Coordinates(
    val latitude: Double,
    val longitude: Double
)
```

### Job.kt
```kotlin
data class Job(
    val id: String,
    val title: String,
    val description: String?,
    val customer: Customer,
    val address: Address,
    val status: String,
    val priority: String,
    val assignedTechnicians: List<User>,
    val startDate: String,
    val endDate: String?,
    val devices: List<String>, // Geräte-IDs
    val notes: List<JobNote>,
    val photos: List<Photo>
)

data class Customer(
    val id: String,
    val name: String,
    val contactPerson: String?,
    val phone: String?,
    val email: String?
)

data class Address(
    val street: String,
    val number: String,
    val city: String,
    val zipCode: String,
    val country: String
)

data class JobNote(
    val id: String,
    val text: String,
    val createdBy: String,
    val createdAt: String
)

data class Photo(
    val id: String,
    val url: String,
    val description: String?,
    val takenAt: String,
    val takenBy: String
)
```

### Measurement.kt
```kotlin
data class Measurement(
    val id: String,
    val deviceId: String,
    val jobId: String,
    val type: String,
    val value: Float,
    val unit: String,
    val timestamp: String,
    val coordinates: Coordinates?,
    val takenBy: String,
    val roomIdentifier: String?,
    val notes: String?
)
```

### User.kt
```kotlin
data class User(
    val id: String,
    val username: String,
    val email: String,
    val name: String,
    val role: String,
    val permissions: List<String>
)
```

## 6. Authentifizierungsimplementierung

### Login Activity

```kotlin
@AndroidEntryPoint
class LoginActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityLoginBinding
    private val viewModel: LoginViewModel by viewModels()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityLoginBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setupUI()
        observeViewModel()
    }
    
    private fun setupUI() {
        binding.btnLogin.setOnClickListener {
            val username = binding.etUsername.text.toString()
            val password = binding.etPassword.text.toString()
            
            if (username.isNotEmpty() && password.isNotEmpty()) {
                viewModel.login(username, password)
            } else {
                showError(getString(R.string.error_empty_credentials))
            }
        }
        
        binding.btnSettings.setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }
    }
    
    private fun observeViewModel() {
        viewModel.loginResult.observe(this) { result ->
            when (result) {
                is Resource.Loading -> showLoading(true)
                is Resource.Success -> {
                    showLoading(false)
                    navigateToDashboard()
                }
                is Resource.Error -> {
                    showLoading(false)
                    showError(result.message ?: getString(R.string.error_login_failed))
                }
            }
        }
    }
    
    private fun showLoading(isLoading: Boolean) {
        binding.progressBar.visibility = if (isLoading) View.VISIBLE else View.GONE
        binding.btnLogin.isEnabled = !isLoading
    }
    
    private fun showError(message: String) {
        Snackbar.make(binding.root, message, Snackbar.LENGTH_LONG).show()
    }
    
    private fun navigateToDashboard() {
        startActivity(Intent(this, MainActivity::class.java))
        finish()
    }
}
```

### Login ViewModel

```kotlin
@HiltViewModel
class LoginViewModel @Inject constructor(
    private val authRepository: AuthRepository
) : ViewModel() {
    
    private val _loginResult = MutableLiveData<Resource<AuthResponse>>()
    val loginResult: LiveData<Resource<AuthResponse>> = _loginResult
    
    fun login(username: String, password: String) {
        viewModelScope.launch {
            _loginResult.value = Resource.Loading()
            try {
                val response = authRepository.login(username, password)
                _loginResult.value = Resource.Success(response)
            } catch (e: Exception) {
                _loginResult.value = Resource.Error(e.message ?: "Login fehlgeschlagen")
            }
        }
    }
}
```

## 7. Hauptnavigation implementieren

### MainActivity 

```kotlin
@AndroidEntryPoint
class MainActivity : AppCompatActivity() {
    
    private lateinit var binding: ActivityMainBinding
    private lateinit var navController: NavController
    private val viewModel: MainViewModel by viewModels()
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        
        setupNavigation()
        observeViewModel()
    }
    
    private fun setupNavigation() {
        val navHostFragment = supportFragmentManager
            .findFragmentById(R.id.nav_host_fragment) as NavHostFragment
        navController = navHostFragment.navController
        
        binding.bottomNav.setupWithNavController(navController)
        
        // Bottom Navigation Elemente verwalten basierend auf Benutzerberechtigungen
        viewModel.userPermissions.observe(this) { permissions ->
            binding.bottomNav.menu.findItem(R.id.nav_admin).isVisible = 
                permissions.contains("ADMIN_ACCESS")
        }
    }
    
    private fun observeViewModel() {
        viewModel.connectionStatus.observe(this) { isConnected ->
            binding.tvConnectionStatus.visibility = 
                if (!isConnected) View.VISIBLE else View.GONE
        }
        
        viewModel.userInfo.observe(this) { user ->
            binding.tvUserName.text = user.name
            binding.tvUserRole.text = user.role
        }
    }
    
    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return true
    }
    
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_settings -> {
                startActivity(Intent(this, SettingsActivity::class.java))
                true
            }
            R.id.action_logout -> {
                viewModel.logout()
                startActivity(Intent(this, LoginActivity::class.java))
                finish()
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
}
```

## 8. Implementierung der Geräteverwaltung

### DevicesFragment

```kotlin
@AndroidEntryPoint
class DevicesFragment : Fragment() {
    
    private var _binding: FragmentDevicesBinding? = null
    private val binding get() = _binding!!
    
    private val viewModel: DevicesViewModel by viewModels()
    private lateinit var devicesAdapter: DevicesAdapter
    
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentDevicesBinding.inflate(inflater, container, false)
        return binding.root
    }
    
    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        
        setupUI()
        setupRecyclerView()
        observeViewModel()
        
        // Geräte beim Start laden
        viewModel.loadDevices()
    }
    
    private fun setupUI() {
        binding.fabAddDevice.setOnClickListener {
            findNavController().navigate(R.id.action_devices_to_add_device)
        }
        
        binding.swipeRefresh.setOnRefreshListener {
            viewModel.loadDevices()
        }
        
        binding.etSearch.doAfterTextChanged { text ->
            viewModel.searchDevices(text.toString())
        }
        
        binding.btnScanQr.setOnClickListener {
            requestCameraPermissionAndScan()
        }
    }
    
    private fun setupRecyclerView() {
        devicesAdapter = DevicesAdapter { device ->
            // Navigiere zur Gerätedetailansicht mit der Geräte-ID
            findNavController().navigate(
                DevicesFragmentDirections.actionDevicesToDeviceDetail(device.id)
            )
        }
        
        binding.rvDevices.apply {
            adapter = devicesAdapter
            layoutManager = LinearLayoutManager(requireContext())
            addItemDecoration(DividerItemDecoration(requireContext(), DividerItemDecoration.VERTICAL))
        }
    }
    
    private fun observeViewModel() {
        viewModel.devices.observe(viewLifecycleOwner) { result ->
            when (result) {
                is Resource.Loading -> {
                    binding.progressBar.visibility = View.VISIBLE
                    binding.tvError.visibility = View.GONE
                }
                is Resource.Success -> {
                    binding.progressBar.visibility = View.GONE
                    binding.swipeRefresh.isRefreshing = false
                    
                    if (result.data.isNullOrEmpty()) {
                        binding.tvEmpty.visibility = View.VISIBLE
                        binding.rvDevices.visibility = View.GONE
                    } else {
                        binding.tvEmpty.visibility = View.GONE
                        binding.rvDevices.visibility = View.VISIBLE
                        devicesAdapter.submitList(result.data)
                    }
                }
                is Resource.Error -> {
                    binding.progressBar.visibility = View.GONE
                    binding.swipeRefresh.isRefreshing = false
                    binding.tvError.visibility = View.VISIBLE
                    binding.tvError.text = result.message
                }
            }
        }
    }
    
    private fun requestCameraPermissionAndScan() {
        // Berechtigungsprüfung und QR-Code-Scan-Implementierung
    }
    
    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
```

## 9. Offline-Modus

Implementieren Sie einen Offline-Modus mit Room-Datenbank, um Daten lokal zu speichern und zu synchronisieren:

```kotlin
@Database(
    entities = [
        DeviceEntity::class,
        JobEntity::class,
        MeasurementEntity::class
    ],
    version = 1
)
abstract class SwissAirDryDatabase : RoomDatabase() {
    abstract fun deviceDao(): DeviceDao
    abstract fun jobDao(): JobDao
    abstract fun measurementDao(): MeasurementDao

    companion object {
        @Volatile
        private var INSTANCE: SwissAirDryDatabase? = null

        fun getDatabase(context: Context): SwissAirDryDatabase {
            return INSTANCE ?: synchronized(this) {
                val instance = Room.databaseBuilder(
                    context.applicationContext,
                    SwissAirDryDatabase::class.java,
                    "swissairdry_database"
                ).build()
                INSTANCE = instance
                instance
            }
        }
    }
}
```

## 10. Verbindung mit IoT-Geräten

### MQTT-Integration

```kotlin
@Singleton
class MqttManager @Inject constructor(
    @ApplicationContext private val context: Context,
    private val mqttRepository: MqttRepository
) {
    private var mqttClient: MqttAndroidClient? = null
    private val _connectionStatus = MutableLiveData<Boolean>()
    val connectionStatus: LiveData<Boolean> = _connectionStatus
    
    private val _deviceUpdates = MutableLiveData<DeviceUpdate>()
    val deviceUpdates: LiveData<DeviceUpdate> = _deviceUpdates
    
    fun connect() {
        viewModelScope.launch {
            try {
                val config = mqttRepository.getMqttConfig()
                
                // MQTT-Client initialisieren
                mqttClient = MqttAndroidClient(
                    context,
                    config.brokerUrl,
                    config.clientId
                )
                
                // Callback für Verbindungsänderungen
                mqttClient?.setCallback(object : MqttCallback {
                    override fun connectionLost(cause: Throwable?) {
                        _connectionStatus.postValue(false)
                    }
                    
                    override fun messageArrived(topic: String?, message: MqttMessage?) {
                        message?.let {
                            handleMessage(topic, String(it.payload))
                        }
                    }
                    
                    override fun deliveryComplete(token: IMqttDeliveryToken?) {
                        // Nachricht wurde erfolgreich gesendet
                    }
                })
                
                // Verbindungsoptionen
                val options = MqttConnectOptions().apply {
                    isAutomaticReconnect = true
                    isCleanSession = true
                    connectionTimeout = 30
                    userName = config.username
                    password = config.password.toCharArray()
                }
                
                // Verbinden
                mqttClient?.connect(options, null, object : IMqttActionListener {
                    override fun onSuccess(asyncActionToken: IMqttToken?) {
                        _connectionStatus.postValue(true)
                        
                        // Auf relevante Themen abonnieren
                        mqttClient?.subscribe("swissairdry/devices/+/status", 1)
                        mqttClient?.subscribe("swissairdry/devices/+/sensors", 1)
                        mqttClient?.subscribe("swissairdry/alerts", 1)
                    }
                    
                    override fun onFailure(asyncActionToken: IMqttToken?, exception: Throwable?) {
                        _connectionStatus.postValue(false)
                    }
                })
                
            } catch (e: Exception) {
                _connectionStatus.postValue(false)
            }
        }
    }
    
    fun disconnect() {
        mqttClient?.disconnect()
        mqttClient = null
        _connectionStatus.postValue(false)
    }
    
    fun sendCommand(deviceId: String, command: String, payload: String) {
        if (mqttClient?.isConnected == true) {
            val topic = "swissairdry/devices/$deviceId/commands"
            val message = MqttMessage(payload.toByteArray())
            message.qos = 1
            mqttClient?.publish(topic, message)
        }
    }
    
    private fun handleMessage(topic: String?, payload: String) {
        topic?.let {
            when {
                it.matches(Regex("swissairdry/devices/.+/status")) -> {
                    val deviceId = it.split("/")[2]
                    val status = try {
                        Gson().fromJson(payload, DeviceStatus::class.java)
                    } catch (e: Exception) {
                        null
                    }
                    
                    status?.let { deviceStatus ->
                        _deviceUpdates.postValue(
                            DeviceUpdate(
                                deviceId = deviceId,
                                updateType = "status",
                                data = deviceStatus
                            )
                        )
                    }
                }
                
                it.matches(Regex("swissairdry/devices/.+/sensors")) -> {
                    val deviceId = it.split("/")[2]
                    val sensorData = try {
                        Gson().fromJson(payload, SensorData::class.java)
                    } catch (e: Exception) {
                        null
                    }
                    
                    sensorData?.let { data ->
                        _deviceUpdates.postValue(
                            DeviceUpdate(
                                deviceId = deviceId,
                                updateType = "sensors",
                                data = data
                            )
                        )
                    }
                }
                
                it == "swissairdry/alerts" -> {
                    // Warnungen verarbeiten
                }
            }
        }
    }
}
```

## 11. Berechtigungen

Fügen Sie die erforderlichen Berechtigungen zur `AndroidManifest.xml` hinzu:

```xml
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.swissairdry.mobile">
    
    <!-- Internet-Berechtigung -->
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
    
    <!-- Kamera für QR-Code-Scanning -->
    <uses-permission android:name="android.permission.CAMERA" />
    
    <!-- Lokaler Speicher für Fotos -->
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" 
                     android:maxSdkVersion="28" />
    
    <!-- Standort für Gerätezuordnung -->
    <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
    <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
    
    <!-- Bluetooth für Geräteverbindung -->
    <uses-permission android:name="android.permission.BLUETOOTH" />
    <uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" 
                     android:maxSdkVersion="30" />
    <uses-permission android:name="android.permission.BLUETOOTH_SCAN" 
                     android:maxSdkVersion="30" />
    
    <!-- Wakelock für MQTT-Service -->
    <uses-permission android:name="android.permission.WAKE_LOCK" />
    
    <!-- ... Rest des Manifests ... -->
</manifest>
```

## 12. Build und Veröffentlichung

### Signieren der App

1. Erstellen Sie einen Keystore für die Signierung:
   ```
   keytool -genkey -v -keystore swissairdry.keystore -keyalg RSA -keysize 2048 -validity 10000 -alias swissairdry
   ```

2. Konfigurieren Sie die Signatureinstellungen in der `app/build.gradle`:
   ```gradle
   android {
       // ... andere Konfigurationen ...
       
       signingConfigs {
           release {
               storeFile file("../swissairdry.keystore")
               storePassword System.getenv("KEYSTORE_PASSWORD") ?: project.properties['keystore.password']
               keyAlias "swissairdry"
               keyPassword System.getenv("KEY_PASSWORD") ?: project.properties['key.password']
           }
       }
       
       buildTypes {
           release {
               minifyEnabled true
               proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
               signingConfig signingConfigs.release
           }
       }
   }
   ```

3. Release-Build erstellen:
   ```
   ./gradlew assembleRelease
   ```

4. Die APK befindet sich im Verzeichnis `app/build/outputs/apk/release/`.

## 13. Fazit

Diese Anleitung beschreibt die grundlegende Struktur und Implementierung der SwissAirDry Mobile App für Android. Stellen Sie sicher, dass alle Module richtig miteinander kommunizieren und dass die App auch im Offline-Modus funktioniert, um eine reibungslose Benutzerfahrung zu gewährleisten.

Für weitere Fragen oder detailliertere Implementierungsanleitungen wenden Sie sich bitte an das SwissAirDry-Entwicklungsteam.