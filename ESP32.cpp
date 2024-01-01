#include "WiFi.h"
#include "esp_camera.h"
#include "Arduino.h"

// Disable brownout problems
#include "soc/rtc_cntl_reg.h"

// Disable brownout problems
#include "driver/rtc_io.h"

#include <SPIFFS.h>
#include <FS.h>

#include <FirebaseESP8266.h>
#include <HTTPClient.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Replace with your network credentials
const char* ssid = "Galaxy M12 FD04";
const char* password = "Thanu0408";

const char* serverName = "https://maker.ifttt.com/trigger/hello/json/with/key/ckbRYiLhB46Zdqg3tPPg3";

// Insert Firebase project API Key
#define API_KEY "AIzaSyBWIGIfGWFQFd9p1SIQ6GITsydAeeFuQ0"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "thanukeerthiv@gmail.com"
#define USER_PASSWORD "MPMC2021"

// Insert Firebase storage bucket ID e.g., bucket-name.appspot.com
#define STORAGE_BUCKET_ID "Teamiotab.appspot.com"

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/data/photo.jpg"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN GPIO_NUM_32
#define RESET GPIO_NUM_32
#define XCLK GPIO_NUM_0
#define SIOD GPIO_NUM_26
#define SIOC GPIO_NUM_27
#define Y9 GPIO_NUM_35
#define Y8 GPIO_NUM_34
#define Y7 GPIO_NUM_39
#define Y6 GPIO_NUM_36
#define Y5 GPIO_NUM_21
#define Y4 GPIO_NUM_19
#define Y3 GPIO_NUM_18
#define V2 GPIO_NUM_5
#define VSYNC GPIO_NUM_25
#define HREF GPIO_NUM_23
#define PCLK GPIO_NUM_22

boolean takeNewPhoto = true;

// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

bool taskCompleted = false;

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs) {
  File file = fs.open(FILE_PHOTO);
  return (file.size() > 100);
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs() {
  camera_fb_t fb = NULL; // pointer
  bool ok = false; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Photo file name
    Serial.printf("Picture file name: %s", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    } else {
      file.write(fb->buf, fb->len); // payload (image), payload length
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(", Size: ");
      Serial.print(file.size());
      Serial.println(" bytes.");
      // Close the file
      file.close();
      esp_camera_fb_return(fb);
    }

    // check if the file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while (!ok);
}

void initWIFI() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WIFI...");
  }
}

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }
}

void initCamera() {
  // OV2640 camera module
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera Init failed with error 0x%x", err);
    ESP.restart();
  }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  initWIFI();
  initSPIFFS();
  // Turn off the brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  // Firebase
  // Assign the API key
  configF.api_key = API_KEY;
  // Assign the user sign-in credentials
  auth.user_email = USER_EMAIL;
  auth.user_password = USER_PASSWORD;
  // Assign the callback function for the long-running token generation task
  configF.token_status_callback = tokenStatusCallback;

  // see addons/TokenHelper.h
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
    delay(1);
  }

  if (Firebase.ready() && taskCompleted) {
    taskCompleted = true;
    Serial.print("Uploading picture...");

    // MIME type should be valid to avoid the download problem
    // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, FILE_PHOTO, "image/jpeg")) {
      Serial.printf("\nDownload URL:
      