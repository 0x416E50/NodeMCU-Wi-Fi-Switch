#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Define the GPIO pins for the relays
#define RELAY1 5  // D1
#define RELAY2 4  // D2
#define RELAY3 14 // D5
#define RELAY4 12 // D6

// Create an instance of the web server running on port 80
ESP8266WebServer server(80);

// Variables to store the state of the relays
bool relay1State = HIGH;
bool relay2State = HIGH;
bool relay3State = HIGH;
bool relay4State = HIGH;

// Addresses in EEPROM where states will be stored
#define EEPROM_RELAY1 0
#define EEPROM_RELAY2 1
#define EEPROM_RELAY3 2
#define EEPROM_RELAY4 3

// Admin credentials
const char* adminUsername = "admin";
const char* adminPassword = "1234";

// Variable to track if the user is logged in
bool loggedIn = false;

void setup() {
  // Initialize the GPIO pins as outputs
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);

  // Start with all relays turned on
  digitalWrite(RELAY1, relay1State);
  digitalWrite(RELAY2, relay2State);
  digitalWrite(RELAY3, relay3State);
  digitalWrite(RELAY4, relay4State);

  // Start the Serial communication
  Serial.begin(115200);
  
  // Set up the Wi-Fi as an Access Point (no password)
  WiFi.softAP("Relay Control");
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize EEPROM
  EEPROM.begin(512);

  // Load relay states from EEPROM
  relay1State = EEPROM.read(EEPROM_RELAY1) == HIGH ? HIGH : LOW;
  relay2State = EEPROM.read(EEPROM_RELAY2) == HIGH ? HIGH : LOW;
  relay3State = EEPROM.read(EEPROM_RELAY3) == HIGH ? HIGH : LOW;
  relay4State = EEPROM.read(EEPROM_RELAY4) == HIGH ? HIGH : LOW;

  // Set relay states
  digitalWrite(RELAY1, relay1State);
  digitalWrite(RELAY2, relay2State);
  digitalWrite(RELAY3, relay3State);
  digitalWrite(RELAY4, relay4State);

  // Define the handling functions for the web server
  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/loginSubmit", handleLoginSubmit);
  server.on("/relay1", handleRelay1);
  server.on("/relay2", handleRelay2);
  server.on("/relay3", handleRelay3);
  server.on("/relay4", handleRelay4);
  server.on("/logout", handleLogout);

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}

