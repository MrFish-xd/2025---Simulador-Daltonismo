#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "esp_http_server.h"
#include "secrets.h"
// ===========================
// Pines ESP32-CAM
// ===========================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define LED_FLASH_GPIO     4

// ===========================
// Servo con PWM nativo
// ===========================
#define PIN_SERVO      13

int servoPos = 90;
unsigned long flashTimer = 0;
bool flashActive = false;

// Función para mover servo 
void writeServo(int angle) {
  // Calcular duty cycle para servo (500us-2400us en periodo de 20ms)
  int pulseWidth = map(angle, 0, 180, 500, 2400);
  int dutyCycle = map(pulseWidth, 0, 20000, 0, 255); // 20ms = 20000us
  ledcWrite(PIN_SERVO, dutyCycle);
}

// ===========================
// WiFi
// ===========================

const char*secrets.h
const char*secrets.h
WebServer server(80);
httpd_handle_t stream_httpd = NULL;

// ===========================
// HTML Mobile + Flash
// ===========================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>ESP32-CAM</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
      background: #0a0a0a;
      color: #e0e0e0;
      overflow-x: hidden;
      padding-bottom: 80px;
    }
    
    .container {
      max-width: 100%;
      margin: 0 auto;
      padding: 12px;
    }
    
    header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 12px 0;
      margin-bottom: 16px;
      border-bottom: 1px solid #222;
    }
    
    h1 {
      font-size: 1.25rem;
      font-weight: 600;
      color: #fff;
    }
    
    .header-controls {
      display: flex;
      gap: 8px;
    }
    
    .flash-btn {
      padding: 8px 16px;
      background: #fff;
      border: none;
      border-radius: 6px;
      color: #000;
      font-size: 0.875rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      align-items: center;
      gap: 6px;
    }
    
    .flash-btn:active {
      transform: scale(0.95);
    }
    
    .flash-btn.active {
      background: #fbbf24;
      animation: flashPulse 0.5s infinite;
    }
    
    @keyframes flashPulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.7; }
    }
    
    .status {
      display: flex;
      align-items: center;
      gap: 6px;
      font-size: 0.75rem;
      color: #888;
    }
    
    .dot {
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background: #0f0;
      box-shadow: 0 0 6px #0f0;
      animation: pulse 2s infinite;
    }
    
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.4; }
    }
    
    .video-wrapper {
      position: relative;
      width: 100%;
      max-width: 800px;
      margin: 0 auto 16px;
      background: #000;
      border-radius: 8px;
      overflow: hidden;
      aspect-ratio: 4/3;
    }
    
    #stream {
      width: 100%;
      height: 100%;
      object-fit: contain;
      transition: filter 0.3s;
    }
    
    /* Filtros mejorados y más fieles */
    #stream.protanopia { 
      filter: url(#protanopia-filter);
    }
    #stream.deuteranopia { 
      filter: url(#deuteranopia-filter);
    }
    #stream.tritanopia { 
      filter: url(#tritanopia-filter);
    }
    #stream.achromatopsia { 
      filter: grayscale(100%) contrast(1.1);
    }
    
    .live {
      position: absolute;
      top: 8px;
      left: 8px;
      display: flex;
      align-items: center;
      gap: 4px;
      padding: 3px 10px;
      background: rgba(239, 68, 68, 0.95);
      border-radius: 4px;
      font-size: 0.7rem;
      font-weight: 700;
      color: #fff;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .live-dot {
      width: 5px;
      height: 5px;
      border-radius: 50%;
      background: #fff;
      animation: pulse 1.5s infinite;
    }
    
    /* Controles Servo Horizontales */
    .servo-controls {
      display: flex;
      justify-content: center;
      gap: 12px;
      margin-bottom: 16px;
      padding: 0 12px;
    }
    
    .servo-btn {
      flex: 1;
      max-width: 120px;
      height: 56px;
      background: #fff;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 4px;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
      transition: all 0.2s;
      font-size: 1.25rem;
    }
    
    .servo-btn span {
      font-size: 0.7rem;
      font-weight: 600;
      color: #333;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .servo-btn:active {
      transform: scale(0.95);
      box-shadow: 0 1px 4px rgba(0, 0, 0, 0.3);
    }
    
    .servo-btn.center {
      background: #555;
    }
    
    .servo-btn.center span {
      color: #fff;
    }
    
    .servo-position {
      text-align: center;
      padding: 8px;
      margin-bottom: 16px;
      font-size: 1.25rem;
      font-weight: 700;
      color: #fff;
    }
    
    /* Filtros en Grid Responsive */
    .filters {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
      gap: 8px;
      padding: 0 12px;
      margin-bottom: 16px;
    }
    
    .filter-btn {
      padding: 10px;
      background: #1a1a1a;
      border: 1px solid #333;
      border-radius: 6px;
      color: #888;
      font-size: 0.875rem;
      cursor: pointer;
      transition: all 0.2s;
      white-space: nowrap;
      text-align: center;
    }
    
    .filter-btn:active {
      transform: scale(0.95);
    }
    
    .filter-btn.active {
      background: #fff;
      color: #000;
      border-color: #fff;
      font-weight: 600;
    }
    
    @media (max-width: 480px) {
      .filters {
        grid-template-columns: repeat(2, 1fr);
      }
      
      .servo-btn {
        height: 64px;
        font-size: 1.5rem;
      }
      
      .servo-btn span {
        font-size: 0.65rem;
      }
    }
    
    @media (min-width: 768px) {
      .container {
        padding: 20px;
      }
      
      header {
        padding: 16px 0;
        margin-bottom: 24px;
      }
      
      h1 {
        font-size: 1.5rem;
      }
      
      .video-wrapper {
        margin-bottom: 24px;
      }
      
      .servo-controls {
        max-width: 600px;
        margin: 0 auto 24px;
      }
      
      .filters {
        max-width: 800px;
        margin: 0 auto 24px;
        grid-template-columns: repeat(5, 1fr);
      }
    }
  </style>
