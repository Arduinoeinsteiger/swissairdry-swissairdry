package com.swissairdry.mobile.data.models

import com.google.gson.annotations.SerializedName
import java.util.Date

/**
 * Generisches Antwortmodell für paginierte Ergebnisse
 */
data class PagedResponse<T>(
    val items: List<T>,
    val page: Int,
    val limit: Int,
    val total: Int
)

/**
 * Modell für Loginanfragen
 */
data class LoginRequest(
    val username: String,
    val password: String
)

/**
 * Antwortmodell für Loginanfragen
 */
data class LoginResponse(
    val token: String,
    val user: UserProfile,
    val expiresAt: String
)

/**
 * Benutzerprofilmodell
 */
data class UserProfile(
    val id: String,
    val username: String,
    val email: String,
    val firstName: String,
    val lastName: String,
    val role: String,
    val permissions: List<String>,
    val createdAt: String,
    val lastLogin: String? = null
)

/**
 * Modell für Geräte
 */
data class Device(
    val id: String,
    val name: String,
    val type: String,
    val serialNumber: String,
    val status: String,
    val location: String? = null,
    val customer: Customer? = null,
    val job: Job? = null,
    val lastReading: DeviceReading? = null,
    val properties: Map<String, Any>? = null,
    val createdAt: String,
    val updatedAt: String
)

/**
 * Modell für Geräteerstellung
 */
data class DeviceCreateRequest(
    val name: String,
    val type: String,
    val serialNumber: String,
    val properties: Map<String, Any>? = null
)

/**
 * Modell für Geräteaktualisierung
 */
data class DeviceUpdateRequest(
    val name: String? = null,
    val status: String? = null,
    val location: String? = null,
    val properties: Map<String, Any>? = null
)

/**
 * Modell für Gerätesensorwerte
 */
data class DeviceReading(
    val id: String,
    val deviceId: String,
    val timestamp: String,
    val type: String,
    val value: Double,
    val unit: String,
    val location: String? = null,
    val properties: Map<String, Any>? = null
)

/**
 * Modell für Kunden
 */
data class Customer(
    val id: String,
    val name: String,
    val contactPerson: String? = null,
    val email: String? = null,
    val phone: String? = null,
    val address: Address? = null,
    val notes: String? = null,
    val createdAt: String,
    val updatedAt: String
)

/**
 * Modell für Kundenerstellung
 */
data class CustomerCreateRequest(
    val name: String,
    val contactPerson: String? = null,
    val email: String? = null,
    val phone: String? = null,
    val address: Address? = null,
    val notes: String? = null
)

/**
 * Modell für Kundenaktualisierung
 */
data class CustomerUpdateRequest(
    val name: String? = null,
    val contactPerson: String? = null,
    val email: String? = null,
    val phone: String? = null,
    val address: Address? = null,
    val notes: String? = null
)

/**
 * Adressmodell
 */
data class Address(
    val street: String,
    val city: String,
    val zipCode: String,
    val country: String
)

/**
 * Modell für Aufträge
 */
data class Job(
    val id: String,
    val title: String,
    val description: String? = null,
    val status: String,
    val customer: Customer,
    val location: String,
    val startDate: String,
    val endDate: String? = null,
    val assignedDevices: List<Device>? = null,
    val measurements: List<Measurement>? = null,
    val reports: List<Report>? = null,
    val createdAt: String,
    val updatedAt: String
)

/**
 * Modell für Auftragserstellung
 */
data class JobCreateRequest(
    val title: String,
    val description: String? = null,
    val customerId: String,
    val location: String,
    val startDate: String,
    val endDate: String? = null
)

/**
 * Modell für Auftragsaktualisierung
 */
data class JobUpdateRequest(
    val title: String? = null,
    val description: String? = null,
    val status: String? = null,
    val location: String? = null,
    val startDate: String? = null,
    val endDate: String? = null
)

/**
 * Modell für Gerätezuweisung zu Aufträgen
 */
data class JobDeviceAssignRequest(
    val deviceId: String,
    val startDate: String? = null,
    val endDate: String? = null,
    val notes: String? = null
)

/**
 * Modell für Messungen
 */
data class Measurement(
    val id: String,
    val jobId: String,
    val type: String,
    val value: Double,
    val unit: String,
    val location: String? = null,
    val timestamp: String,
    val deviceId: String? = null,
    val photos: List<Photo>? = null,
    val notes: String? = null,
    val createdAt: String,
    val updatedAt: String
)

/**
 * Modell für Messungserstellung
 */
data class MeasurementCreateRequest(
    val jobId: String,
    val type: String,
    val value: Double,
    val unit: String,
    val location: String? = null,
    val timestamp: String? = null, // Wenn null, wird aktueller Zeitpunkt verwendet
    val deviceId: String? = null,
    val notes: String? = null
)

/**
 * Modell für Messungsaktualisierung
 */
data class MeasurementUpdateRequest(
    val value: Double? = null,
    val unit: String? = null,
    val location: String? = null,
    val notes: String? = null
)

/**
 * Modell für Fotos/Bilder
 */
data class Photo(
    val id: String,
    val url: String,
    val thumbnailUrl: String? = null,
    val title: String? = null,
    val description: String? = null,
    val createdAt: String
)

/**
 * Modell für Fotouploads
 */
data class PhotoUploadRequest(
    val base64Image: String,
    val title: String? = null,
    val description: String? = null
)

/**
 * Antwortmodell für Fotouploads
 */
data class PhotoResponse(
    val id: String,
    val url: String,
    val thumbnailUrl: String? = null
)

/**
 * Modell für Berichte
 */
data class Report(
    val id: String,
    val jobId: String,
    val type: String,
    val title: String,
    val description: String? = null,
    val status: String,
    val fileUrl: String? = null,
    val thumbnailUrl: String? = null,
    val createdAt: String,
    val updatedAt: String
)

/**
 * Anforderungsmodell für Berichtgenerierung
 */
data class ReportGenerateRequest(
    val jobId: String,
    val type: String,
    val title: String? = null,
    val description: String? = null,
    val options: Map<String, Any>? = null
)

/**
 * Anforderungsmodell für Berichtversand
 */
data class ReportSendRequest(
    val recipients: List<String>,
    val subject: String? = null,
    val message: String? = null,
    val format: String = "pdf"
)

/**
 * Modell für den Antworttext eines API-Aufrufs
 */
typealias ResponseBody = okhttp3.ResponseBody