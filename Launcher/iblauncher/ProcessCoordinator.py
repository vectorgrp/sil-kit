#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c)  Vector Informatik GmbH. All rights reserved.
#######################################################################################################################

import subprocess
import threading
import sys
import os
import time
import datetime
import atexit

from threading import Thread
try:
    from Queue import Queue, Empty  # for Python 2.x
except ImportError:
    from queue import Queue, Empty  # for Python 3.x

from iblauncher import Configuration

class ProcessCoordinator:
    """Piping multiple subprocesses with input and output to parent

        Inspired by:
        * https://stackoverflow.com/questions/22565606/python-asynhronously-print-stdout-from-multiple-subprocesses
        * https://lyceum-allotments.github.io/2017/03/python-and-pipes-part-6-multiple-subprocesses-and-pipes/

        Note:
        * If the launched process does not flush stdout after each line, buffering will occur when it connects to our 
          pipe instead of a tty. Use `std::endl` instead of `"\n"`.
          A Posix-only solution would be https://gist.github.com/hayd/4f46a68fc697ba8888a7b517a414583e but requires the 
          termios module not part of the standard Python distro.
    """

    #__logFilePath: str
    #__verbose: bool
    #__logFile: object
    #__processes: dict
    #__endThreadsSignal: object
    #__inputPipeThreads: dict
    #__outputThreads: dict
    #__outputQueue: Queue
    
    #######################################################################################################################
    #def __init__(self: object, logFilePath: str, verbose: bool):
    def __init__(self, logFilePath, verbose):
        """Constructor of the `ProcessCoordinator`.

        Parameters
        ----------
        logFilePath: str
            Path to a log file to create and fill with output from the spawned processes
        verbose: bool
            True iff informational output should be sent to stdout
        """
        self.__verbose = verbose

        # Dictionary of managed processes
        self.__processes = {}
        # Signal to terminate all threads
        self.__endThreadsSignal = threading.Event()
        # Queue for storing input/output lines
        self.__outputQueue = Queue()
        # Dictionaries of threads per process, monitoring their pipes
        self.__inputPipeThreads = {}
        self.__outputPipeThreads = {}

        # File for storing output lines
        self.__logFilePath = logFilePath
        self.__logFile = None
        if logFilePath:
            try:
                self.__logFile = open(logFilePath, 'w')
            except BaseException as e:
                print("Error: Could not create log file '" + logFilePath + "': '" + str(e) + "'.")
                raise e
            if verbose:
                print("Writing to log file '" + logFilePath + "'")
            self.__log("Launcher: Log file started")

        atexit.register(self.__cleanup)

    #######################################################################################################################
    #def __cleanup(self: object):
    def __cleanup(self):
        """Destructor of the `ProcessCoordinator`. Free resources."""
        if self.__logFile:
            self.__log("Launcher: Log file ended")
            self.__logFile.close()
            self.__logFile = None

    @staticmethod
    def __readOutputFromPipe(outPipe, queue, prefix, event):
        """Thread method for reading output from `pipe`; and whenever a line has been read, it enqueues the line to `queue`
        
        Parameters
        ----------
        outPipe: Pipe
            The pipe to read, usually an stdout/stderr pipe
        queue: Queue
            The queue to fill with string entries from the outPipe
        prefix: str
            The prefix to prepend to the string, usually the process's unique identifier
        event: Event
            The event object, fired when this thread should terminate
        """
        try:
            for buffer in iter(outPipe.readline, b''):
                if event.is_set(): break
                # Remove line breaks, ignore empty lines
                line = buffer.rstrip('\n')
                if line: queue.put(prefix + ": " + line)
        except ValueError:
            # This happens when the pipe is no longer open
            pass
        outPipe.close()

    @staticmethod
    def __forwardInputFromNamedPipeToStdin(outPipe, inPipeName, event):
        """Thread method for reading input from a pipe with name `inPipeName`, and writing this input straight into `outPipe`
        
        Parameters
        ----------
        outPipe: Pipe
            The pipe to fill, usually an stdin pipe
        inPipeName: str
            The pipe to read, usually a named pipe that is filled by the user
        event: Event
            The event object, fired when this thread should terminate
        """
        while True and not event.is_set():
            with open(inPipeName, "r") as f:
                outPipe.write(f.read())

    def __log(self, line, newline=True):
        """Write `line` to standard output and additionally --if opened-- to the `logFile`"""
        if newline: line += '\n'
        sys.stdout.write(line)
        sys.stdout.flush()
        if self.__logFile:
            self.__logFile.write(datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S - ") + line)
            self.__logFile.flush()

    #######################################################################################################################
    #def launchProcess(self: object, processName: str, commandAbsolutePath: str, arguments: str, workingFolderAbsolutePath: str, integrationBusEnvironment: list, shell: bool, verbose: bool) -> object:
    def launchProcess(self, processName, commandAbsolutePath, arguments, workingFolderAbsolutePath, integrationBusEnvironment, shell):
        """Spawn the given process under Windows/Posix with/without shell. Note: shell=True exposes a security vulnerability

        Parameters
        ----------
        processName: str
            The identifier to use for logging information, should be unique
        commandAbsolutePath: str
            The executable to spawn
        arguments: str
            The commandline arguments to set for this process
        workingFolderAbsolutePath: str
            The working folder to set for this process
        integrationBusEnvironment: list
            Environment variables to be set additionally
        shell: bool
            Use the system's shell if true; else, launch the process with the list of `arguments` split internally by ' ' (Windows) or via shlex (Linux). 
            Must be used when cmd/bash commands are involved.
        """
        # Environment variables
        environment = os.environ
        environment.update(integrationBusEnvironment)
        environment.update({ "INTEGRATIONBUS_SPAWNEDBYLAUNCHER": "1" })

        if os.name == "nt":
            environment.update({
                # Make IntegrationBus DLL dependencies resolve from the driver's installed location: set "PATH=%IntegrationBusWin32Folder%;%PATH%"
                # Note: os.path.sep != os.pathsep (file system vs. environment path separator)
                "PATH": Configuration.getIntegrationBusLibraryPath() + ((os.pathsep + environment["PATH"]) if "PATH" in environment else "")
            })
            if self.__verbose:
                print("  Windows invocation: '" + commandAbsolutePath + " " + arguments + "'")
        elif os.name == "posix":
            environment.update({
                "LD_LIBRARY_PATH": Configuration.getIntegrationBusLibraryPath() + ((os.pathsep + environment["LD_LIBRARY_PATH"]) if "LD_LIBRARY_PATH" in environment else "")
            })
            if self.__verbose:
                print("  Linux invocation: '" + commandAbsolutePath + " " + arguments + "'")
        if shell:
            args = commandAbsolutePath + " " + arguments
        else:
            args = [commandAbsolutePath] + (shlex.split(arguments) if os.name == "posix" else arguments.split(' '))

        process = subprocess.Popen(args, env=environment, cwd=workingFolderAbsolutePath, shell=shell, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, universal_newlines=True)
        self.__processes[processName] = process

        self.__endThreadsSignal.clear()

        # Create thread to read output of this spawned process into a queue
        t = Thread(target=ProcessCoordinator.__readOutputFromPipe, args=(process.stdout, self.__outputQueue, processName, self.__endThreadsSignal))
        t.daemon = True
        t.start()
        self.__outputPipeThreads[processName] = t

        # Create a name pipe and forward their input to this spawned process (Posix only)
        if os.name == "posix":
            # Create OS-wide named pipes, so the user can 'echo "Hello!" > Process1' from another terminal
            # Start threads to read input from the named pipes
            try:
                os.remove(processName)
            except:
                pass
            try:
                os.mkfifo(processName)
                if self.__verbose:
                    print("Created input pipe '" + processName + "' that forwards input to process '" + processName + "'.")  # e.g., call 'echo "[Input]" > [ProcessName]'
            except OSError as e:
                if self.__verbose:
                    print("Warning: Failed to create input pipe '" + processName + "' ('" + e + "').")
            t = Thread(target=ProcessCoordinator.__forwardInputFromNamedPipeToStdin, args=(process.stdin, processName, self.__endThreadsSignal))
            t.daemon = True
            t.start()
            self.__inputPipeThreads[processName] = t

        return process

    #######################################################################################################################
    #def monitorProcesses(self: object) -> bool:
    def monitorProcesses(self):
        """Await the previously spawned processes until all terminate successfully, or until one terminates with an error code

        Return
        ------
        bool
            True iff successful, all processes exited with a return code of zero. 
            False iff one of the spawned processes terminated with a non-zero return code.
        """
        while True:
            try:
                line = self.__outputQueue.get(block=True, timeout=0.1)
            except Empty:
                pass
            else:
                self.__log(line)

            newProcesses = dict(self.__processes)
            for processName in self.__processes.keys():
                p = self.__processes[processName]
                p.poll()
                if p.returncode is not None:
                    self.__log("Launcher: Process '" + processName + "' exited with return code " + "0x{0:0{1}x}".format(p.returncode, 8) + ".")
                    try:
                        os.remove(processName)
                    except:
                        pass
                    del newProcesses[processName]
                    if p.returncode != 0:
                        return False
            self.__processes = newProcesses

            # Break when all processes are done.
            if len(self.__processes) == 0: #all(p.poll() is not None for p in processes.values()):
                self.__log("Launcher: All processes ended without error.")
                sys.stdout.flush()
                break

        return True

    #######################################################################################################################
    #def terminateProcesses(self: object) -> bool:
    def terminateProcesses(self):
        """Terminate all processes previously spawned

        Return
        ------
        bool
            True iff successful, all processes could be terminated
        """
        self.__endThreadsSignal.set()

        if len(self.__processes) == 0:
            return True

        self.__log("Launcher: Processes that are still running are terminated.")
        sys.stdout.flush()
        # Nicely ask the processes to stop
        for p in self.__processes.values():
            p.terminate()
        time.sleep(.5)
        # Kill if still alive
        newProcesses = dict(self.__processes)
        for processName in self.__processes.keys():
            p = self.__processes[processName]
            p.poll()
            if p.returncode is not None:
                try:
                    p.kill()
                    self.__log("Launcher: Process '" + processName + "' was unresponsive and had to be killed.")
                except OSError:
                    self.__log("Launcher: Process '" + processName + "' has ended.")
            else:
                self.__log("Launcher: Process '" + processName + "' exited with return code " + str(p.returncode) + ".")
            try:
                os.remove(processName)
            except:
                pass
            del newProcesses[processName]
        self.__processes = newProcesses

        if len(self.__processes) > 0:
            return False

        return True
