#!/usr/bin/env python3 -B

import sys

from enum import Enum

class Cmds(Enum):
	LOAD = 0
	CLEAR = 1
	SETMINTERM = 2
	WAIT = 3
	PAUSE = 4
	RESUME = 5

def main():
	if len(sys.argv) != 2:
		print(f"usage: {sys.argv[0]} <script>", file=sys.stderr)
		sys.exit(2)

	parsed = []	

	with open(sys.argv[1]) as f:
		for l in f.readlines():
			lx = l.strip().lower().split(" ")
			if lx[0] == "load":
				parsed.append((Cmds.LOAD, int(lx[1])))
			elif lx[0] == "clear":
				parsed.append((Cmds.CLEAR, None))
			elif lx[0] == "setminterm":
				parsed.append((Cmds.SETMINTERM, int(lx[1])))
			elif lx[0] == "wait":
				parsed.append((Cmds.WAIT, int(lx[1])))
			# ...

	# DFA generation
	dfa_cstate = 0
	dfa_prelude = True
	dfa_mustwait = False
	dfa_cframe = 0
	dfa_norender = False

	tprint = lambda s: print("	"+s)

	def bwait(mustwait):
		if mustwait:
			tprint("WaitBlitter();")
			return False
		return True

	print("""
void BLITZ_MainLoop(void) {
	static int cstate;
	static bool norender = false;

	switch(cstate) {
""")

	for (cmd, arg) in parsed:
		if dfa_prelude:
			dfa_prelude = False
			dfa_norender = False
			print(f"case {dfa_cstate}:")
			tprint("norender = false;")
			tprint(f"if (frameCount < {dfa_cframe}) break;")
		if cmd == Cmds.LOAD:
			dfa_norender = True
			dfa_mustwait = bwait(dfa_mustwait)
			tprint(f"BLITZ_Load({arg});")
			dfa_mustwait = True
		elif cmd == Cmds.CLEAR:
			dfa_norender = True
			dfa_mustwait = bwait(dfa_mustwait)
			tprint(f"BLITZ_Clear();")
			dfa_mustwait = True
		elif cmd == Cmds.SETMINTERM:
			tprint(f"minterm_curr = {int(arg)};")
		elif cmd == Cmds.WAIT:
			dfa_cstate += 1
			dfa_prelude = True
			dfa_cframe += int(arg)
			if dfa_norender:
				tprint("norender = true;")
			tprint(f"cstate = {dfa_cstate}; break;\n")
	print("""	default: break;
	}
	if (!norender)
		BLITZ_Render();
}""")

	

if __name__ == '__main__':
	main()