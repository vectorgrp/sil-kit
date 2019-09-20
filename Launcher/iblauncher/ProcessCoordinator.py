#######################################################################################################################
# IntegrationBus Launcher Plugin
# Copyright (c) Vector Informatik GmbH. All rights reserved.
#######################################################################################################################

import subprocess
import threading
import sys
import os
import re
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

    @staticmethod
    def __cmdline_split(s, platform='this'):
        """Multi-platform variant of shlex.split() for command-line splitting.
        For use with subprocess, for argv injection etc. Using fast REGEX.
        This is a smarter alternative for expression "(shlex.split(arguments) if os.name == "posix" else arguments.split(' '))".
        Source: https://stackoverflow.com/questions/33560364/python-windows-parsing-command-lines-with-shlex

        Parameters
        ----------
        s: str
            The command line string containing command plus command-line arguments to split
        platform: 'this' = auto from current platform;
                  1 = POSIX; 
                  0 = Windows/CMD
                  (other values reserved)

        Returns
        -------
        list
            The list of strings, one element per command or argument
        """
        if platform == 'this':
            platform = (sys.platform != 'win32')
        if platform == 1:
            RE_CMD_LEX = r'''"((?:\\["\\]|[^"])*)"|'([^']*)'|(\\.)|(&&?|\|\|?|\d?\>|[<])|([^\s'"\\&|<>]+)|(\s+)|(.)'''
        elif platform == 0:
            RE_CMD_LEX = r'''"((?:""|\\["\\]|[^"])*)"?()|(\\\\(?=\\*")|\\")|(&&?|\|\|?|\d?>|[<])|([^\s"&|<>]+)|(\s+)|(.)'''
        else:
            raise AssertionError('unkown platform %r' % platform)

        args = []
        accu = None   # collects pieces of one arg
        for qs, qss, esc, pipe, word, white, fail in re.findall(RE_CMD_LEX, s):
            if word:
                pass   # most frequent
            elif esc:
                word = esc[1]
            elif white or pipe:
                if accu is not None:
                    args.append(accu)
                if pipe:
                    args.append(pipe)
                accu = None
                continue
            elif fail:
                raise ValueError("invalid or incomplete shell string")
            elif qs:
                word = qs.replace('\\"', '"').replace('\\\\', '\\')
                if platform == 0:
                    word = word.replace('""', '"')
            else:
                word = qss   # may be even empty; must be last

            accu = (accu or '') + word

        if accu is not None:
            args.append(accu)

        return args

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
                print("  Windows invocation: '" + '"' + commandAbsolutePath + '"' + " " + arguments + "'")
        elif os.name == "posix":
            environment.update({
                "LD_LIBRARY_PATH": Configuration.getIntegrationBusLibraryPath() + ((os.pathsep + environment["LD_LIBRARY_PATH"]) if "LD_LIBRARY_PATH" in environment else "")
            })
            if self.__verbose:
                print("  Linux invocation: '" + '"' + commandAbsolutePath + '"' + " " + arguments + "'")
        if shell:
            # Enquote absolute command path to support paths with spaces
            args = '"' + commandAbsolutePath + '"' + ' ' + arguments
        else:
            args = [commandAbsolutePath] + ProcessCoordinator.__cmdline_split(arguments)

        process = subprocess.Popen(args, env=environment, cwd=workingFolderAbsolutePath, shell=shell, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, bufsize=1, universal_newlines=True)
        self.__processes[processName] = process

        self.__endThreadsSignal.clear()

        # Create thread to read output of this spawned process into a queue
        t = Thread(target=ProcessCoordinator.__readOutputFromPipe, args=(process.stdout, self.__outputQueue, processName, self.__endThreadsSignal))
        t.daemon = True
        t.start()
        self.__outputPipeThreads[processName] = t

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
                    self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " exited with return code " + "0x{0:0{1}x}".format(p.returncode, 8) + ".")
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
        # On Linux, nicely ask the processes with SIGTERM to stop
        # Under Windows there is no SIGTERM, both p.terminate() and p.kill() simply terminate
        if os.name == "posix":
            for p in self.__processes.values():
                p.terminate()
            time.sleep(.5)
        # Kill remaining processes if still alive
        newProcesses = dict(self.__processes)
        for processName in self.__processes.keys():
            p = self.__processes[processName]
            returncode = p.poll()
            if returncode is None:
                if os.name == "posix":
                    # On Linux, try to send a SIGKILL to the process
                    try:
                        p.kill()
                        self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " was unresponsive and thus killed.")
                    except OSError as e:
                        self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " was unresponsive and could not be killed (" + str(e) + ").")
                elif os.name == "nt":
                    # Under Windows, since p.kill() simply terminates a process without its child processes, we must rely on an OS command.
                    # Cf. https://stackoverflow.com/questions/1230669/subprocess-deleting-child-processes-in-windows
                    try:
                        from subprocess import DEVNULL  # for Python 3.x
                    except ImportError:
                        DEVNULL = open(os.devnull, 'wb')  # for Python 2.x
                    exitcode = subprocess.call(['taskkill', '/F', '/T', '/PID', str(p.pid)], stdout=DEVNULL, stderr=subprocess.STDOUT)
                    if exitcode == 0 or exitcode == 128:  # Success, or 'No child processes to wait for', see http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381(v=vs.85).aspx
                        self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " was still running and thus killed.")
                    else:  
                        self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " is still running and could not be killed (error code " + "0x{0:0{1}x}".format(exitcode, 8) + ").")
            else:
                self.__log("Launcher: Process of '" + processName + "' with PID " + str(p.pid) + " exited with return code " + "0x{0:0{1}x}".format(p.returncode, 8) + ".")
            del newProcesses[processName]

        self.__processes = newProcesses

        if len(self.__processes) > 0:
            return False

        return True
