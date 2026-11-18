import json
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

    def parse_events(self, output):
        events = []
        for line in output.splitlines(keepends=True):
            if not line.endswith("\n"):
                continue
            try:
                event = json.loads(line)
            except json.JSONDecodeError:
                if '"source":"silkit_registry"' in line:
                    self.fail(f"Invalid registry JSON event: {line!r}")
                continue
            if isinstance(event, dict) and event.get("source") == "silkit_registry":
                self.assertEqual(event["version"], 1)
                events.append(event)
        return events

    def wait_event(self, running, event_type, predicate=None, **fields):
        process, log = running
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            for event in self.parse_events(log.read_text()):
                if (event["type"] == event_type
                        and all(event.get(key) == value for key, value in fields.items())
                        and (predicate is None or predicate(event))):
                    return event
            self.assertIsNone(process.poll(), log.read_text())
            time.sleep(0.02)
        self.fail(f"Timed out waiting for {event_type!r} {fields!r}:\n{log.read_text()}")

    def registry(self):
        running = self.start(
            REGISTRY, "--configuration", str(self.config), "--listen-uri", "silkit://127.0.0.1:0"
        )
        uri = self.wait_event(running, "registry_uri")["uri"]
        self.assertNotEqual(uri.rsplit(":", 1)[1], "0")
        with socket.create_connection(("127.0.0.1", int(uri.rsplit(":", 1)[1])), timeout=2):
            pass
        return running, uri

    def test_configured_listen_uri(self):
        self.config.write_text(
            "Middleware:\n  EnableDomainSockets: false\n  RegistryUri: silkit://127.0.0.1:0\n"
        )
        registry = self.start(REGISTRY, "--configuration", str(self.config))
        uri = self.wait_event(registry, "registry_uri")["uri"]
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
        for simulation in ("session/alpha", "session/beta"):
            client = self.start(PARTICIPANT, f"{uri}/{simulation}")
            self.wait_for(client, "READY")
            context = {"simulation": simulation, "name": "Fixture"}
            self.wait_event(registry, "generation_created", simulation=simulation, generation=1)
            self.wait_event(registry, "participant_connected", generation=1, **context)
            self.wait_event(registry, "required_participants", generation=1, required=["Fixture"], **context)
            self.wait_event(registry, "participant_status", generation=1, state="Running", **context)
            self.wait_event(
                registry, "generation_state_changed", simulation=simulation, generation=1, state="Running"
            )
            self.wait_event(
                registry, "service_discovery",
                predicate=lambda event: event["service"]["name"] == "SmokeCan", **context,
            )
            self.wait_event(registry, "metrics", simulation=simulation, origin="Fixture",
                            predicate=lambda event: bool(event["metrics"]))
            client[0].stdin.write("\n")
            client[0].stdin.flush()
            self.assertEqual(client[0].wait(timeout=10), 0, client[1].read_text())
            self.wait_event(registry, "participant_disconnected", generation=1, **context)
            self.wait_event(
                registry, "generation_state_changed", simulation=simulation, generation=1, state="Shutdown"
            )
            self.wait_event(registry, "generation_removed", simulation=simulation, generation=1)
        self.wait_event(registry, "logger_created")
        registry[0].send_signal(signal.SIGINT)
        self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())

    def test_generation_created_once_per_generation(self):
        registry, uri = self.registry()
        clients = [self.start(PARTICIPANT, f"{uri}/session/generations") for _ in range(2)]
        for client in clients:
            self.wait_for(client, "READY")
        for generation in (1, 2):
            context = {"simulation": "session/generations", "name": "Fixture", "generation": generation}
            self.wait_event(registry, "participant_connected", **context)
            self.wait_event(registry, "required_participants", required=["Fixture"], **context)
            self.wait_event(registry, "participant_status", state="Running", **context)
            self.wait_event(
                registry, "generation_created", simulation="session/generations", generation=generation
            )
            self.wait_event(
                registry, "generation_state_changed",
                simulation="session/generations", generation=generation, state="Running",
            )
        for client in clients:
            client[0].stdin.write("\n")
            client[0].stdin.flush()
            self.assertEqual(client[0].wait(timeout=10), 0, client[1].read_text())
        for generation in (1, 2):
            self.wait_event(
                registry, "participant_disconnected", simulation="session/generations",
                name="Fixture", generation=generation,
            )
            self.wait_event(
                registry, "generation_removed", simulation="session/generations", generation=generation
            )
        registry[0].send_signal(signal.SIGTERM)
        self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())
        self.assertEqual(
            [event["generation"] for event in self.parse_events(registry[1].read_text())
             if event["type"] == "generation_created"],
            [1, 2],
        )

    def test_signal_shutdown_with_connected_participant(self):
        for sig in (signal.SIGINT, signal.SIGTERM):
            with self.subTest(signal=sig):
                registry, uri = self.registry()
                client = self.start(PARTICIPANT, uri)
                self.wait_for(client, "READY")
                for event_type in ("participant_connected", "required_participants", "participant_status"):
                    event = self.wait_event(registry, event_type, simulation="", name="Fixture")
                    self.assertIn("generation", event)
                    self.assertIsNone(event["generation"])
                registry[0].send_signal(sig)
                self.assertEqual(registry[0].wait(timeout=10), 0, registry[1].read_text())
                self.assertNotIn("generation_created",
                                 [event["type"] for event in self.parse_events(registry[1].read_text())])
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
                    self.assertNotIn("registry_uri",
                                     [event["type"] for event in self.parse_events(result.stdout)])

    def test_json_round_trip_and_structured_payloads(self):
        result = subprocess.run(
            [PARTICIPANT, "--emit-events"], capture_output=True, text=True, timeout=10
        )
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue(result.stdout.endswith("\n"))
        events = [json.loads(line) for line in result.stdout.splitlines()]
        for event in events:
            self.assertEqual(event["source"], "silkit_registry")
            self.assertEqual(event["version"], 1)
        by_type = {}
        for event in events:
            by_type.setdefault(event["type"], []).append(event)

        text = '"\\' + "".join(chr(ch) for ch in range(0x20)) + "\u00e9"
        self.assertEqual(by_type["registry_uri"][0]["uri"], text)
        self.assertEqual(by_type["participant_connected"][0]["simulation"], text)
        self.assertEqual(by_type["participant_connected"][0]["name"], text)
        self.assertEqual(by_type["participant_disconnected"][0]["name"], text)
        for event_type in ("generation_created", "generation_state_changed", "generation_removed",
                           "participant_connected", "participant_disconnected",
                           "participant_status", "required_participants"):
            self.assertEqual(by_type[event_type][0]["simulation"], text)
            self.assertEqual(by_type[event_type][0]["generation"], 2**64 - 1)
        self.assertEqual(by_type["generation_state_changed"][0]["state"], "Running")
        self.assertEqual(by_type["required_participants"][0]["required"], [text, "", "123"])
        self.assertEqual(by_type["required_participants"][1]["required"], [])
        self.assertIsNone(by_type["required_participants"][1]["generation"])
        status = by_type["participant_status"][0]
        self.assertEqual(status["state"], "Running")
        self.assertEqual(status["enter_reason"], text)
        self.assertEqual(status["enter_time_ns"], -123000000)
        self.assertEqual(status["refresh_time_ns"], 456000000)
        service = by_type["service_discovery"][0]
        self.assertEqual(service["event"], 1)
        self.assertEqual(service["service"]["id"], 2**64 - 1)
        self.assertEqual(service["service"]["name"], text)
        self.assertEqual(service["service"]["participant_name"], text)
        self.assertEqual(service["service"]["network_name"], text)
        self.assertEqual(service["service"]["supplemental"], {text: text, "": "", "123": "123"})
        metrics = by_type["metrics"][0]
        self.assertEqual(metrics["origin"], text)
        self.assertEqual(metrics["metrics"], [
            {"timestamp_ns": 1, "name": text, "kind": 3, "value": text},
            {"timestamp_ns": 2, "name": "counter", "kind": 0, "value": "18446744073709551615"},
            {"timestamp_ns": 3, "name": "statistic", "kind": 1, "value": "[1,2,3]"},
            {"timestamp_ns": 4, "name": "peers", "kind": 2, "value": '["123"]'},
        ])
        self.assertEqual(by_type["metrics"][1]["metrics"], [])


if __name__ == "__main__":
    unittest.main()
