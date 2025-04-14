package com.swissairdry.mobile.di

import android.content.Context
import android.content.SharedPreferences
import com.google.gson.Gson
import com.google.gson.GsonBuilder
import com.swissairdry.mobile.BuildConfig
import com.swissairdry.mobile.api.ApiService
import com.swissairdry.mobile.api.AuthInterceptor
import com.swissairdry.mobile.api.NetworkFailoverInterceptor
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import okhttp3.OkHttpClient
import okhttp3.logging.HttpLoggingInterceptor
import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory
import java.util.concurrent.TimeUnit
import javax.inject.Singleton

/**
 * Hilt-Modul zur Bereitstellung von Netzwerkkomponenten.
 */
@Module
@InstallIn(SingletonComponent::class)
object NetworkModule {

    /**
     * Stellt das OkHttpClient-Objekt bereit.
     */
    @Provides
    @Singleton
    fun provideOkHttpClient(
        authInterceptor: AuthInterceptor,
        networkFailoverInterceptor: NetworkFailoverInterceptor
    ): OkHttpClient {
        val loggingInterceptor = HttpLoggingInterceptor().apply {
            level = if (BuildConfig.DEBUG) {
                HttpLoggingInterceptor.Level.BODY
            } else {
                HttpLoggingInterceptor.Level.NONE
            }
        }

        return OkHttpClient.Builder()
            .addInterceptor(networkFailoverInterceptor) // Wichtig: Als erstes hinzufügen!
            .addInterceptor(authInterceptor)
            .addInterceptor(loggingInterceptor)
            .connectTimeout(30, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(30, TimeUnit.SECONDS)
            .build()
    }

    /**
     * Stellt das Gson-Objekt bereit.
     */
    @Provides
    @Singleton
    fun provideGson(): Gson {
        return GsonBuilder()
            .setDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'")
            .create()
    }

    /**
     * Stellt das Retrofit-Objekt bereit.
     * Die Basis-URL wird nur als Platzhalter verwendet, da der tatsächliche Server
     * durch den NetworkFailoverInterceptor bestimmt wird.
     */
    @Provides
    @Singleton
    fun provideRetrofit(okHttpClient: OkHttpClient, gson: Gson): Retrofit {
        return Retrofit.Builder()
            .baseUrl(BuildConfig.PRIMARY_API_SCHEME + "://" + BuildConfig.PRIMARY_API_HOST + ":" + BuildConfig.PRIMARY_API_PORT)
            .client(okHttpClient)
            .addConverterFactory(GsonConverterFactory.create(gson))
            .build()
    }

    /**
     * Stellt den ApiService bereit.
     */
    @Provides
    @Singleton
    fun provideApiService(retrofit: Retrofit): ApiService {
        return retrofit.create(ApiService::class.java)
    }

    /**
     * Stellt die SharedPreferences bereit.
     */
    @Provides
    @Singleton
    fun provideSharedPreferences(@ApplicationContext context: Context): SharedPreferences {
        return context.getSharedPreferences("swissairdry_preferences", Context.MODE_PRIVATE)
    }
}