</head>
<body>
  <svg style="display: none;">
    <defs>
      <!-- Protanopia -->
      <filter id="protanopia-filter">
        <feColorMatrix type="matrix" values="0.56667, 0.43333, 0, 0, 0
                                              0.55833, 0.44167, 0, 0, 0
                                              0, 0.24167, 0.75833, 0, 0
                                              0, 0, 0, 1, 0"/>
      </filter>
      <!-- Deuteranopia -->
      <filter id="deuteranopia-filter">
        <feColorMatrix type="matrix" values="0.625, 0.375, 0, 0, 0
                                              0.7, 0.3, 0, 0, 0
                                              0, 0.3, 0.7, 0, 0
                                              0, 0, 0, 1, 0"/>
      </filter>
      <!-- Tritanopia -->
      <filter id="tritanopia-filter">
        <feColorMatrix type="matrix" values="0.95, 0.05, 0, 0, 0
                                              0, 0.43333, 0.56667, 0, 0
                                              0, 0.475, 0.525, 0, 0
                                              0, 0, 0, 1, 0"/>
      </filter>
    </defs>
  </svg>

  <div class="container">
    <header>
      <div>
        <h1>ESP32-CAM</h1>
      </div>
      <div class="header-controls">
        <div class="status">
          <div class="dot"></div>
          <span>LIVE</span>
        </div>
        <button class="flash-btn" id="flashBtn" onclick="toggleFlash()">
          💡 <span id="flashText">Flash</span>
        </button>
      </div>
    </header>

    <div class="video-wrapper">
      <img id="stream" src="" alt="Stream">
      <div class="live">
        <div class="live-dot"></div>
        <span>Live</span>
      </div>
    </div>

    <div class="servo-position" id="servoPos">● 90°</div>

    <div class="servo-controls">
      <button class="servo-btn" onclick="moveServo(0)">
        ⬅️
        <span>Izquierda</span>
      </button>
      <button class="servo-btn center" onclick="moveServo(90)">
        ⏺️
        <span>Centro</span>
      </button>
      <button class="servo-btn" onclick="moveServo(180)">
        ➡️
        <span>Derecha</span>
      </button>
    </div>

    <div class="filters">
      <button class="filter-btn active" onclick="setFilter('normal')">👁️ Normal</button>
      <button class="filter-btn" onclick="setFilter('protanopia')">🔴 Protanopia</button>
      <button class="filter-btn" onclick="setFilter('deuteranopia')">🟢 Deuteranopia</button>
      <button class="filter-btn" onclick="setFilter('tritanopia')">🔵 Tritanopia</button>
      <button class="filter-btn" onclick="setFilter('achromatopsia')">⚫ Acromatopsia</button>
    </div>
  </div>

  <script>
    let flashTimeout = null;

    function initStream() {
      const streamImg = document.getElementById('stream');
      const baseUrl = window.location.protocol + '//' + window.location.hostname;
      streamImg.src = baseUrl + ':81/stream';
    }

    function setFilter(filter) {
      document.querySelectorAll('.filter-btn').forEach(btn => btn.classList.remove('active'));
      event.target.classList.add('active');
      
      const streamImg = document.getElementById('stream');
      streamImg.className = filter !== 'normal' ? filter : '';
    }

    async function moveServo(pos) {
      try {
        const response = await fetch(`/servo?pos=${pos}`);
        await response.text();
        document.getElementById('servoPos').textContent = `● ${pos}°`;
      } catch (error) {
        console.error('Error:', error);
      }
    }

    async function toggleFlash() {
      const btn = document.getElementById('flashBtn');
      const text = document.getElementById('flashText');
      
      try {
        const response = await fetch('/flash');
        const data = await response.text();
        
        if (data === 'ON') {
          btn.classList.add('active');
          
          let seconds = 10;
          text.textContent = seconds + 's';
          
          const countdown = setInterval(() => {
            seconds--;
            if (seconds > 0) {
              text.textContent = seconds + 's';
            } else {
              clearInterval(countdown);
              btn.classList.remove('active');
              text.textContent = 'Flash';
            }
          }, 1000);
        } else if (data === 'BUSY') {
          alert('⚠️ El flash ya está siendo usado por otro dispositivo');
        }
      } catch (error) {
        console.error('Error:', error);
        alert('❌ Error al activar el flash');
      }
    }

    async function updateServoPosition() {
      try {
        const response = await fetch('/servo/position');
        const data = await response.text();
        document.getElementById('servoPos').textContent = `● ${data}°`;
      } catch (error) {
        // Silencioso
      }
    }

    window.onload = function() {
      initStream();
      setInterval(updateServoPosition, 2000);
    };
  </script>
