package com.swissairdry.mobile.api

import com.swissairdry.mobile.data.model.User
import retrofit2.Response
import retrofit2.http.*

/**
 * ApiService - Retrofit-Interface für API-Aufrufe
 *
 * Dieses Interface definiert alle API-Endpunkte, die von der App verwendet werden.
 * Es wird von Retrofit verwendet, um HTTP-Anfragen zu erstellen und die Antworten
 * in Kotlin-Objekte zu konvertieren.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
interface ApiService {

    // Authentifizierung
    @POST("auth/login")
    suspend fun login(@Body loginRequest: LoginRequest): Response<LoginResponse>
    
    @POST("auth/refresh")
    suspend fun refreshToken(): Response<LoginResponse>
    
    @POST("auth/logout")
    suspend fun logout(): Response<Unit>
    
    // Benutzer
    @GET("users/me")
    suspend fun getCurrentUser(): Response<User>
    
    @GET("users/{userId}")
    suspend fun getUserById(@Path("userId") userId: Int): Response<User>
    
    // Geräte
    @GET("devices")
    suspend fun getDevices(
        @Query("status") status: String? = null,
        @Query("limit") limit: Int? = null,
        @Query("offset") offset: Int? = null
    ): Response<DeviceListResponse>
    
    @GET("devices/{deviceId}")
    suspend fun getDeviceById(@Path("deviceId") deviceId: String): Response<DeviceResponse>
    
    @GET("devices/{deviceId}/measurements")
    suspend fun getDeviceMeasurements(
        @Path("deviceId") deviceId: String,
        @Query("from") from: String? = null,
        @Query("to") to: String? = null
    ): Response<MeasurementListResponse>
    
    // Aufträge
    @GET("jobs")
    suspend fun getJobs(
        @Query("status") status: String? = null,
        @Query("limit") limit: Int? = null,
        @Query("offset") offset: Int? = null
    ): Response<JobListResponse>
    
    @GET("jobs/{jobId}")
    suspend fun getJobById(@Path("jobId") jobId: Int): Response<JobResponse>
    
    @POST("jobs")
    suspend fun createJob(@Body jobRequest: JobRequest): Response<JobResponse>
    
    @PUT("jobs/{jobId}")
    suspend fun updateJob(
        @Path("jobId") jobId: Int,
        @Body jobRequest: JobRequest
    ): Response<JobResponse>
    
    // Kunden
    @GET("customers")
    suspend fun getCustomers(
        @Query("search") search: String? = null,
        @Query("limit") limit: Int? = null,
        @Query("offset") offset: Int? = null
    ): Response<CustomerListResponse>
    
    @GET("customers/{customerId}")
    suspend fun getCustomerById(@Path("customerId") customerId: Int): Response<CustomerResponse>
    
    // Messungen
    @POST("measurements")
    suspend fun createMeasurement(@Body measurementRequest: MeasurementRequest): Response<MeasurementResponse>
    
    // Berichte
    @GET("reports")
    suspend fun getReports(
        @Query("jobId") jobId: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("offset") offset: Int? = null
    ): Response<ReportListResponse>
    
    @GET("reports/{reportId}")
    suspend fun getReportById(@Path("reportId") reportId: Int): Response<ReportResponse>
    
    @POST("reports")
    suspend fun createReport(@Body reportRequest: ReportRequest): Response<ReportResponse>
    
    // Fotos
    @Multipart
    @POST("photos")
    suspend fun uploadPhoto(
        @Part("jobId") jobId: Int,
        @Part("description") description: String,
        @Part photo: MultipartBody.Part
    ): Response<PhotoResponse>
    
    @GET("photos")
    suspend fun getPhotos(
        @Query("jobId") jobId: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("offset") offset: Int? = null
    ): Response<PhotoListResponse>
    
    // System
    @GET("system/health")
    suspend fun getSystemHealth(): Response<SystemHealthResponse>
}

// Datenmodelle für API-Requests und -Responses

data class LoginRequest(
    val username: String,
    val password: String
)

data class LoginResponse(
    val token: String,
    val refreshToken: String,
    val user: User
)

data class DeviceListResponse(
    val items: List<DeviceResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class DeviceResponse(
    val id: String,
    val name: String,
    val type: String,
    val status: String,
    val lastSeen: String?,
    val measurements: List<MeasurementResponse>?
)

data class MeasurementListResponse(
    val items: List<MeasurementResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class MeasurementResponse(
    val id: Int,
    val deviceId: String,
    val timestamp: String,
    val type: String,
    val value: Double,
    val unit: String
)

data class MeasurementRequest(
    val deviceId: String,
    val type: String,
    val value: Double,
    val unit: String
)

data class JobListResponse(
    val items: List<JobResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class JobResponse(
    val id: Int,
    val customerId: Int,
    val title: String,
    val description: String,
    val status: String,
    val createdAt: String,
    val updatedAt: String,
    val customer: CustomerResponse?
)

data class JobRequest(
    val customerId: Int,
    val title: String,
    val description: String,
    val status: String
)

data class CustomerListResponse(
    val items: List<CustomerResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class CustomerResponse(
    val id: Int,
    val name: String,
    val contactPerson: String,
    val email: String,
    val phone: String,
    val address: String,
    val city: String,
    val zipCode: String
)

data class ReportListResponse(
    val items: List<ReportResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class ReportResponse(
    val id: Int,
    val jobId: Int,
    val title: String,
    val content: String,
    val createdAt: String,
    val updatedAt: String,
    val createdBy: Int
)

data class ReportRequest(
    val jobId: Int,
    val title: String,
    val content: String
)

data class PhotoListResponse(
    val items: List<PhotoResponse>,
    val total: Int,
    val limit: Int,
    val offset: Int
)

data class PhotoResponse(
    val id: Int,
    val jobId: Int,
    val url: String,
    val description: String,
    val createdAt: String,
    val createdBy: Int
)

data class SystemHealthResponse(
    val status: String,
    val version: String,
    val database: Boolean,
    val mqttBroker: Boolean
)