#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h> // Libreria per la gestione del tempo

// --- CONFIGURAZIONE RETE ---
const char* ssid     = "Dj";
const char* password = "12345678";

ESP8266WebServer server(80);

// --- VARIABILI GLOBALI ---
String stato_robot = "PATTUGLIAMENTO";

const int MAX_LOGS = 10;
String registro_log[MAX_LOGS];
int indice_testa = 0;
int log_totali = 0;

// =============================================================
// FUNZIONE CHE GENERA LA PAGINA WEB (AJAX)
// =============================================================
void gestisciPaginaWeb() {
  String html = "<!DOCTYPE html><html lang='it'><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Controllo Robot</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f9; color: #333; }";
  html += ".card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); margin-bottom: 20px;}";
  html += "h1 { color: #0056b3; font-size: 24px; }";
  html += "h2 { font-size: 18px; margin-bottom: 10px; }";
  html += ".status { font-size: 20px; font-weight: bold; padding: 10px; border-radius: 5px; display: inline-block; }";
  html += ".safe { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
  html += ".danger { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
  html += "ul { list-style-type: none; padding: 0; }";
  /* Il font monospace rende i log allineati e professionali */
  html += "li { background: #e9ecef; margin-bottom: 5px; padding: 10px; border-radius: 4px; border-left: 4px solid #0056b3; font-family: 'Courier New', monospace; font-size: 14px; }";
  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "<h1>Pannello di Controllo Robot</h1>";
  html += "<h2>Stato Attuale:</h2>";
  html += "<div id='box-stato' class='status safe'>In attesa di dati...</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>Registro Eventi:</h2>";
  html += "<ul id='lista-log'><li>Sincronizzazione orologio di sistema...</li></ul>";
  html += "</div>";

  html += "<script>";
  html += "function aggiorna() {";
  html += "  fetch('/dati').then(r => r.json()).then(d => {";
  html += "    const box = document.getElementById('box-stato');";
  html += "    box.innerText = d.stato;";
  html += "    box.className = (d.stato === 'ALLARME') ? 'status danger' : 'status safe';";
  
  html += "    const list = document.getElementById('lista-log');";
  html += "    list.innerHTML = '';";
  html += "    if(d.log.length === 0) {";
  html += "      list.innerHTML = '<li>Nessun evento registrato.</li>';";
  html += "    } else {";
  html += "      d.log.forEach(item => { list.innerHTML += '<li>' + item + '</li>'; });";
  html += "    }";
  html += "  }).catch(e => console.log('Errore di rete'));";
  html += "}";
  
  html += "aggiorna();";
  html += "setInterval(aggiorna, 1000);";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// =============================================================
// ENDPOINT JSON
// =============================================================
void gestisciDati() {
  String json = "{";
  json += "\"stato\":\"" + stato_robot + "\",";
  json += "\"log\":[";

  // Calcola l'indice del log più recente in assoluto (la "coda" effettiva del buffer)
  // Se indice_testa è 0, l'ultimo elemento è in fondo all'array (MAX_LOGS - 1)
  int ultimo_inserito = (indice_testa - 1 + MAX_LOGS) % MAX_LOGS;

  for (int i = 0; i < log_totali; i++) {
    // Sottraendo 'i', camminiamo all'indietro nella timeline (dal più nuovo al più vecchio)
    int indice_reale = (ultimo_inserito - i + MAX_LOGS) % MAX_LOGS;
    
    if (i > 0) json += ",";
    json += "\"" + registro_log[indice_reale] + "\"";
  }

  json += "]}";
  server.send(200, "application/json", json);
}

// =============================================================
// FUNZIONE PER AGGIUNGERE UN LOG CON TIMESTAMP
// =============================================================
void aggiungiLog(String messaggio) {
  time_t now = time(nullptr);
  char orario[16];
  
  // Se l'orario è stato sincronizzato (maggiore dell'anno 2000 in epoch)
  if (now > 1000000000) {
    struct tm* timeinfo = localtime(&now);
    // Formatta come [HH:MM:SS]
    snprintf(orario, sizeof(orario), "[%02d:%02d:%02d] ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  } else {
    // Se internet non ha ancora risposto all'avvio
    snprintf(orario, sizeof(orario), "[SYS_SYNC] ");
  }

  // Concatena il tempo e il messaggio
  String log_finale = String(orario) + messaggio;

  registro_log[indice_testa] = log_finale;
  indice_testa = (indice_testa + 1) % MAX_LOGS;
  if (log_totali < MAX_LOGS) log_totali++;
}

// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // --- CONFIGURAZIONE SERVER NTP (ORARIO ITALIANO) ---
  // "CET-1CEST,M3.5.0,M10.5.0/3" gestisce automaticamente Ora Solare/Legale italiana
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");

  server.on("/", gestisciPaginaWeb);
  server.on("/dati", gestisciDati);
  server.begin();
  
  // Aggiungiamo un log iniziale autogenerato per confermare l'avvio
  aggiungiLog("Web Server inizializzato e in ascolto");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
  server.handleClient();

  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.startsWith("STATO:")) {
      stato_robot = comando.substring(6);
    }
    else if (comando.startsWith("LOG:")) {
      aggiungiLog(comando.substring(4));
    }
  }
}