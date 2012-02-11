#!/usr/bin/python -O
#---------------------------------------
#  WRITTEN  BY:    Simon Schaefer  2011
#---------------------------------------

import sys
import socket
import string
import subprocess
import shlex
import time

from optparse import OptionParser

SSH_TUNNEL_CALL="ssh -R DESTPORT:localhost:22 192.168.111.23"

def parseArgs(args):
	parser = OptionParser()
	parser.add_option('-p', '--port',     dest='port',      default=4324,             help='The port the world adapter should listen on')
	parser.add_option('-c', '--host',     dest='host',      default="192.168.111.23", help='Connect daemon to a specific host ip')
	parser.add_option('-i', '--id',       dest='id',        default="test.id",        help='The id of this client')

	return parser.parse_args(args)

options = parseArgs(sys.argv[1:])
print options
print options[0].id

Dream = 0

def login(host, port, clientId):
	global Dream
	print "logging in to " + host + ":" + str(port) + " and id: " + str(clientId)

	e = "dowhilemissing"
	while (e != ""):
		try:
			Dream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			Dream.connect((host, port))
			Dream.send(clientId)
			e = ""
		except Exception, e:
			time.sleep(1);
			print e	
	print "done"

login(options[0].host, options[0].port, options[0].id)

while (1):
	buffer = Dream.recv(1024)
	if buffer == "":
		print "disconnected trying to reconnect"
		Dream.shutdown(2) # 2 = SHUT_RDWR
		Dream.close()
		login(options[0].host, options[0].port, options[0].id)
		continue
	
	msg = buffer.split("|")
	print msg
	if msg[0].startswith("CREATE SSH"):
		print "starting ssh tunnel to " + str(msg[1])
		try:
			args = shlex.split(SSH_TUNNEL_CALL.replace("DESTPORT", str(msg[1])))
			print args
			#subprocess.Popen(args)
			Dream.send("DONE")
		except Exception, e:
			print e
