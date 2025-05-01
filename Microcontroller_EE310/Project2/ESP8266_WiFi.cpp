#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>

// === OLED Setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === WebServer Setup ===
AsyncWebServer server(80);

String latestJoystick = "WAIT";
String latestButton = "WAIT";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  // === Initialize OLED ===
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // halt if display fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // === Create WiFi Access Point ===
  const char* ssid = "Joystick_Controller";
  const char* password = "12345678";
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();

  // === Display WiFi Info on OLED ===
  display.setCursor(0, 0);
  display.println("Joystick_Controller");
  display.print("IP: ");
  display.println(myIP);
  display.display();

  // === Serve Webpage ===
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <!DOCTYPE html><html><head><title>ESP8266 Snake</title>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <style>
        body {
          display: flex;
          flex-direction: column;
          align-items: center;
          font-family: sans-serif;
          text-align: center;
          margin: 0;
          padding: 0;
        }
        .canvas-container {
          display: flex;
          flex-direction: column;
          align-items: center;
          gap: 20px;
          margin-top: 10px;
        }
        canvas {
          border: 2px solid black;
          width: 150px;
          height: 150px;
        }
      </style>
      </head><body>
      <h2>ESP8266 Dual Snake Game</h2>
      <div><b>Joystick:</b> <span id='joystick'>WAIT</span></div>
      <div><b>Button:</b> <span id='button'>WAIT</span></div>
      <div class="canvas-container">
        <canvas id='canvasJoy' width='150' height='150'></canvas>
        <canvas id='canvasBtn' width='150' height='150'></canvas>
      </div>
      <script src='/game.js'></script>
      </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/joystick", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", latestJoystick);
  });

  server.on("/button", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", latestButton);
  });

  server.on("/game.js", HTTP_GET, [](AsyncWebServerRequest *request){
    String js = R"rawliteral(
      const joyCanvas = document.getElementById('canvasJoy');
      const btnCanvas = document.getElementById('canvasBtn');
      const joyCtx = joyCanvas.getContext('2d');
      const btnCtx = btnCanvas.getContext('2d');

      const gridSize = 15;
      const canvasSize = 150;

      let joySnake = [{x: 75, y: 75}];
      let btnSnake = [{x: 75, y: 75}];
      let joyDir = 'RIGHT';
      let btnDir = 'RIGHT';
      let joyGameOver = false;
      let btnGameOver = false;

      function drawSnake(ctx, snake) {
        ctx.clearRect(0, 0, canvasSize, canvasSize);
        ctx.fillStyle = 'black';
        snake.forEach(p => ctx.fillRect(p.x, p.y, gridSize, gridSize));
      }

      function moveSnake(snake, direction, ctx, gameFlag) {
        if (gameFlag.value) return;

        const head = {...snake[0]};
        if (direction === 'UP') head.y -= gridSize;
        if (direction === 'DOWN') head.y += gridSize;
        if (direction === 'LEFT') head.x -= gridSize;
        if (direction === 'RIGHT') head.x += gridSize;

        if (head.x < 0 || head.x >= canvasSize || head.y < 0 || head.y >= canvasSize) {
          gameFlag.value = true;
          ctx.fillStyle = 'red';
          ctx.font = '14px Arial';
          ctx.fillText('Game Over', 30, 75);
          setTimeout(() => {
            snake.splice(0, snake.length, {x: 75, y: 75});
            gameFlag.value = false;
          }, 2000);
          return;
        }

        snake.unshift(head);
        snake.pop();
        drawSnake(ctx, snake);
      }

      function fetchInputs() {
        Promise.all([
          fetch('/joystick').then(r => r.text()),
          fetch('/button').then(r => r.text())
        ]).then(([joy, btn]) => {
          document.getElementById('joystick').innerText = joy;
          document.getElementById('button').innerText = btn;

          if (joy.includes('LEFT')) joyDir = 'LEFT';
          if (joy.includes('RIGHT')) joyDir = 'RIGHT';
          if (joy.includes('UP')) joyDir = 'UP';
          if (joy.includes('DOWN')) joyDir = 'DOWN';

          if (btn.includes('LEFT')) btnDir = 'LEFT';
          if (btn.includes('RIGHT')) btnDir = 'RIGHT';
          if (btn.includes('UP')) btnDir = 'UP';
          if (btn.includes('DOWN')) btnDir = 'DOWN';
        });
      }

      setInterval(() => moveSnake(joySnake, joyDir, joyCtx, {
        get value() { return joyGameOver; },
        set value(v) { joyGameOver = v; }
      }), 200);

      setInterval(() => moveSnake(btnSnake, btnDir, btnCtx, {
        get value() { return btnGameOver; },
        set value(v) { btnGameOver = v; }
      }), 200);

      setInterval(fetchInputs, 100);
      drawSnake(joyCtx, joySnake);
      drawSnake(btnCtx, btnSnake);
    )rawliteral";
    request->send(200, "application/javascript", js);
  });

  server.onNotFound(notFound);
  server.begin();
}

void loop() {
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();

    if (receivedData.length() > 0) {
      Serial.print("Received: ");
      Serial.println(receivedData);

      if (receivedData.indexOf("(button)") >= 0) {
        latestButton = receivedData;
      } else {
        latestJoystick = receivedData;
      }

      // === Update OLED ===
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Joystick_Controller");
      display.print("IP: ");
      display.println(WiFi.softAPIP());
      display.setCursor(0, 32);
      display.print("Joy: ");
      display.println(latestJoystick);
      display.setCursor(0, 50);
      display.print("Btn: ");
      display.println(latestButton);
      display.display();
    }
  }
}
