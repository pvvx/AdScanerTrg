#!/usr/bin/env python
 
### TlsrMemInfo.py ###
###  Autor: pvvx   ###
 
import sys
import signal
import struct
import platform
import time
import argparse
import subprocess
import os
import io

__progname__ = "TLSR825x Check RetentionMem Address"
__filename__ = "TlsrRetMemAddr"
__version__ = "20.11.20"

SRAM_BASE_ADDR = 0x840000

class FatalError(RuntimeError):
	def __init__(self, message):
		RuntimeError.__init__(self, message)

	@staticmethod
	def WithResult(message, result):
		message += " (result was %s)" % hexify(result)
		return FatalError(message)

def signal_handler(signal, frame):
	print()
	print('Keyboard Break!')
	sys.exit(0)
	
def arg_auto_int(x):
	return int(x, 0)
	
class ELFFile:

	def __init__(self, name, tool_nm):
		self.name = name
		self.tool_nm = tool_nm
		self.symbols = {}
		try:
			#if sys.platform == 'linux2':
			#	tool_nm = "tc32-elf-nm"
			proc = subprocess.Popen([self.tool_nm, self.name], stdout=subprocess.PIPE)
		except OSError:
			print("Error calling " + self.tool_nm + ", do you have toolchain in PATH?")
			sys.exit(1)
		for l in proc.stdout:
			fields = l.strip().split()
			try:
				if fields[0] == b"U":
					#print("Warning: Undefined symbol '%s'!" %(fields[1].decode('ASCII')))
					continue
				if fields[0] == b"w":
					continue  # can skip weak symbols
				self.symbols[fields[2]] = int(fields[0], 16)
			except ValueError:
				raise FatalError("Failed to strip symbol output from nm: %s" % fields)

	def get_symbol_addr(self, sym, default = 0):
		try:
			x = self.symbols[sym]
		except:
			return default
		return x

def main():

	signal.signal(signal.SIGINT, signal_handler);
	parser = argparse.ArgumentParser(description='%s version %s' % (__progname__, __version__), prog=__filename__);
	parser.add_argument('-e','--elffile', help='Name of elf file', default = 'out.elf');
	parser.add_argument('-t','--tools', help='Path and name tc32-elf-nm', default = 'tc32-elf-nm');
	args = parser.parse_args();

	e = ELFFile(args.elffile, args.tools);
	rrs = e.get_symbol_addr(b"_retention_data_end_");
	if rrs == 0:
		rrs = e.get_symbol_addr(b"_ictag_start_");
	if rrs > 0:
		rrs = (rrs + 255) & 0x0001ff00;
		print("0x%x" % rrs);
	else:
		print("0x8000");
	sys.exit(0);
	

if __name__ == '__main__':
	main()