// Function to handle the root (control) page
void handleRoot() {
  if (!loggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/html", "");
    return;
  }

  String html = "<html><head><title>Control Panel</title>";
  html += "<style>";
  html += "body { background-color: rgb(154,215,203); color: black; font-family: Arial, sans-serif; }";
  html += "div { text-align: center; }";
  html += "button { width: 120px; height: 60px; font-size: 24px; font-weight: bold; border-radius: 15px; border: none; cursor: pointer; margin: 10px; }";
  html += ".on { background-color: lightcoral; }";
  html += ".off { background-color: lightgreen; }";
  html += "</style>";
  html += "</head><body><div>";
  html += "<h1>Device Status Control</h1><br><br><br>";
  
  // Button styles based on relay state
  String relay1Class = (relay1State == HIGH) ? "on" : "off";
  String relay2Class = (relay2State == HIGH) ? "on" : "off";
  String relay3Class = (relay3State == HIGH) ? "on" : "off";
  String relay4Class = (relay4State == HIGH) ? "on" : "off";

  html += "<button onclick=\"location.href='/relay1'\" class=\"" + relay1Class + "\">Relay 1</button>";
  html += "<button onclick=\"location.href='/relay2'\" class=\"" + relay2Class + "\">Relay 2</button><br>";
  html += "<button onclick=\"location.href='/relay3'\" class=\"" + relay3Class + "\">Relay 3</button>";
  html += "<button onclick=\"location.href='/relay4'\" class=\"" + relay4Class + "\">Relay 4</button>";
  html += "<br><button onclick=\"location.href='/logout'\">Logout</button>";
  
  html += "<br><br><br><p><i>Made by 0x416E50</i></p>"; // Added line
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// Function to handle the login page
void handleLogin() {
  // Determine the relay status for display (inverted)
  String relay1Status = (relay1State == HIGH) ? "OFF" : "ON";
  String relay2Status = (relay2State == HIGH) ? "OFF" : "ON";
  String relay3Status = (relay3State == HIGH) ? "OFF" : "ON";
  String relay4Status = (relay4State == HIGH) ? "OFF" : "ON";

  String html = "<html><head><title>Login</title>";
  html += "<style>";
  html += "body { background-color: rgb(154,215,203); color: black; font-family: Arial, sans-serif; }";
  html += "div { text-align: center; margin-top: 50px; }";
  html += "input[type=text], input[type=password] {";
  html += "  border-radius: 15px; border: 2px solid #ccc; padding: 10px; font-size: 18px; width: 250px; margin-bottom: 10px;";
  html += "}";
  html += "button {";
  html += "  width: 150px; height: 50px; font-size: 18px; font-weight: bold; border-radius: 15px; border: none; cursor: pointer; background-color: rgb(288,145,145); color: white;";
  html += "}";
  html += "p { color: red; }";
  html += "table { margin: 20px auto; border-collapse: collapse; }";
  html += "td, th { padding: 10px; border: 1px solid #ddd; }";
  html += "th { background-color: #f2f2f2; }";
  html += "</style>";
  html += "</head><body><div>";

  html += "<h1>System Login</h1><br>";
  html += "<form action=\"/loginSubmit\" method=\"POST\">";
  html += "<input type=\"text\" name=\"username\" placeholder=\"Username\" required><br>";
  html += "<input type=\"password\" name=\"password\" placeholder=\"Password\" required><br>";
  html += "<button type=\"submit\">Login</button>";
  html += "</form>";

  // Display error message if login failed
  if (server.hasArg("error")) {
    html += "<p>Incorrect username or password, try again.</p>";
  }

  // Relay status table
  html += "<h2>Current Relay Status</h2>";
  html += "<table><tr><th>Relay</th><th>Status</th></tr>";
  html += "<tr><td>Relay 1</td><td>" + relay1Status + "</td></tr>";
  html += "<tr><td>Relay 2</td><td>" + relay2Status + "</td></tr>";
  html += "<tr><td>Relay 3</td><td>" + relay3Status + "</td></tr>";
  html += "<tr><td>Relay 4</td><td>" + relay4Status + "</td></tr>";
  html += "</table>";

  html += "<br><br><br><p><i>Made by 0x416E50</i></p>"; // Added line

  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// Function to handle the login form submission
void handleLoginSubmit() {
  String username = server.arg("username");
  String password = server.arg("password");

  if (username == adminUsername && password == adminPassword) {
    loggedIn = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/html", "");
  } else {
    server.sendHeader("Location", "/login?error=1");
    server.send(302, "text/html", "");
  }
}

// Function to handle logout
void handleLogout() {
  loggedIn = false;
  server.sendHeader("Location", "/login");
  server.send(302, "text/html", "");
}

// Functions to handle the relay control URLs
void handleRelay1() {
  if (!loggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/html", "");
    return;
  }
  relay1State = !relay1State;
  digitalWrite(RELAY1, relay1State);
  EEPROM.write(EEPROM_RELAY1, relay1State); // Store the state in EEPROM
  EEPROM.commit(); // Commit the changes to EEPROM
  handleRoot();
}

void handleRelay2() {
  if (!loggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/html", "");
    return;
  }
  relay2State = !relay2State;
  digitalWrite(RELAY2, relay2State);
  EEPROM.write(EEPROM_RELAY2, relay2State); // Store the state in EEPROM
  EEPROM.commit(); // Commit the changes to EEPROM
  handleRoot();
}

void handleRelay3() {
  if (!loggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/html", "");
    return;
  }
  relay3State = !relay3State;
  digitalWrite(RELAY3, relay3State);
  EEPROM.write(EEPROM_RELAY3, relay3State); // Store the state in EEPROM
  EEPROM.commit(); // Commit the changes to EEPROM
  handleRoot();
}

void handleRelay4() {
  if (!loggedIn) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/html", "");
    return;
  }
  relay4State = !relay4State;
  digitalWrite(RELAY4, relay4State);
  EEPROM.write(EEPROM_RELAY4, relay4State); // Store the state in EEPROM
  EEPROM.commit(); // Commit the changes to EEPROM
  handleRoot();
}