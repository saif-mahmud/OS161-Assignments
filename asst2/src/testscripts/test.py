#!/usr/pkg/bin/python2.7
# test.py - run some test material
# usage: auto/test.py [options] test-commands
# options:
#    --menuprompt=STR	Change menu prompt string
#    --shellprompt=STR	Change shell prompt string
#    --conf=sys161.conf	Use alternate sys161 config
#    --ram=N		Force RAM size (default from sys161 config)
#    --cpus=N		Force number of cpus (default from sys161 config)
#    --doom=N		Set doom counter to N (default none)
#    --progress=N	Progress monitoring with N-second timeout (default 30)
#    --no-progress	Disable progress monitoring
#    --timeout=N	Global timeout, in seconds (default 300)
#    --kernel=KERNEL	Choose kernel to run (default "kernel")
#
#
# This is a directly executable wrapper around runtest.py. You can use
# it from the host OS shell to run more or less arbitrary scripts, or
# you can write your own Python scripts using runtest.py directly.
#
# See the top of runtest.py for an explanation of the arguments.
#

import sys
from optparse import OptionParser

import runtest

############################################################
# global settings

g_doom = None
g_conf = None
g_cpus = None
g_kernel = None
g_menuprompt = None
g_progress = 30
g_ram = None
g_shellprompt = None
g_timeout = 300

############################################################
# main

def getargs():
	global g_menuprompt
	global g_shellprompt
	global g_ram
	global g_conf
	global g_cpus
	global g_doom
	global g_progress
	global g_timeout
	global g_kernel

	# XXX is there no better scheme for this?
	p = OptionParser()

	# grr -h is hardwired
	p.add_option("-c", "--conf", dest="conf")
	p.add_option("-D", "--doom", dest="doom")
	p.add_option("-j", "--cpus", dest="cpus")
	p.add_option("-k", "--kernel", dest="kernel")
	p.add_option("-m", "--menuprompt", dest="menuprompt")
	p.add_option("-r", "--ram", dest="ram")
	p.add_option("-s", "--shellprompt", dest="shellprompt")
	p.add_option("-t", "--timeout", dest="timeout")
	p.add_option("-z", "--no-progress", dest="no_progress")
	p.add_option("-Z", "--progress", dest="progress")

	(options, args) = p.parse_args()
	if options.menuprompt is not None:
		g_menuprompt = options.menuprompt
	if options.shellprompt is not None:
		g_shellprompt = options.shellprompt
	if options.conf is not None:
		g_conf = options.conf
	if options.ram is not None:
		g_ram = options.ram
	if options.cpus is not None:
		g_cpus = int(options.cpus)
	if options.doom is not None:
		g_doom = int(options.doom)
	if options.progress is not None:
		g_progress = int(options.progress)
	if options.no_progress is not None:
		g_progress = None
	if options.timeout is not None:
		g_timeout = int(options.timeout)
	if options.kernel is not None:
		g_kernel = options.kernel

	if len(args) != 1:
		sys.stderr.write("Usage: test.py [options] test-commands\n")
		exit(1)
	return args[0]
# end getargs

testcommands = getargs()
msg = runtest.run(testcommands,
	sys.stdout,
	menuprompt=g_menuprompt,
	shellprompt=g_shellprompt,
	conf=g_conf,
	ram=g_ram,
	cpus=g_cpus,
	doom=g_doom,
	progress=g_progress,
	timeout=g_timeout,
	kernel=g_kernel)
if msg is not None:
	sys.stderr.write("test.py: test commands aborted with %s\n" % msg)
exit(0)
