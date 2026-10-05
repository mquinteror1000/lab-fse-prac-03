#!/usr/bin/env python3
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
import csv
import html
import datetime

PORT = 8000
LOG_FILE = (
    Path.home()
    / "practica03"
    / "actividad_1"
    / "lm75b_log.csv"
)

def read_log_file():
    """
    Leer el CSV y devolver una lista de lecturas.
    Cada lectura debe representarse como una tupla:
    (timestamp, temperatura).
    """
    readings = []
    
    if not LOG_FILE.exists():
        return readings
        
    try:
        with open(LOG_FILE, mode='r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                try:
                    timestamp = int(row['timestamp'])
                    temperature_c = float(row['temperature_c'])
                    readings.append((timestamp, temperature_c))
                except (ValueError, KeyError, TypeError):
                    # Ignorar filas incompletas o inválidas para evitar la caída del servidor
                    continue
    except OSError:
        pass
        
    return readings

def build_html(readings):
    """Construir una página HTML a partir de las lecturas."""
    if not readings:
        content = (
            "<h1>Monitor de temperatura LM75B</h1>\n"
            "<p>No hay lecturas disponibles.</p>"
        )
    else:
        temperatures = [r[1] for r in readings]
        total_samples = len(readings)
        last_timestamp, last_temp = readings[-1]
        min_temp = min(temperatures)
        max_temp = max(temperatures)
        avg_temp = sum(temperatures) / total_samples
        
        # Fecha y hora legible extra
        last_time_str = datetime.datetime.fromtimestamp(last_timestamp).strftime('%Y-%m-%d %H:%M:%S')

        content = (
            "<h1>Monitor de temperatura LM75B</h1>\n"
            "<div class='stats'>\n"
            f"<p><span class='label'>Última temperatura:</span> <span class='value highlight'>{last_temp:.3f} °C</span> <span class='time'>({last_time_str})</span></p>\n"
            f"<p><span class='label'>Total de muestras:</span> <span class='value'>{total_samples}</span></p>\n"
            f"<p><span class='label'>Temperatura mínima:</span> <span class='value'>{min_temp:.3f} °C</span></p>\n"
            f"<p><span class='label'>Temperatura máxima:</span> <span class='value'>{max_temp:.3f} °C</span></p>\n"
            f"<p><span class='label'>Temperatura promedio:</span> <span class='value'>{avg_temp:.3f} °C</span></p>\n"
            "</div>"
        )

    return f"""<!doctype html>
<html lang="es">
<head>
    <meta charset="utf-8">
    <meta http-equiv="refresh" content="5"> <!-- Actualización automática (Actividad extra) -->
    <title>Monitor LM75B</title>
    <style>
        body {{
            background-color: #282c34;
            color: #abb2bf;
            font-family: 'JetBrains Mono', 'Cascadia Code', monospace;
            padding: 40px;
            display: flex;
            flex-direction: column;
            align-items: center;
        }}
        h1 {{
            color: #e5c07b;
            border-bottom: 2px solid #61afef;
            padding-bottom: 10px;
        }}
        .stats {{
            background-color: #21252b;
            padding: 20px 40px;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
            margin-top: 20px;
            min-width: 350px;
        }}
        p {{ font-size: 1.1em; line-height: 1.6; margin: 12px 0; }}
        .label {{ color: #c678dd; font-weight: bold; }}
        .value {{ color: #98c379; }}
        .highlight {{ color: #e06c75; font-weight: bold; font-size: 1.2em; }}
        .time {{ color: #5c6370; font-size: 0.9em; margin-left: 10px; }}
    </style>
</head>
<body>
    {content}
</body>
</html>"""

class SensorWebHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path != "/":
            self.send_error(404, "Página no encontrada")
            return
        try:
            readings = read_log_file()
            page = build_html(readings)
        except OSError as error:
            message = html.escape(str(error))
            page = build_html([])
            print(f"Error al leer el archivo: {message}")
            
        body = page.encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

def run_server():
    server = HTTPServer(("0.0.0.0", PORT), SensorWebHandler)
    print(f"Servidor escuchando en el puerto {PORT}")
    print("Presione Ctrl+C para detenerlo.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()

if __name__ == "__main__":
    run_server()
