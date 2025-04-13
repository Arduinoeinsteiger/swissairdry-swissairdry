package com.swissairdry.mobile.data.model

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

/**
 * User - Benutzermodell
 *
 * Diese Klasse repräsentiert einen Benutzer im System mit
 * seinen grundlegenden Daten und Berechtigungen.
 * 
 * @author Swiss Air Dry Team <info@swissairdry.com>
 * @copyright 2023-2025 Swiss Air Dry Team
 */
@Parcelize
data class User(
    val id: Int,
    val username: String,
    val email: String,
    val firstName: String,
    val lastName: String,
    val role: String,
    val permissions: List<String>,
    val isActive: Boolean
) : Parcelable {
    
    /**
     * Gibt den vollständigen Namen des Benutzers zurück
     */
    val displayName: String
        get() = "$firstName $lastName"
    
    /**
     * Prüft, ob der Benutzer eine bestimmte Berechtigung hat
     */
    fun hasPermission(permission: String): Boolean {
        return permissions.contains(permission)
    }
    
    /**
     * Prüft, ob der Benutzer eine der Berechtigungen hat
     */
    fun hasAnyPermission(vararg requiredPermissions: String): Boolean {
        return requiredPermissions.any { permission -> permissions.contains(permission) }
    }
    
    /**
     * Prüft, ob der Benutzer die Rolle "Trocknungsspezialist" hat
     */
    fun isTrocknungsspezialist(): Boolean {
        return role == "trocknungsspezialist"
    }
    
    /**
     * Prüft, ob der Benutzer die Rolle "Logistikmitarbeiter" hat
     */
    fun isLogistikmitarbeiter(): Boolean {
        return role == "logistikmitarbeiter"
    }
    
    /**
     * Prüft, ob der Benutzer die Rolle "Projektleiter" hat
     */
    fun isProjektleiter(): Boolean {
        return role == "projektleiter"
    }
    
    /**
     * Prüft, ob der Benutzer die Rolle "ESP Host" hat
     */
    fun isEspHost(): Boolean {
        return role == "esp_host"
    }
    
    /**
     * Prüft, ob der Benutzer die Rolle "Schlusskontrolleur" hat
     */
    fun isSchlusskontrolleur(): Boolean {
        return role == "schlusskontrolleur"
    }
    
    /**
     * Prüft, ob der Benutzer Administrator-Berechtigungen hat
     */
    fun isAdmin(): Boolean {
        return isProjektleiter() || hasPermission("admin")
    }
}