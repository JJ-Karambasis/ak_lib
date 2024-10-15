plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.example.ak_atomic_unit_test"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.example.ak_atomic_unit_test"
        minSdk = 21
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    buildFeatures {
        prefab = true
    }
}

dependencies {
    implementation("androidx.startup:startup-runtime:1.2.0")
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.appcompat)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}