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

# UDP discovery: la ESP encuentra la IP de la PC aunque cambie en el hotspot.
DISCOVERY_PORT = 4210
DISCOVERY_MAGIC = b"BRAZO_SRV"
DISCOVERY_WHO = b"BRAZO_WHO"
BEACON_INTERVAL_S = 2.0

MEGA_CMD_MAP = {
    0: "Inicial",
    1: "Trabajo",
    2: "Trabajo 2",
    3: "Standby",
    4: "Start / Check",
    5: "Clear errors",
}

CMD_LABELS = {i: (MEGA_CMD_MAP[i] if i in MEGA_CMD_MAP else f"Cmd {i}") for i in range(16)}

HTML = """<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Control Brazo Cobot</title>
  <style>
    :root {
      --bg: #0b0f14;
      --panel: #141c27;
      --panel2: #1a2433;
      --line: #2a3544;
      --text: #e8eef7;
      --muted: #8b98a5;
      --accent: #1d9bf0;
      --accent2: #0ea5e9;
      --ok: #34d399;
      --warn: #fbbf24;
      --err: #f87171;
      --slot: #182230;
    }
    * { box-sizing: border-box; }
    body {
      font-family: "Segoe UI", system-ui, sans-serif;
      margin: 0;
      background: radial-gradient(circle at top, #152033 0%, var(--bg) 55%);
      color: var(--text);
      min-height: 100vh;
    }
    .wrap {
      max-width: 920px;
      margin: 0 auto;
      padding: 1.25rem 1rem 2rem;
    }
    header {
      display: flex;
      flex-wrap: wrap;
      align-items: flex-end;
      justify-content: space-between;
      gap: 1rem;
      margin-bottom: 1rem;
    }
    h1 { font-size: 1.35rem; margin: 0; }
    .sub { color: var(--muted); font-size: 0.92rem; margin: 0.25rem 0 0; }
    .badges { display: flex; gap: 0.5rem; flex-wrap: wrap; }
    .badge {
      font-size: 0.78rem;
      padding: 0.35rem 0.65rem;
      border-radius: 999px;
      background: var(--panel2);
      border: 1px solid var(--line);
      color: var(--muted);
    }
    .badge.live { color: var(--ok); border-color: #1f6f52; background: #10241c; }
    .badge.idle { color: var(--muted); }
    .badge.alert { color: var(--err); border-color: #7f1d1d; background: #2a1212; }

    .grid-15 {
      display: grid;
      grid-template-columns: repeat(5, minmax(0, 1fr));
      gap: 0.65rem;
      margin: 1rem 0;
    }
    @media (max-width: 720px) {
      .grid-15 { grid-template-columns: repeat(3, minmax(0, 1fr)); }
    }
    .cmd-btn {
      display: flex;
      flex-direction: column;
      align-items: stretch;
      min-height: 74px;
      padding: 0.55rem 0.5rem 0.45rem;
      border-radius: 12px;
      border: 1px solid var(--line);
      background: linear-gradient(180deg, var(--slot) 0%, #121820 100%);
      color: var(--text);
      cursor: pointer;
      transition: transform 0.08s ease, border-color 0.15s ease, box-shadow 0.15s ease;
      text-align: left;
    }
    .cmd-btn:hover {
      transform: translateY(-1px);
      border-color: #3d5168;
      box-shadow: 0 8px 24px rgba(0,0,0,0.25);
    }
    .cmd-btn:active { transform: translateY(0); }
    .cmd-btn.active {
      border-color: var(--accent);
      box-shadow: 0 0 0 1px rgba(29,155,240,0.35);
    }
    .cmd-btn.named {
      background: linear-gradient(180deg, #15283a 0%, #101820 100%);
    }
    .cmd-num {
      font-size: 0.72rem;
      color: var(--muted);
      letter-spacing: 0.04em;
      text-transform: uppercase;
    }
    .cmd-label {
      font-size: 0.92rem;
      font-weight: 700;
      margin-top: 0.15rem;
      line-height: 1.15;
    }
    .cmd-bits {
      font-size: 0.72rem;
      color: var(--accent2);
      margin-top: 0.25rem;
      font-family: ui-monospace, monospace;
    }

    .panel {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 12px;
      padding: 0.9rem 1rem;
      margin-top: 0.85rem;
    }
    .panel h2 {
      font-size: 0.82rem;
      text-transform: uppercase;
      letter-spacing: 0.06em;
      color: var(--muted);
      margin: 0 0 0.55rem;
    }
    .status-line { font-size: 0.95rem; line-height: 1.45; }
    .ok { color: var(--ok); }
    .err { color: var(--err); }
    .warn { color: var(--warn); }
    .muted { color: var(--muted); }

    table { width: 100%; border-collapse: collapse; font-size: 0.84rem; }
    th, td {
      text-align: left;
      padding: 0.38rem 0.45rem;
      border-bottom: 1px solid var(--line);
    }
    .hi { color: var(--ok); font-weight: 700; }
    .lo { color: var(--muted); }
    code {
      background: #0a1018;
      padding: 0.12rem 0.35rem;
      border-radius: 4px;
      font-size: 0.85em;
    }
    .bits-row {
      display: flex;
      gap: 0.35rem;
      flex-wrap: wrap;
      margin-top: 0.5rem;
    }
    .bit {
      width: 2rem;
      text-align: center;
      padding: 0.35rem 0;
      border-radius: 8px;
      font-size: 0.75rem;
      border: 1px solid var(--line);
      background: #0f141c;
      color: var(--muted);
    }
    .bit.on {
      background: #123047;
      border-color: #2563eb;
      color: #bfdbfe;
      font-weight: 700;
    }
  </style>
</head>
<body>
  <div class="wrap">
    <header>
      <div>
        <h1>Brazo HerkuleX · Panel de control</h1>
        <p class="sub">Servidor → ESP8266 → Mega · 4 bits (D5–D8) + latch D3→44</p>
      </div>
      <div class="badges">
        <span id="espBadge" class="badge idle">ESP sin datos</span>
        <span id="megaBadge" class="badge idle">Mega sin respuesta</span>
      </div>
    </header>

    <div class="grid-15" id="cmdGrid"></div>

    <div id="status" class="panel">
      <h2>Estado</h2>
      <div class="status-line muted">Seleccioná un comando (0–15).</div>
    </div>

    <div class="panel">
      <h2>Mapa de bits → Mega</h2>
      <div class="muted" style="font-size:0.88rem;margin-bottom:0.35rem;">
        Comando N se envía como valor binario en D5 (bit0) … D8 (bit3).
      </div>
      <div id="bitPreview" class="bits-row"></div>
    </div>

    <div class="panel">
      <h2>Último reporte ESP</h2>
      <div id="espReport" class="muted">Esperando reporte...</div>
    </div>

    <div class="panel">
      <h2>Respuesta Mega (Serial2 → ESP)</h2>
      <div id="megaReport" class="muted">Esperando OK/ERR del Mega...</div>
    </div>
  </div>

  <script>
    const CMD_LABELS = {
      0: "Inicial", 1: "Trabajo", 2: "Trabajo 2", 3: "Standby",
      4: "Start / Check", 5: "Clear errors"
    };
    let lastSent = null;

    function labelFor(cmd) {
      return CMD_LABELS[cmd] || ("Reservado " + cmd);
    }

    function bitsString(cmd) {
      return cmd.toString(2).padStart(4, "0");
    }

    function buildGrid() {
      const grid = document.getElementById("cmdGrid");
      for (let cmd = 0; cmd <= 15; cmd++) {
        const btn = document.createElement("button");
        btn.className = "cmd-btn" + (cmd <= 5 ? " named" : "");
        btn.dataset.cmd = cmd;
        btn.innerHTML =
          '<span class="cmd-num">CMD ' + cmd + '</span>' +
          '<span class="cmd-label">' + labelFor(cmd) + '</span>' +
          '<span class="cmd-bits">' + bitsString(cmd) + 'b</span>';
        btn.onclick = () => send(cmd, btn);
        grid.appendChild(btn);
      }
      renderBitPreview(null);
    }

    function renderBitPreview(cmd) {
      const row = document.getElementById("bitPreview");
      row.innerHTML = "";
      if (cmd === null) {
        row.innerHTML = '<span class="muted">Pulsá un botón para ver los bits.</span>';
        return;
      }
      const bits = bitsString(cmd);
      for (let i = 0; i < 4; i++) {
        const el = document.createElement("div");
        el.className = "bit" + (bits[3 - i] === "1" ? " on" : "");
        el.textContent = "b" + i;
        row.appendChild(el);
      }
      const info = document.createElement("div");
      info.className = "muted";
      info.style.width = "100%";
      info.style.marginTop = "0.35rem";
      info.textContent = "Valor " + cmd + " → bits " + bits;
      row.appendChild(info);
    }

    async function send(cmd, btn) {
      lastSent = cmd;
      renderBitPreview(cmd);
      document.querySelectorAll(".cmd-btn").forEach(b => b.classList.remove("active"));
      if (btn) btn.classList.add("active");

      const el = document.getElementById("status");
      el.innerHTML = '<h2>Estado</h2><div class="status-line">Enviando CMD <strong>' + cmd + '</strong> · ' + labelFor(cmd) + '...</div>';

      try {
        const r = await fetch("/api/command", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ cmd })
        });
        const j = await r.json();
        if (j.ok) {
          el.innerHTML =
            '<h2>Estado</h2><div class="status-line ok">CMD <strong>' + cmd + '</strong> en cola. La ESP lo tomará en el próximo poll.</div>';
        } else {
          el.innerHTML =
            '<h2>Estado</h2><div class="status-line err">Error: ' + (j.error || "desconocido") + '</div>';
        }
      } catch (e) {
        el.innerHTML =
          '<h2>Estado</h2><div class="status-line err">Error de red: ' + e.message + '</div>';
      }
    }

    function levelCell(v) {
      return v ? '<span class="hi">HIGH</span>' : '<span class="lo">LOW</span>';
    }

    async function refreshStatus() {
      try {
        const r = await fetch("/api/status");
        const j = await r.json();

        const espBadge = document.getElementById("espBadge");
        if (j.last_report && j.age_sec < 8) {
          espBadge.className = "badge live";
          espBadge.textContent = "ESP activa · hace " + Math.round(j.age_sec) + "s";
        } else {
          espBadge.className = "badge idle";
          espBadge.textContent = "ESP sin datos recientes";
        }

        const megaBadge = document.getElementById("megaBadge");
        if (j.last_mega_report) {
          const ok = (j.last_mega_report.line || "").startsWith("OK:");
          const overload = (j.last_mega_report.line || "").startsWith("ERR:");
          megaBadge.className = "badge " + (overload ? "alert" : (ok ? "live" : "idle"));
          megaBadge.textContent = ok ? "Mega OK" : (overload ? "Mega · sobrecarga/ERR" : "Mega respondió");
        } else {
          megaBadge.className = "badge idle";
          megaBadge.textContent = "Mega sin respuesta";
        }

        const box = document.getElementById("espReport");
        if (!j.last_report) {
          box.innerHTML = "Aún no hay reporte. Enviá un comando cuando la ESP esté conectada.";
        } else {
          const rep = j.last_report;
          let rows = "";
          (rep.pins || []).forEach((p, i) => {
            rows += '<tr><td>bit ' + i + '</td><td>' + p.esp + ' (GPIO ' + p.gpio + ')</td>'
              + '<td>' + levelCell(p.level) + '</td><td>Mega ' + p.mega + '</td></tr>';
          });
          const latch = rep.latch || {};
          rows += '<tr><td>latch</td><td>' + (latch.esp || "?") + '</td>'
            + '<td>pulso</td><td>Mega ' + (latch.mega || "?") + '</td></tr>';
          box.innerHTML =
            '<div><strong>CMD:</strong> <code>' + rep.cmd + '</code> · '
            + labelFor(Number(rep.cmd)) + ' · <strong>bits:</strong> <code>' + (rep.bits || "?") + '</code></div>'
            + '<div class="muted" style="margin-top:0.35rem;">' + (rep.mega_expected || "") + '</div>'
            + '<div class="muted">IP ESP: ' + (rep.esp_ip || "?") + '</div>'
            + '<table style="margin-top:0.5rem;"><tr><th>Bit</th><th>ESP</th><th>Nivel</th><th>Mega</th></tr>' + rows + '</table>';
        }

        const megaBox = document.getElementById("megaReport");
        if (j.last_mega_report) {
          const m = j.last_mega_report;
          const line = m.line || "";
          const ok = line.startsWith("OK:");
          const cls = ok ? "ok" : "err";
          let hint = ok ? "Movimiento completado." : "Posible aborto por sobrecarga o error.";
          megaBox.innerHTML =
            '<div class="' + cls + '"><strong>' + line + '</strong></div>'
            + '<div class="muted" style="margin-top:0.35rem;">' + hint + ' · hace ' + Math.round(j.mega_age_sec) + ' s</div>';
        } else {
          megaBox.innerHTML = "Sin respuesta del Mega (¿TX2 pin16 → ESP D1 con divisor?).";
        }
      } catch (e) {
        console.error(e);
      }
    }

    buildGrid();
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


def discovery_beacon(http_ip: str, http_port: int, stop: threading.Event) -> None:
    """Anuncia periodicamente la IP/puerto HTTP y responde a BRAZO_WHO."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    try:
        sock.bind(("", DISCOVERY_PORT))
    except OSError as exc:
        print(f"  AVISO: no se pudo abrir discovery UDP :{DISCOVERY_PORT} ({exc})")
        sock.close()
        return
    sock.settimeout(0.5)
    print(f"  Discovery UDP: puerto {DISCOVERY_PORT} (beacon cada {BEACON_INTERVAL_S:.0f}s)")
    next_beacon = 0.0
    announced_ip = http_ip
    while not stop.is_set():
        now = time.time()
        if now >= next_beacon:
            # Releer IP por si el hotspot reasignó la de la PC.
            announced_ip = local_ip()
            msg = f"BRAZO_SRV {announced_ip} {http_port}".encode("ascii")
            try:
                sock.sendto(msg, ("255.255.255.255", DISCOVERY_PORT))
            except OSError:
                pass
            next_beacon = now + BEACON_INTERVAL_S
        try:
            data, addr = sock.recvfrom(256)
            if data.startswith(DISCOVERY_WHO):
                msg = f"BRAZO_SRV {announced_ip} {http_port}".encode("ascii")
                try:
                    sock.sendto(msg, addr)
                    print(f"  ↔ Discovery: respondí a {addr[0]}")
                except OSError:
                    pass
        except socket.timeout:
            pass
        except OSError:
            break
    sock.close()


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
    stop = threading.Event()
    beacon = threading.Thread(
        target=discovery_beacon,
        args=(ip, port, stop),
        daemon=True,
        name="discovery-beacon",
    )
    beacon.start()

    print("=" * 50)
    print("Servidor de control del brazo")
    if port != args.port:
        print(f"  AVISO: puerto {args.port} ocupado, usando {port}")
    print(f"  Interfaz web:  http://{ip}:{port}/")
    print(f"  Poll ESP:      GET  http://{ip}:{port}/api/poll")
    print(f"  Reporte ESP:   POST http://{ip}:{port}/api/esp/report")
    print(f"  Discovery UDP: BRAZO_SRV {ip} {port}  (puerto {DISCOVERY_PORT})")
    print(f'  Fallback ESP:  SERVER_HOST="{ip}"  SERVER_PORT={port}')
    print("=" * 50)

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nCerrando servidor.")
        stop.set()
        server.server_close()


if __name__ == "__main__":
    main()
