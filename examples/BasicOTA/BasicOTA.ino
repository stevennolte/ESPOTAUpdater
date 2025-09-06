/**
 * @file BasicOTA.ino
 * @brief Basic ESPOTAUpdater example
 * @author Steve Nolte
 * @version 1.0.0
 * 
 * This example demonstrates the simplest possible usage of the ESPOTAUpdater library.
 * It connects to WiFi, checks for updates from GitHub releases, and automatically
 * installs any newer firmware found.
 * 
 * Hardware Requirements:
 * - ESP32 development board
 * - WiFi connection
 * 
 * Setup:
 * 1. Update WiFi credentials below
 * 2. Change GITHUB_REPO to your repository
 * 3. Set CURRENT_VERSION to match your firmware
 * 4. Upload and monitor serial output
 */

#include <WiFi.h>
#include <ESPOTAUpdater.h>

// WiFi Configuration
const char* ssid = "YourWiFiNetwork";
const char* password = "YourWiFiPassword";

// GitHub Repository Configuration
const char* GITHUB_REPO = "username/repository";  // Change to your repository
const int CURRENT_VERSION = 100;  // v1.00 - update this with each release

// Create OTA updater instance
ESPOTAUpdater otaUpdater(GITHUB_REPO, CURRENT_VERSION);

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Basic OTA Update Example ===");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Set board type (optional - will auto-detect from build flags)
    #ifdef BOARD_TYPE
        otaUpdater.setBoardType(String(BOARD_TYPE));
        Serial.println("Board type: " + String(BOARD_TYPE));
    #endif
    
    // Configure update settings
    otaUpdater.enableAutoUpdate(true);  // Enable automatic updates
    otaUpdater.setUpdateInterval(2 * 60 * 1000);  // Check every 2 minutes for demo
    
    Serial.printf("Current firmware version: %d\n", CURRENT_VERSION);
    Serial.println("Checking for updates...");
    
    // Perform initial update check
    otaUpdater.checkForUpdates();
    
    Serial.println("Setup complete. Monitoring for updates...");
}

void loop() {
    static unsigned long lastStatusPrint = 0;
    
    // Check for updates periodically
    if (otaUpdater.shouldCheckForUpdates(millis())) {
        Serial.println("Performing scheduled update check...");
        otaUpdater.checkForUpdates();
        otaUpdater.updateLastCheckTime(millis());
    }
    
    // Print status every 30 seconds
    if (millis() - lastStatusPrint > 30000) {
        Serial.println("Status: Running - monitoring for updates");
        Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
        lastStatusPrint = millis();
    }
    
    // Your main application code goes here
    
    delay(1000);
}
