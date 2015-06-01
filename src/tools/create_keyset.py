#!/usr/bin/python

import csv
import pprint
import struct
import sys

import copy

# Read in file and return array of each line
def read_file(name):
	ret = []
	with open(name) as f:
		ret = f.readlines()
	return ret

# Filter out unecessary information from the header file and create a dictionary
# of all the constant values.
def filter_constants(ll):
	ret = {}
	for l in ll:
		n = l.find("#define KBD_EVENT_")
		if n != -1:
			tmp = l[18:].split()
			if len(tmp) != 2:
				print "ERROR"
			ret[tmp[0]] = int(tmp[1], 16)
	return ret

# Split up a series of hex integers separated by space and return a list of
# integers.
def get_integers(s):
	ret = []
	re = s.split()
	for r in re:
		ret.append(int(r, 16))
	return ret
		
# Read in CSV-file and create a list of dictionaries containing all the
# translations
def get_sc_values(name):
	ret = []
	head = []
	first = True
	with open(name, 'rb') as f:
		reader = csv.reader(f, delimiter=',', quotechar='"')
		for r in reader:
			if first == True:
				for rr in r:
					head.append(get_integers(rr))
				first = False
				continue
			ins = {}
			ins["keycode"] = r[0]
			ins["make"] = get_integers(r[1])
			ins["break"] = get_integers(r[2])
			ret.append(ins)
	return (ret, head)

# Replace the key code values with either a constant or an ASCII value
# Format:
# [
#  {"break" : [int,..], "keycode" : int, "make" : [int,...]}, ...
# ]
def replace_sc_values(sc, constants):
	for i in range(0, len(sc)):
#		if len(sc[i]["keycode"]) == 1:
#			sc[i]["keycode"] = ord(sc[i]["keycode"])
		if sc[i]["keycode"] in constants:
			sc[i]["keycode"] = constants[sc[i]["keycode"]]
		else:
			print "ERROR on " + sc[i]["keycode"]
			sys.exit(0)
	return sc

# Transform the list into something that is easier to handle, format:
# [
#  {
#    "scancode" : [int,..], "keycode" : int, "break" : true/false
#  }
# ]
def transform_list(sc):
	ret = []
	for s in sc:
		i1 = {
			"scancode" : s["break"],
			"keycode" : s["keycode"],
			"break" : True
		}
		i2 = {
			"scancode" : s["make"],
			"keycode" : s["keycode"],
			"break" : False
		}
		ret.append(i1)
		ret.append(i2)
	return sorted(ret, key=lambda k: k['scancode'])


def get_binary(li, ret):
	for l in li:
		sc = l["scancode"][0]
		fl = 0
		if l["break"] == True:
			fl = 1
		ret[sc] = struct.pack("<B", l["keycode"]) + struct.pack("<B", fl)
	return ret


def get_all_blocks(li, head):
	remaining = []
	blocks = []

	block = ["\x00\x00"]*256

	lli = copy.deepcopy(li)

	# Take care of the singles first
	bl1 = []
	for l in li:
		if len(l["scancode"]) == 1:
			bl1.append(l)
	
	for b in bl1:
		li.remove(b)

	block = get_binary(bl1, block)
	blocks.append(block)
	
	
	# Get (almost) all the other blocks 
	for i in range(0, len(head)):
		block = ["\x00\x00"]*256
		bl1 = []
		for l in li:
			if len(l["scancode"]) > len(head[i]):
				insert = True
				for j in range(0, len(head[i])):
					if head[i][j] != l["scancode"][j]:
						insert = False
						break
				if insert == True:
					if (len(head[i])+1) == len(l["scancode"]):
						l["scancode"] = l["scancode"][len(head[i]):]
						bl1.append(l)
		for b in bl1:
			li.remove(b)
		block = get_binary(bl1, block)
		blocks.append(block)


	# Need to fill in references to the other blocks where necessary

	ret = []
	ret.append(blocks[0])
	mappings = {}

	block_index = 0
	for i in reversed(range(0, len(head))):
		ret.append(blocks[i+1])
		block_index += 1
		val = 0
		if len(head[i]) == 1:
			mappings[head[i][0]] = block_index
			ret[0][head[i][0]] = struct.pack("<B", block_index) + "\x80"
		else:
			change_index = 0
			for j in range(0, len(head[i])-1):
				if head[i][j] not in mappings:
					print "ERROR"
					sys.exit(0)
				else:
					change_index = mappings[head[i][j]]
			ret[change_index][head[i][len(head[i])-1]] = \
				struct.pack("<B", block_index) + "\x80"
	return ret


def write_binary_file(name, data):
	with open(name, 'wb') as f:
		for dd in data:
			for d in dd:
				f.write(d)



def get_key_translation(const):
	normal = ["\x00"]*256
	shift  = ["\x00"]*256

	for c in const.keys():
		if len(c) == 1:
			normal[const[c]] = ord(c)
			shift[const[c]] = (c.lower())
		elif c.startswith('KP'):
			end = c[2:]
			if len(end) == 1:
				normal[const[c]] = ord(end)
				shift[const[c]] = (end.lower())
#		elif c == "COMMA":
		
# Format of file

# Example:
# LGUI,E0 1F
# - Index E0 in block 0 == 0xF1 (start of block 1)
# - Index 1F in block 1 == key code
# LGUIs,F0 1F
# - Index F0 in block 0 == 0xF1*2 (start of block 2)
# - Index 1F in block 2 == key code

def main():
	c = read_file("../include/kbd_constants.h")

	constants = filter_constants(c)

#	pprint.pprint(constants)

	(sc, head) = get_sc_values("../../data/sc/sc2_101.csv")

	sc = replace_sc_values(sc, constants)

	li = transform_list(sc)

	blocks = get_all_blocks(li, head)

	write_binary_file("sc2.bin", blocks)


main()



