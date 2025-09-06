/**
 * @file AdvancedOTA.ino
 * @brief Advanced ESPOTAUpdater example with callbacks and custom logic
 * @author Steve Nolte
 * @version 1.0.0
 * 
 * This example demonstrates advanced usage of the ESPOTAUpdater library including:
 * - Custom update callbacks for monitoring progress
 * - Conditional update logic
 * - Manual update control
 * - LED status indication
 * - Web interface integration
 * 
 * Features demonstrated:
 * - Update available notifications
 * - Progress monitoring with LED indication
 * - User confirmation before updates
 * - Error handling and recovery
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPOTAUpdater.h>

// WiFi Configuration
const char* ssid = "YourWiFiNetwork";
const char* password = "YourWiFiPassword";

// GitHub Repository Configuration
const char* GITHUB_REPO = "username/repository";
const int CURRENT_VERSION = 150;  // v1.50

// Hardware Configuration
const int LED_PIN = 2;
const int BUTTON_PIN = 0;  // Boot button on most ESP32 boards

// Create instances
ESPOTAUpdater otaUpdater(GITHUB_REPO, CURRENT_VERSION);
WebServer server(80);

// Update state variables
bool updateAvailable = false;
String pendingUpdateUrl = "";
int pendingVersion = 0;
bool updateInProgress = false;

// Function declarations
void setupWebServer();
void handleRoot();
void handleUpdate();
void handleStatus();
void blinkLED(int times, int delayMs = 200);
void setLED(bool state);

// Callback functions for OTA events
void onUpdateAvailable(int currentVersion, int newVersion, const String& downloadUrl) {
    Serial.println("\n*** UPDATE AVAILABLE ***");
    Serial.printf("Current version: v%d.%02d\n", currentVersion / 100, currentVersion % 100);
    Serial.printf("New version: v%d.%02d\n", newVersion / 100, newVersion % 100);
    Serial.println("Download URL: " + downloadUrl);
    
    // Store update information
    updateAvailable = true;
    pendingUpdateUrl = downloadUrl;
    pendingVersion = newVersion;
    
    // Visual indication - rapid blinks
    blinkLED(5, 100);
    
    Serial.println("Update ready! Press boot button or visit web interface to install.");
}

void onUpdateProgress(size_t progress, size_t total) {
    static unsigned long lastPrint = 0;
    float percentage = (float)progress / total * 100;
    
    // Print progress every 5% or every 2 seconds
    if (percentage >= (int)(percentage / 5) * 5 || millis() - lastPrint > 2000) {
        Serial.printf("Update progress: %.1f%% (%zu/%zu bytes)\n", percentage, progress, total);
        lastPrint = millis();
        
        // LED indication - flash faster as progress increases
        if ((int)percentage % 10 == 0) {
            setLED(true);
            delay(50);
            setLED(false);
        }
    }
}

void onUpdateComplete(bool success, const String& message) {
    updateInProgress = false;
    
    if (success) {
        Serial.println("\n*** UPDATE SUCCESSFUL ***");
        Serial.println("Device will restart in 3 seconds...");
        
        // Success indication - solid LED for 2 seconds
        setLED(true);
        delay(2000);
        setLED(false);
        
        // Device will restart automatically
    } else {
        Serial.println("\n*** UPDATE FAILED ***");
        Serial.println("Error: " + message);
        
        // Reset update state
        updateAvailable = false;
        pendingUpdateUrl = "";
        
        // Error indication - rapid blinking
        blinkLED(10, 100);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Advanced OTA Update Example ===");
    
    // Setup hardware
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    setLED(false);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        // Blink LED while connecting
        setLED(!digitalRead(LED_PIN));
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Solid LED to indicate WiFi connected
    setLED(true);
    delay(1000);
    setLED(false);
    
    // Configure OTA updater
    otaUpdater.setBoardType("ESP32_DEVKIT");  // Adjust for your board
    otaUpdater.enableAutoUpdate(false);  // Manual updates only
    otaUpdater.setUpdateInterval(5 * 60 * 1000);  // Check every 5 minutes
    
    // Set up callbacks
    otaUpdater.setUpdateAvailableCallback(onUpdateAvailable);
    otaUpdater.setUpdateProgressCallback(onUpdateProgress);
    otaUpdater.setUpdateCompleteCallback(onUpdateComplete);
    
    // Setup web server
    setupWebServer();
    
    Serial.printf("Current firmware version: v%d.%02d\n", CURRENT_VERSION / 100, CURRENT_VERSION % 100);
    Serial.println("Web interface: http://" + WiFi.localIP().toString());
    Serial.println("Press boot button to install pending updates");
    
    // Initial update check
    otaUpdater.checkForUpdates();
}

void loop() {
    static unsigned long lastCheck = 0;
    static bool lastButtonState = HIGH;
    
    // Handle web server
    server.handleClient();
    
    // Check button for manual update trigger
    bool buttonState = digitalRead(BUTTON_PIN);
    if (lastButtonState == HIGH && buttonState == LOW) {  // Button pressed
        if (updateAvailable && !updateInProgress) {
            Serial.println("Button pressed - starting update...");
            updateInProgress = true;
            otaUpdater.performUpdate(pendingUpdateUrl.c_str());
        } else if (!updateAvailable) {
            Serial.println("Button pressed - checking for updates...");
            otaUpdater.checkForUpdates();
        }
    }
    lastButtonState = buttonState;
    
    // Periodic update checks
    if (otaUpdater.shouldCheckForUpdates(millis())) {
        Serial.println("Performing scheduled update check...");
        otaUpdater.checkForUpdates();
        otaUpdater.updateLastCheckTime(millis());
    }
    
    // Status LED indication
    static unsigned long lastBlink = 0;
    if (updateAvailable && millis() - lastBlink > 2000) {
        blinkLED(2, 100);  // Double blink every 2 seconds when update available
        lastBlink = millis();
    }
    
    delay(100);
}

void setupWebServer() {
    server.on("/", handleRoot);
    server.on("/update", HTTP_POST, handleUpdate);
    server.on("/status", handleStatus);
    
    server.begin();
    Serial.println("Web server started");
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 OTA Update</title>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:40px;} .button{background:#007bff;color:white;padding:10px 20px;border:none;border-radius:5px;margin:10px;cursor:pointer;} .status{padding:15px;margin:10px 0;border-radius:5px;} .info{background:#d1ecf1;border:1px solid #bee5eb;} .warning{background:#fff3cd;border:1px solid #ffeaa7;} .success{background:#d4edda;border:1px solid #c3e6cb;}</style>";
    html += "</head><body>";
    html += "<h1>ESP32 OTA Update Manager</h1>";
    
    html += "<div class='status info'>";
    html += "<strong>Current Version:</strong> v" + String(CURRENT_VERSION / 100) + "." + String(CURRENT_VERSION % 100, DEC);
    html += "<br><strong>Board Type:</strong> " + otaUpdater.getBoardType();
    html += "<br><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes";
    html += "<br><strong>WiFi:</strong> " + WiFi.localIP().toString();
    html += "</div>";
    
    if (updateAvailable) {
        html += "<div class='status warning'>";
        html += "<strong>Update Available!</strong><br>";
        html += "New Version: v" + String(pendingVersion / 100) + "." + String(pendingVersion % 100, DEC) + "<br>";
        html += "<form method='post' action='/update'>";
        html += "<button type='submit' class='button'>Install Update</button>";
        html += "</form>";
        html += "</div>";
    } else {
        html += "<div class='status success'>";
        html += "✓ Firmware is up to date";
        html += "</div>";
    }
    
    html += "<button onclick='location.reload()' class='button'>Refresh</button>";
    html += "<button onclick='window.location=\"/status\"' class='button'>Detailed Status</button>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void handleUpdate() {
    if (updateAvailable && !updateInProgress) {
        updateInProgress = true;
        server.send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2;url=/'></head><body><h1>Starting Update...</h1><p>Please wait while the firmware is updated. The device will restart automatically.</p></body></html>");
        
        delay(1000);  // Allow response to be sent
        otaUpdater.performUpdate(pendingUpdateUrl.c_str());
    } else {
        server.send(400, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='2;url=/'></head><body><h1>Update Error</h1><p>No update available or update already in progress.</p></body></html>");
    }
}

void handleStatus() {
    String json = "{";
    json += "\"currentVersion\":" + String(CURRENT_VERSION) + ",";
    json += "\"updateAvailable\":" + String(updateAvailable ? "true" : "false") + ",";
    json += "\"pendingVersion\":" + String(pendingVersion) + ",";
    json += "\"updateInProgress\":" + String(updateInProgress ? "true" : "false") + ",";
    json += "\"boardType\":\"" + otaUpdater.getBoardType() + "\",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"wifiIP\":\"" + WiFi.localIP().toString() + "\"";
    json += "}";
    
    server.send(200, "application/json", json);
}

void blinkLED(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        setLED(true);
        delay(delayMs);
        setLED(false);
        delay(delayMs);
    }
}

void setLED(bool state) {
    digitalWrite(LED_PIN, state);
}
