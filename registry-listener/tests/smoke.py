import pathlib
import re
import signal
import socket
import subprocess
import sys
import tempfile
import time
import unittest


REGISTRY, PARTICIPANT = sys.argv[1:]
sys.argv[1:] = []


class RegistryListenerSmoke(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="registry-listener-")
        self.addCleanup(self.directory.cleanup)
        self.root = pathlib.Path(self.directory.name)
        self.processes = []
        self.config = self.root / "registry.yaml"
        self.config.write_text("Middleware:\n  EnableDomainSockets: false\n")
        self.addCleanup(self.cleanup_processes)

    def cleanup_processes(self):
        for process, _ in reversed(self.processes):
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            process.stdin.close()

    def start(self, *args):
        log = self.root / f"process-{len(self.processes)}.log"
        with log.open("w") as output:
            process = subprocess.Popen(
                args, stdin=subprocess.PIPE, stdout=output, stderr=subprocess.STDOUT, text=True
            )
        self.processes.append((process, log))
        return process, log

    def wait_for(self, running, pattern):
        process, log = running
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            output = log.read_text()
            match = re.search(pattern, output)
            if match:
                return match
            self.assertIsNone(process.poll(), output)
            time.sleep(0.02)
        self.fail(f"Timed out waiting for {pattern!r}:\n{log.read_text()}")

    def registry(self):
        running = self.start(
            REGISTRY, "--configuration", str(self.config), "--listen-uri", "silkit://127.0.0.1:0"
        )
        uri = self.wait_for(running, r'OnRegistryUri uri="([^"]+)"').group(1)
        self.assertNotEqual(uri.rsplit(":", 1)[1], "0")
        with socket.create_connection(("127.0.0.1", int(uri.rsplit(":", 1)[1])), timeout=2):
            pass
        return running, uri

    def test_configured_listen_uri(self):
        self.config.write_text(
            "Middleware:\n  EnableDomainSockets: false\n  RegistryUri: silkit://127.0.0.1:0\n"
        )
        registry = self.start(REGISTRY, "--configuration", str(self.config))
        uri = self.wait_for(registry, r'OnRegistryUri uri="([^"]+)"').group(1)
        self.assertRegex(uri, r"^silkit://127\.0\.0\.1:[1-9][0-9]*$")
        registry[0].send_signal(signal.SIGTERM)
        self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())

        self.config.write_text(
            "Middleware:\n  EnableDomainSockets: false\n  RegistryUri: not-a-uri\n"
        )
        registry, _ = self.registry()
        registry[0].send_signal(signal.SIGTERM)
        self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())

    def test_all_events_across_simulations(self):
        registry, uri = self.registry()
        for simulation in ("alpha", "beta"):
            client = self.start(PARTICIPANT, f"{uri}/{simulation}")
            self.wait_for(client, "READY")
            context = f'simulation="{simulation}" participant="Fixture"'
            self.wait_for(registry, f"OnParticipantConnected {context}")
            self.wait_for(registry, f'OnRequiredParticipantsUpdate {context} required=\\["Fixture"\\]')
            self.wait_for(registry, f"OnParticipantStatusUpdate {context}.*State=Running")
            self.wait_for(registry, f"OnServiceDiscoveryEvent {context}.*SmokeCan")
            self.wait_for(registry, f'OnMetricsUpdate simulation="{simulation}" origin="Fixture".*MetricData')
            client[0].stdin.write("\n")
            client[0].stdin.flush()
            self.assertEqual(client[0].wait(timeout=10), 0, client[1].read_text())
            self.wait_for(registry, f"OnParticipantDisconnected {context}")
        self.assertIn("OnLoggerCreated logger=", registry[1].read_text())
        registry[0].send_signal(signal.SIGINT)
        self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())

    def test_signal_shutdown_with_connected_participant(self):
        for sig in (signal.SIGINT, signal.SIGTERM):
            with self.subTest(signal=sig):
                registry, uri = self.registry()
                client = self.start(PARTICIPANT, uri)
                self.wait_for(client, "READY")
                registry[0].send_signal(sig)
                self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())
                client[0].terminate()
                client[0].wait(timeout=10)

    def test_startup_errors(self):
        malformed = self.root / "malformed.yaml"
        malformed.write_text("Middleware: [\n")
        cases = [
            ("--unknown",),
            ("--listen-uri", "not-a-uri"),
            ("--listen-uri", "tcp://127.0.0.1:0"),
            ("--configuration", str(malformed)),
            ("--configuration", str(self.root / "missing.yaml")),
        ]
        with socket.socket() as occupied:
            occupied.bind(("127.0.0.1", 0))
            occupied.listen()
            cases.append((
                "--configuration", str(self.config),
                "--listen-uri", f"silkit://127.0.0.1:{occupied.getsockname()[1]}",
            ))
            for args in cases:
                with self.subTest(args=args):
                    result = subprocess.run(
                        [REGISTRY, *args], capture_output=True, text=True, timeout=10
                    )
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn("registry-listener:", result.stderr)
                    self.assertNotIn("OnRegistryUri", result.stdout)


if __name__ == "__main__":
    unittest.main()
