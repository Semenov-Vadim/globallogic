import atexit
import sys
import os
import time
from signal import SIGTERM

# File mode creation mask of the daemon.
UMASK = 0
# Default working directory for the daemon.
WORKDIR = "/"


class Daemon:
    def __init__(self, pidfile: str):
        self.pidfile = pidfile

    def create_daemon(self) -> None:
        # fork 1 to spin off the child that will spawn the deamon.
        try:
            pid = os.fork()
            if pid > 0:
                # exit parent
                sys.exit(0)
        except OSError as err:
            sys.stderr.write("fork failed: %d (%s)\n" % (err.errno, err.strerror))
            sys.exit(1)

        # decouple from parent environment
        os.chdir(WORKDIR)
        os.setsid()
        os.umask(UMASK)

        # fork 2
        try:
            pid = os.fork()
            if pid > 0:
                # exit from second parent
                sys.exit(0)
        except OSError as err:
            sys.stderr.write('fork #2 failed: {0}\n'.format(err))
            sys.exit(1)

        # write pidfile
        atexit.register(self.delpid)
        pid = str(os.getpid())
        with open(self.pidfile, 'w+') as pidfile:
            pidfile.write(pid + '\n')

    def delpid(self) -> None:
        os.remove(self.pidfile)

    def get_pid_by_file(self) -> int or None:
        """ Return the pid read from the pid file """
        try:
            with open(self.pidfile, 'r') as pidfile:
                pid = int(pidfile.read().strip())
            return pid
        except IOError:
            return

    def start(self) -> None:
        """Start the daemon"""
        sys.stderr.write("Starting...\n")

        # Check for a pidfile to see if the daemon already run
        if self.get_pid_by_file():
            message = "pidfile %s already exist. Daemon already running\n"
            sys.stderr.write(message % self.pidfile)
            sys.exit(1)

        # Start the daemon
        self.create_daemon()
        self.run()

    def stop(self) -> None:
        """Stop the daemon"""
        sys.stderr.write("Stopping...\n")

        pid = self.get_pid_by_file()
        if not pid:
            message = "pidfile %s does not exist. Daemon not running\n"
            sys.stderr.write(message % self.pidfile)
            return

        # Try killing the daemon process
        try:
            while 1:
                os.kill(pid, SIGTERM)
                time.sleep(0.1)
        except OSError as err:
            if 'No such process' in err.strerror and os.path.exists(self.pidfile):
                os.remove(self.pidfile)
            else:
                print(str(err))
                sys.exit(1)

    def restart(self) -> None:
        """Restart the daemon"""
        self.stop()
        self.start()

    def run(self) -> None:
        """ The main loop of the daemon. """
        while 1:
            # TODO add impl
            def func(a):
                if a < 900:
                    func(a+1)
            func(1)


if __name__ == "__main__":
    daemon = Daemon('/tmp/daemon-example.pid')
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            daemon.start()
        elif 'stop' == sys.argv[1]:
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            daemon.restart()
        else:
            print("Unknown command")
            sys.exit(2)
        sys.exit(0)
    else:
        print("usage: %s start|stop|restart" % sys.argv[0])
        sys.exit(2)
