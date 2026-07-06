#!/usr/bin/env python3
"""Servidor web + feedback ESP para control del brazo."""

from __future__ import annotations

import argparse
import json
import socket
import threading
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import parse_qs

pending_cmd: int | None = None
last_esp_report: dict | None = None
last_mega_report: dict | None = None
lock = threading.Lock()

MEGA_CMD_MAP = {
    0: "POSE_INICIAL",
    1: "POSE_TRABAJO",
    2: "POSE_TRABAJO_2",
    3: "POSE_STANDBY",
}

HTML = """<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Control Brazo Cobot</title>
  <style>
    * { box-sizing: border-box; }
    body {
      font-family: system-ui, sans-serif;
      max-width: 520px;
      margin: 2rem auto;
      padding: 0 1rem;
      background: #0f1419;
      color: #e7ecf3;
    }
    h1 { font-size: 1.25rem; margin-bottom: 0.25rem; }
    h2 { font-size: 1rem; margin: 1.5rem 0 0.5rem; color: #8b98a5; }
    p.sub { color: #8b98a5; font-size: 0.9rem; margin-top: 0; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 0.75rem; margin-top: 1rem; }
    button {
      padding: 1rem; font-size: 1rem; border: none; border-radius: 10px;
      cursor: pointer; background: #1d9bf0; color: white; font-weight: 600;
    }
    button:hover { background: #1a8cd8; }
    .panel {
      margin-top: 1rem; padding: 0.85rem 1rem; border-radius: 8px;
      background: #1a2332; font-size: 0.88rem; line-height: 1.5;
    }
    table { width: 100%; border-collapse: collapse; margin-top: 0.5rem; font-size: 0.85rem; }
    th, td { text-align: left; padding: 0.35rem 0.5rem; border-bottom: 1px solid #2a3544; }
    .hi { color: #6ee7a0; font-weight: 700; }
    .lo { color: #8b98a5; }
    .ok { color: #6ee7a0; }
    .err { color: #f87171; }
    .muted { color: #8b98a5; }
    code { background: #0f1419; padding: 0.1rem 0.35rem; border-radius: 4px; }
  </style>
</head>
<body>
  <h1>Brazo HerkuleX</h1>
  <p class="sub">Comandos vía servidor → ESP → Mega (bus paralelo)</p>
  <div class="grid">
    <button onclick="send(0)">0 · Inicial</button>
    <button onclick="send(1)">1 · Trabajo</button>
    <button onclick="send(2)">2 · Trabajo 2</button>
    <button onclick="send(3)">3 · Standby</button>
  </div>
  <div id="status" class="panel">Listo.</div>

  <h2>Mapa comando → pines ESP → Mega</h2>
  <div class="panel muted">
    Bit 0 (LSB) = D5, bit 1 = D6, bit 2 = D7, bit 3 = D8. Latch = D3 → Mega 44.
  </div>

  <h2>Último reporte de la ESP</h2>
  <div id="espReport" class="panel muted">Esperando reporte...</div>

  <h2>Respuesta del Mega (serial)</h2>
  <div id="megaReport" class="panel muted">Esperando OK del Mega...</div>

  <script>
    async function send(cmd) {
      const el = document.getElementById('status');
      el.textContent = 'Enviando comando ' + cmd + '...';
      el.className = 'panel';
      try {
        const r = await fetch('/api/command', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ cmd })
        });
        const j = await r.json();
        if (j.ok) {
          el.textContent = 'Comando ' + cmd + ' en cola. La ESP lo tomará en el próximo poll.';
          el.className = 'panel ok';
        } else {
          el.textContent = 'Error: ' + (j.error || 'desconocido');
          el.className = 'panel err';
        }
      } catch (e) {
        el.textContent = 'Error de red: ' + e.message;
        el.className = 'panel err';
      }
    }

    function levelCell(v) {
      return v ? '<span class="hi">HIGH</span>' : '<span class="lo">LOW</span>';
    }

    async function refreshStatus() {
      try {
        const r = await fetch('/api/status');
        const j = await r.json();
        const box = document.getElementById('espReport');
        if (!j.last_report) {
          box.innerHTML = 'Aún no hay reporte. Pulsá un botón cuando la ESP esté conectada.';
        } else {
          const rep = j.last_report;
          let rows = '';
          (rep.pins || []).forEach((p, i) => {
            rows += '<tr><td>bit ' + i + '</td><td>' + p.esp + ' (GPIO ' + p.gpio + ')</td>'
              + '<td>' + levelCell(p.level) + '</td><td>Mega ' + p.mega + '</td></tr>';
          });
          const latch = rep.latch || {};
          rows += '<tr><td>latch</td><td>' + (latch.esp || '?') + ' (GPIO ' + (latch.gpio || '?') + ')</td>'
            + '<td>pulso LOW</td><td>Mega ' + (latch.mega || '?') + '</td></tr>';
          box.innerHTML =
            '<div><strong>Comando:</strong> <code>' + rep.cmd + '</code> &nbsp; '
            + '<strong>Bits:</strong> <code>' + rep.bits + '</code></div>'
            + '<div><strong>Mega debería:</strong> ' + rep.mega_expected + '</div>'
            + '<div class="muted">Hace ' + Math.round(j.age_sec) + ' s · IP ESP: ' + (rep.esp_ip || '?') + '</div>'
            + '<table><tr><th>Bit</th><th>ESP</th><th>Nivel</th><th>Mega pin</th></tr>' + rows + '</table>';
        }

        const megaBox = document.getElementById('megaReport');
        if (j.last_mega_report) {
          const m = j.last_mega_report;
          const ok = (m.line || '').startsWith('OK:');
          megaBox.innerHTML =
            '<div class="' + (ok ? 'ok' : 'err') + '"><strong>' + m.line + '</strong></div>'
            + '<div class="muted">Hace ' + Math.round(j.mega_age_sec) + ' s</div>';
        } else {
          megaBox.innerHTML = 'Sin respuesta del Mega aún (¿cable TX2→D1 con divisor?).';
        }
      } catch (e) {
        console.error(e);
      }
    }

    setInterval(refreshStatus, 1000);
    refreshStatus();
  </script>
</body>
</html>
"""


