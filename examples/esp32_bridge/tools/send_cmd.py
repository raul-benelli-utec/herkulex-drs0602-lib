#!/usr/bin/env python3
"""Cliente UDP mínimo para probar el bridge ESP32 -> Arduino."""

import argparse
import socket
import sys


def main() -> int:
    parser = argparse.ArgumentParser(description="Enviar comando al bridge ESP32")
    parser.add_argument("ip", help="IP de la ESP32")
    parser.add_argument("command", type=int, help="Comando binario 0-15")
    parser.add_argument("--port", type=int, default=3333)
    parser.add_argument("--timeout", type=float, default=1.0)
    args = parser.parse_args()

    if not 0 <= args.command <= 15:
        print("El comando debe estar entre 0 y 15", file=sys.stderr)
        return 1

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(args.timeout)

    payload = bytes([args.command])
    sock.sendto(payload, (args.ip, args.port))

    try:
        ack, _ = sock.recvfrom(8)
        print(f"Enviado: {args.command} | ACK: {ack!r}")
    except socket.timeout:
        print(f"Enviado: {args.command} | sin ACK (puede ser normal si el FW no responde)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
