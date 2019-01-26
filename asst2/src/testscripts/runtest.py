#
# Usage:
#   import runtest
#   runtest.run(testcommands, outputfile,
#               menuprompt=None,	default "OS/161 kernel [? for menu]: "
#               shellprompt=None,	default "OS/161$ "
#               conf=None,		default is sys161 default behavior
#               ram=None, 		default is per sys161 config
#               cpus=None,		default is per sys161 config
#               doom=None,		default is no doom counter
#               progress=30,		default is 30 seconds
#               timeout=300,		default is 300 seconds
#               kernel=None)		default is "kernel"
#
# Returns None on success or a (string) message if something apparently
# went wrong in the middle. (XXX: should it throw exceptions instead?)
#
# * The testcommands argument is a string containing a list of commands
# separated by semicolons.  These can be either kernel menu commands
# or shell commands; the command 's' is recognized for switching from
# the menu to the shell and 'exit' for switching back to the menu.
# (This affects waiting for prompts - running the shell via 'p' or
# crashing out of the shell will confuse things.)
#
# The command 'q' from the menu is also recognized as causing a
# shutdown. This will be done automatically after everything else if
# not issued explicitly.
#
# The following commands are interpreted as macros:
#    DOMOUNT		expands to "mount sfs lhd1:; cd lhd1:"
#    DOUNMOUNT		expands to "cd /; unmount lhd1:"
#    WAIT		sleeps 3 seconds and just presses return
#
# * The outputfile argument should be a python file (e.g. sys.stdout)
# and receives a copy of the System/161 output.
#
# * The menuprompt and shellprompt arguments can be used to change the
# menu and shell prompt strings looked for. For the moment these can
# only be fixed strings, not regular expressions. (This is probably
# easy to improve, but I ran into some mysterious problems when I
# tried, so YMMV.) By default if you pass None prompt strings matching
# what OS/161 issues by default are used.
#
# * The conf argument can be used to supply an alternate sys161.conf
# file. If None is given (the default), sys161 will use its default
# config file.
#
# * The ram and cpus arguments can be used to override the RAM size
# and number-of-cpus settings in the sys161 config file. The number of
# cpus must be an integer, but any RAM size specification understood
# by sys161 can be used. Note: this feature requires System/161 2.0.5
# or higher.
#
# * The doom argument can be used to set the doom counter. If None is
# given (the default) the doom counter is not engaged.
#
# * The progress and timeout arguments can be used to set the timeouts
# for System/161 progress monitoring and pexpect-level global timeout,
# respectively. The defaults (somewhat arbitraily chosen) are 30 and
# 300 seconds. Passing progress=None disables progress monitoring; this
# is necessary for nontrivial tests that run within the kernel, as
# progress monitoring measures userland progress. Passing timeout=None
# probably either disables the global timeout or makes pexpect crash;
# I haven't tested it. I don't recommend trying: it is your defense
# against test runs hanging forever.
#
# Note that no-debugger unattended mode (sys161 -X) is always used.
# The purpose of this script is specifically to support unattended
# test runs...
#
# Depends on pexpect, which you may need to install specifically
# depending on your OS.
#

import time
import pexpect

#
# Macro commands
#
macros = {
	"MOUNT" : ["mount sfs lhd1:", "cd lhd1:"],
	"UNMOUNT" : ["cd /", "unmount lhd1:"],
	# "WAIT" special-cased below
}

#
# Wait for a prompt; returns True if we got it, False if we need to
# bail.
#
def getprompt(proc, prompt):
	which = proc.expect_exact([
			prompt,
			"panic: ",		# panic message
			"sys161: No progress in ", # sys161 deadman print
			"sys161: Elapsed ",	# sys161 shutdown print
			pexpect.EOF,
			pexpect.TIMEOUT
		])
	if which == 0:
		# got the prompt
		return None
	if which == 1:
		proc.expect_exact([pexpect.EOF, pexpect.TIMEOUT])
		return "panic"
	if which == 2:
		proc.expect_exact([pexpect.EOF, pexpect.TIMEOUT])
		return "progress timeout"
	if which == 3:
		proc.expect_exact([pexpect.EOF, pexpect.TIMEOUT])
		return "unexpected shutdown"
	if which == 4:
		return "unexpected end of input"
	if which == 5:
		return "top-level timeout"
	return "runtest: Internal error: pexpect returned out-of-range result"
# end getprompt

#
# main test function
#
def run(testcommands, outputfile,
		menuprompt=None, shellprompt=None,
		conf=None, ram=None, cpus=None,
		doom=None,
		progress=30, timeout=300,
		kernel=None):
	if menuprompt is None:
		menuprompt = "OS/161 kernel [? for menu]: "
	if shellprompt is None:
		shellprompt = "OS/161$ "
	if kernel is None:
		kernel = "kernel"

	args = ["-X"]
	if conf is not None:
		args.append("-c")
		args.append(conf)
	if cpus is not None:
		args.append("-C")
		args.append("31:cpus=%d" % cpus)
	if doom is not None:
		args.append("-D")
		args.append("%d" % doom)
	if progress is not None:
		args.append("-Z")
		args.append("%d" % progress)
	if ram is not None:
		args.append("-C")
		args.append("31:ramsize=%s" % ram)
	args.append(kernel)

	proc = pexpect.spawn("sys161", args, timeout=timeout,
				ignore_sighup=False)
	proc.logfile_read = outputfile

	commands = [s.strip() for s in testcommands.split(";")]
	commands = [macros[c] if c in macros else [c] for c in commands]
	# Apparently list flatten() is unpythonic...
	commands = [c for sublist in commands for c in sublist]

	prompts = { True: shellprompt, False: menuprompt }
	inshell = False
	quit = False
	for cmd in commands:
		msg = getprompt(proc, prompts[inshell])
		if msg is not None:
			return msg
		if cmd == "WAIT":
			time.sleep(3)
			cmd = ""
		proc.send("%s\r" % cmd)
		if not inshell and cmd == "q":
			quit = True
		if not inshell and cmd == "s":
			inshell = True
		if inshell and cmd == "exit":
			inshell = False
	if not quit:
		if inshell:
			msg = getprompt(proc, prompts[inshell])
			if msg is not None:
				return msg
			proc.send("exit\r")
			inshell = False
		msg = getprompt(proc, prompts[inshell])
		if msg is not None:
			return msg
		proc.send("q\r")
		quit = True

	proc.expect_exact([pexpect.EOF, pexpect.TIMEOUT])

	# Apparently if you call pexpect.wait() you must have
	# explicitly read all the input, or it hangs; and the process
	# can't be already dead, or it crashes. Therefore it appears
	# to be entirely useless. I hope not calling it doesn't cause
	# zombies to accumulate.
	#proc.wait()

	return None
# end run