def local_ip() -> str:
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except OSError:
        return "127.0.0.1"


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt: str, *args) -> None:
        print(f"[{self.address_string()}] {fmt % args}")

    def _send(self, code: int, body: str, content_type: str = "text/plain") -> None:
        data = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _send_json(self, code: int, obj: dict) -> None:
        self._send(code, json.dumps(obj), "application/json")

    def _read_json(self) -> dict | None:
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length).decode("utf-8") if length else ""
        if not raw.strip():
            return None
        try:
            return json.loads(raw)
        except json.JSONDecodeError:
            return None

    def do_GET(self) -> None:
        if self.path in ("/", "/index.html"):
            self._send(200, HTML, "text/html; charset=utf-8")
            return

        if self.path == "/api/poll":
            global pending_cmd
            with lock:
                cmd = pending_cmd
                pending_cmd = None
            if cmd is None:
                self._send(200, "-1")
            else:
                print(f"  → ESP tomó comando {cmd}")
                self._send(200, str(cmd))
            return

        if self.path == "/api/status":
            with lock:
                rep = dict(last_esp_report) if last_esp_report else None
                mega = dict(last_mega_report) if last_mega_report else None
            age = time.time() - rep["ts"] if rep else None
            mega_age = time.time() - mega["ts"] if mega else None
            self._send_json(200, {
                "last_report": rep,
                "age_sec": age,
                "last_mega_report": mega,
                "mega_age_sec": mega_age,
                "mega_cmd_map": MEGA_CMD_MAP,
            })
            return

        if self.path == "/api/pinmap":
            self._send_json(200, {
                "esp8266": {
                    "data": [
                        {"bit": 0, "esp": "D5", "gpio": 14, "mega": 22},
                        {"bit": 1, "esp": "D6", "gpio": 12, "mega": 24},
                        {"bit": 2, "esp": "D7", "gpio": 13, "mega": 26},
                        {"bit": 3, "esp": "D8", "gpio": 15, "mega": 28},
                    ],
                    "latch": {"esp": "D3", "gpio": 0, "mega": 44},
                },
                "mega_commands": MEGA_CMD_MAP,
            })
            return

        self._send(404, "not found")

    def do_POST(self) -> None:
        if self.path == "/api/command":
            data = self._read_json()
            cmd = int(data["cmd"]) if data and "cmd" in data else None
            if cmd is None or not 0 <= cmd <= 15:
                self._send_json(400, {"ok": False, "error": "cmd debe ser 0-15"})
                return
            global pending_cmd
            with lock:
                pending_cmd = cmd
            print(f"  ← Comando {cmd} encolado desde web")
            self._send_json(200, {"ok": True, "cmd": cmd})
            return

        if self.path == "/api/esp/report":
            data = self._read_json()
            if not data or "cmd" not in data:
                self._send_json(400, {"ok": False, "error": "JSON inválido"})
                return
            data["ts"] = time.time()
            data["esp_ip"] = self.client_address[0]
            global last_esp_report
            with lock:
                last_esp_report = data
            bits = data.get("bits", "?")
            mega = data.get("mega_expected", "?")
            print(f"  ↩ ESP reportó cmd={data['cmd']} bits={bits} → {mega}")
            self._send_json(200, {"ok": True})
            return

        if self.path == "/api/mega/report":
            data = self._read_json()
            if not data or "line" not in data:
                self._send_json(400, {"ok": False, "error": "JSON inválido"})
                return
            data["ts"] = time.time()
            global last_mega_report
            with lock:
                last_mega_report = data
            print(f"  ✓ Mega reportó: {data['line']}")
            self._send_json(200, {"ok": True})
            return

        self._send(404, "not found")


class ReusableHTTPServer(HTTPServer):
    allow_reuse_address = True


def bind_server(host: str, port: int) -> tuple[ReusableHTTPServer, int]:
    last_error: OSError | None = None
    for candidate in range(port, port + 20):
        try:
            return ReusableHTTPServer((host, candidate), Handler), candidate
        except OSError as exc:
            last_error = exc
            if exc.errno != 98:
                raise
    raise OSError(f"Puertos {port}-{port + 19} ocupados: {last_error}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Servidor de control para ESP polling")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()

    ip = local_ip()
    server, port = bind_server(args.host, args.port)

    print("=" * 50)
    print("Servidor de control del brazo")
    if port != args.port:
        print(f"  AVISO: puerto {args.port} ocupado, usando {port}")
    print(f"  Interfaz web:  http://{ip}:{port}/")
    print(f"  Poll ESP:      GET  http://{ip}:{port}/api/poll")
    print(f"  Reporte ESP:   POST http://{ip}:{port}/api/esp/report")
    print(f'  wifi_config.h: SERVER_HOST="{ip}"  SERVER_PORT={port}')
    print("=" * 50)

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nCerrando servidor.")
        server.server_close()


if __name__ == "__main__":
    main()
