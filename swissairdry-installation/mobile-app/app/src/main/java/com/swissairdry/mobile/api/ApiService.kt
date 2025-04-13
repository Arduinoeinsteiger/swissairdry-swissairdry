package com.swissairdry.mobile.api

import com.swissairdry.mobile.data.models.*
import retrofit2.Response
import retrofit2.http.*

/**
 * API Service für die Kommunikation mit dem SwissAirDry Backend
 * 
 * Diese Schnittstelle definiert alle verfügbaren API-Endpunkte
 * für die Interaktion mit dem SwissAirDry Backend.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
interface ApiService {

    // ===== Authentifizierung =====
    
    @POST("auth/login")
    suspend fun login(@Body loginRequest: LoginRequest): Response<LoginResponse>
    
    @POST("auth/logout")
    suspend fun logout(@Header("Authorization") token: String): Response<Unit>
    
    @GET("auth/me")
    suspend fun getUserProfile(@Header("Authorization") token: String): Response<UserProfile>

    // ===== Geräte =====
    
    @GET("devices")
    suspend fun getDevices(
        @Header("Authorization") token: String,
        @Query("page") page: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("filter") filter: String? = null,
        @Query("sort") sort: String? = null
    ): Response<PagedResponse<Device>>
    
    @GET("devices/{id}")
    suspend fun getDeviceById(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Device>
    
    @POST("devices")
    suspend fun createDevice(
        @Header("Authorization") token: String,
        @Body device: DeviceCreateRequest
    ): Response<Device>
    
    @PUT("devices/{id}")
    suspend fun updateDevice(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body device: DeviceUpdateRequest
    ): Response<Device>
    
    @DELETE("devices/{id}")
    suspend fun deleteDevice(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Unit>
    
    @GET("devices/{id}/readings")
    suspend fun getDeviceReadings(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Query("start") startDate: String? = null,
        @Query("end") endDate: String? = null,
        @Query("type") type: String? = null
    ): Response<List<DeviceReading>>

    // ===== Kunden =====
    
    @GET("customers")
    suspend fun getCustomers(
        @Header("Authorization") token: String,
        @Query("page") page: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("filter") filter: String? = null,
        @Query("sort") sort: String? = null
    ): Response<PagedResponse<Customer>>
    
    @GET("customers/{id}")
    suspend fun getCustomerById(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Customer>
    
    @POST("customers")
    suspend fun createCustomer(
        @Header("Authorization") token: String,
        @Body customer: CustomerCreateRequest
    ): Response<Customer>
    
    @PUT("customers/{id}")
    suspend fun updateCustomer(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body customer: CustomerUpdateRequest
    ): Response<Customer>
    
    @DELETE("customers/{id}")
    suspend fun deleteCustomer(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Unit>

    // ===== Aufträge =====
    
    @GET("jobs")
    suspend fun getJobs(
        @Header("Authorization") token: String,
        @Query("page") page: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("filter") filter: String? = null,
        @Query("sort") sort: String? = null,
        @Query("status") status: String? = null,
        @Query("customerId") customerId: String? = null
    ): Response<PagedResponse<Job>>
    
    @GET("jobs/{id}")
    suspend fun getJobById(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Job>
    
    @POST("jobs")
    suspend fun createJob(
        @Header("Authorization") token: String,
        @Body job: JobCreateRequest
    ): Response<Job>
    
    @PUT("jobs/{id}")
    suspend fun updateJob(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body job: JobUpdateRequest
    ): Response<Job>
    
    @DELETE("jobs/{id}")
    suspend fun deleteJob(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Unit>
    
    @GET("jobs/{id}/devices")
    suspend fun getJobDevices(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<List<Device>>
    
    @POST("jobs/{id}/devices")
    suspend fun addDeviceToJob(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body request: JobDeviceAssignRequest
    ): Response<Unit>
    
    @DELETE("jobs/{jobId}/devices/{deviceId}")
    suspend fun removeDeviceFromJob(
        @Header("Authorization") token: String,
        @Path("jobId") jobId: String,
        @Path("deviceId") deviceId: String
    ): Response<Unit>

    // ===== Messungen =====
    
    @GET("measurements")
    suspend fun getMeasurements(
        @Header("Authorization") token: String,
        @Query("page") page: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("jobId") jobId: String? = null,
        @Query("type") type: String? = null,
        @Query("startDate") startDate: String? = null,
        @Query("endDate") endDate: String? = null
    ): Response<PagedResponse<Measurement>>
    
    @GET("measurements/{id}")
    suspend fun getMeasurementById(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Measurement>
    
    @POST("measurements")
    suspend fun createMeasurement(
        @Header("Authorization") token: String,
        @Body measurement: MeasurementCreateRequest
    ): Response<Measurement>
    
    @PUT("measurements/{id}")
    suspend fun updateMeasurement(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body measurement: MeasurementUpdateRequest
    ): Response<Measurement>
    
    @DELETE("measurements/{id}")
    suspend fun deleteMeasurement(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Unit>
    
    @POST("measurements/{id}/photos")
    suspend fun uploadMeasurementPhoto(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body photo: PhotoUploadRequest
    ): Response<PhotoResponse>

    // ===== Berichte =====
    
    @GET("reports")
    suspend fun getReports(
        @Header("Authorization") token: String,
        @Query("page") page: Int? = null,
        @Query("limit") limit: Int? = null,
        @Query("jobId") jobId: String? = null,
        @Query("type") type: String? = null
    ): Response<PagedResponse<Report>>
    
    @GET("reports/{id}")
    suspend fun getReportById(
        @Header("Authorization") token: String,
        @Path("id") id: String
    ): Response<Report>
    
    @POST("reports/generate")
    suspend fun generateReport(
        @Header("Authorization") token: String,
        @Body request: ReportGenerateRequest
    ): Response<Report>
    
    @GET("reports/{id}/download")
    suspend fun downloadReport(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Query("format") format: String = "pdf"
    ): Response<ResponseBody>
    
    @POST("reports/{id}/send")
    suspend fun sendReport(
        @Header("Authorization") token: String,
        @Path("id") id: String,
        @Body request: ReportSendRequest
    ): Response<Unit>
}