</body>
</html>
)rawliteral";

// ===========================
// Handler Streaming Multi-Cliente
// ===========================
static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[128];

  static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
  static const char* _STREAM_BOUNDARY = "\r\n--frame\r\n";
  static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK) return res;

  // Habilitar chunked transfer encoding para múltiples clientes
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  Serial.println("📹 Nuevo cliente conectado al stream");

  while(true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("⚠️ Error capturando frame");
      res = ESP_FAIL;
      break;
    }

    if(fb->format != PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
      esp_camera_fb_return(fb);
      fb = NULL;
      if(!jpeg_converted) {
        Serial.println("⚠️ Error convirtiendo a JPEG");
        res = ESP_FAIL;
        break;
      }
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }
    
    if(res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    
    if(fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    
    if(res != ESP_OK) {
      Serial.println("📴 Cliente desconectado del stream");
      break;
    }

    taskYIELD();
  }
  
  return res;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  config.max_open_sockets = 5;  // Permitir hasta 5 conexiones simultáneas
  config.lru_purge_enable = true;  // Limpiar conexiones inactivas automáticamente

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("✅ Streaming: puerto 81 (Multi-cliente)");
  }
}

// ===========================
// Setup
// ===========================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=================================");
  Serial.println("ESP32-CAM Optimizado");
  Serial.println("=================================");

  // Configurar servo con nueva API de ESP32 Core 3.x
  ledcAttach(PIN_SERVO, 50, 8); // Pin, Frecuencia (50Hz), Resolución (8 bits)
  writeServo(servoPos);
  Serial.println("✅ Servo configurado");
  
  pinMode(LED_FLASH_GPIO, OUTPUT);
  digitalWrite(LED_FLASH_GPIO, LOW);

  // Configurar cámara
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Error cámara: 0x%x\n", err);
    delay(5000);
    return;
  }
  Serial.println("✅ Cámara OK");

  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_gain_ctrl(s, 1);
    s->set_hmirror(s, 0);
    s->set_vflip(s, 0);
  }

  // WiFi
  Serial.println("\n📡 Conectando WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi falló");
    return;
  }

  // Servidor web
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send_P(200, "text/html", INDEX_HTML);
    Serial.println("🌐 Nueva conexión web");
  });

  server.on("/servo", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (server.hasArg("pos")) {
      int pos = server.arg("pos").toInt();
      if (pos >= 0 && pos <= 180) {
        servoPos = pos;
        writeServo(servoPos);
        server.send(200, "text/plain", "OK");
        Serial.printf("🎮 Servo: %d° (Cliente: %s)\n", pos, server.client().remoteIP().toString().c_str());
      } else {
        server.send(400, "text/plain", "Inválido");
      }
    } else {
      server.send(400, "text/plain", "Falta pos");
    }
  });

  server.on("/servo/position", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", String(servoPos));
  });

  server.on("/flash", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (!flashActive) {
      digitalWrite(LED_FLASH_GPIO, HIGH);
      flashActive = true;
      flashTimer = millis();
      server.send(200, "text/plain", "ON");
      Serial.printf("💡 Flash ON (Cliente: %s)\n", server.client().remoteIP().toString().c_str());
    } else {
      server.send(200, "text/plain", "BUSY");
      Serial.println("⚠️ Flash ya está activo");
    }
  });

  server.begin();
  Serial.println("✅ Servidor: puerto 80");

  startCameraServer();

  Serial.println("=================================");
  Serial.println("🚀 Sistema listo!");
  Serial.println("=================================\n");
}

// ===========================
// Loop
// ===========================
void loop() {
  server.handleClient();
  
  // Control de flash (10 segundos)
  if (flashActive && (millis() - flashTimer >= 10000)) {
    digitalWrite(LED_FLASH_GPIO, LOW);
    flashActive = false;
    Serial.println("💡 Flash OFF");
  }
  
  delay(1);
}