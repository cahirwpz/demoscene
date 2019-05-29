#!/usr/bin/env python3

import struct
import sys
import math


class TrackRow:
  def __init__(self, f):
    i4_period, i0_cmd, self.arg = struct.unpack(">HBB", f.read(4))
    self.period = i4_period & 0x0fff
    self.inst = (i0_cmd >> 4) | ((i4_period & 0xf000) >> 8)
    self.cmd = i0_cmd & 0x0f
    self.note = int(round(math.log(856.0 / self.period, 2) * 12.0)) if self.period > 0 else None


class Instrument:
  def __init__(self, f):
    self.name = f.read(22).rstrip(b'\0').decode()
    self.length, self.finetune, self.volume, self.repoffset, self.replen = struct.unpack(
        ">HBBHH", f.read(8))
    self.samples = None


class Module:
  def __init__(self, f):
    self.name = f.read(20).rstrip(b'\0').decode()
    self.instruments = [None] * 32
    for i in range(1, 32):
      self.instruments[i] = Instrument(f)
    self.songlength, dummy = struct.unpack("BB", f.read(2))
    self.positions = list(struct.unpack("128B", f.read(128)))
    mk = f.read(4)
    self.patterns = []
    num_patterns = max(self.positions) + 1
    for p in range(num_patterns):
      pat = []
      for r in range(64):
        row = []
        for t in range(4):
          trackrow = TrackRow(f)
          row.append(trackrow)
        pat.append(row)
      self.patterns.append(pat)
    for inst in self.instruments:
      if inst:
        inst.samples = f.read(inst.length * 2)


def notename(n):
  if n is None:
    return "   "
  return ["C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"][n % 12] + str(n // 12 + 1)


def printpattern(pat):
  for r in range(64):
    for t in range(4):
      tr = pat[r][t]
      sys.stdout.write(" %3s %2X %1X %02X   " %
                       (notename(tr.note), tr.inst, tr.cmd, tr.arg))
    print()


n_errors = 0
reported_errors = set()


def error(msg, p, t, r):
  if (msg, p, t, r) not in reported_errors:
    print("%s in pattern %d track %d row %d" % (msg, p, t, r))
  reported_errors.add((msg, p, t, r))


# Commandline
if len(sys.argv) < 3:
  print(
      "Usage: %s <input module file> <output binary data file> [<output raw instrument file>]" % sys.argv[0])
  sys.exit(1)
module_file = sys.argv[1]
output_file = sys.argv[2]
raw_inst_file = sys.argv[3] if len(sys.argv) > 3 else None

print("Converting module file %s..." % module_file)
module = Module(open(module_file, "rb"))


# Parse instrument parameters
def param(s):
  if s == "":
    raise ValueError
  if s.upper() == "X" * len(s):
    return pow(10, len(s))
  return int(s)


inst_params = [None]
for i in range(1, 32):
  inst = module.instruments[i]
  try:
    # Read parameters
    p = [param(inst.name[pi * 2 + 1:pi * 2 + 3]) for pi in range(8)]
    p += [param(inst.name[pi + 17:pi + 18]) for pi in range(4)]
    inst_params.append(p)
  except ValueError:
    inst_params.append(None)


# Parse music data
volumedata = [[], [], [], []]
notedata = [[], [], [], []]
perioddata = [[], [], [], []]
offsetdata = [[], [], [], []]
posdata = []
vblank = 0

musicspeed = 6
inst = [0, 0, 0, 0]
period = [0, 0, 0, 0]
volume = [0, 0, 0, 0]
portamento_target = [0, 0, 0, 0]
portamento_speed = [0, 0, 0, 0]
offset_value = [0, 0, 0, 0]

states = dict()

periodtable = [
    856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
    428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
]

startrow = 0
restart = 0
stopped = False
looped = False
skip = False
pos = 0
while not stopped and not looped:
  p = module.positions[pos]
  pat = module.patterns[p]
  next_pos = pos + 1

  for r in range(startrow, 64):
    if skip:
      # Skip first row after patterndelay + patternbreak
      skip = False
      continue
    state = (pos, r, musicspeed, tuple(inst), tuple(period), tuple(volume), tuple(
        portamento_target), tuple(portamento_speed), tuple(offset_value))
    if state in states:
      restart = states[state]
      looped = True
      break
    states[state] = vblank
    row = [(t, tr, 0xE0 | (tr.arg >> 4) if tr.cmd == 0xE else tr.cmd,
            tr.arg >> 4, tr.arg & 0xF) for t, tr in enumerate(pat[r])]

    # Check for unsupported commands
    for t, tr, cmd, arg1, arg2 in row:
      if cmd in [0x4, 0x6, 0x7, 0xE0, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xEF]:
        error("Unsupported command %X" % cmd, p, t, r)

    # Pick up speed and break
    patternbreak = False
    startrow = 0
    patterndelay = 0
    for t, tr, cmd, arg1, arg2 in row:
      if cmd == 0xF:
        if tr.arg != 0:
          if tr.arg < 0x20:
            musicspeed = tr.arg
          else:
            if tr.arg != 125:
              error("Tempo set", p, t, r)
        else:
          stopped = True
      if cmd == 0xD:
        patternbreak = True
        startrow = arg1 * 10 + arg2
        if startrow > 63:
          error("Break to position outside pattern", p, t, r)
          startrow = 0
      if cmd == 0xB:
        patternbreak = True
        next_pos = tr.arg
      if cmd == 0xEE:
        patterndelay = arg2
    speed = musicspeed * (patterndelay + 1)
    if patterndelay > 0 and patternbreak:
      # Skip first row after patterndelay + patternbreak
      skip = True
    if stopped:
      speed = 1
      patternbreak = True
      restart = vblank + 1

    for row_number, row_data in enumerate(row):
      t, tr, cmd, arg1, arg2 = row_data
      # print(pos, r, row_number)
      # print(tr.cmd, notename(tr.note), tr.inst)
      # Volume data
      if tr.inst != 0:
        volume[t] = module.instruments[tr.inst].volume
      if cmd == 0xC:
        # Set volume
        volume[t] = tr.arg
      if cmd == 0xEC and arg2 < speed and arg2 < musicspeed:
        # Notecut
        volumedata[t] += [volume[t]] * arg2 + [0] * (speed - arg2)
        volume[t] = 0
      elif cmd in [0x5, 0xA]:
        # Volumeslide
        if arg1:
          slide = arg1
        else:
          slide = -arg2
        volumedata[t] += [max(0, min(volume[t] + i * slide, 64))
                          for i in range(speed)]
        volume[t] = volumedata[t][-1]
      else:
        if cmd == 0xEA:
          # Finevolume up
          volume[t] = min(volume[t] + arg2, 64)
        if cmd == 0xEB:
          # Finevolume down
          volume[t] = max(0, volume[t] - arg2)
        volumedata[t] += [volume[t]] * speed

      # Note trigger data
      if tr.inst != 0:
        if tr.inst != inst[t] and cmd in [0x3, 0x5]:
          error("Instrument change on toneportamento", p, t, r)
        inst[t] = tr.inst
      if inst[t] == 0:
        if tr.note is not None or (cmd == 0xE9 and arg2 != 0):
          error("Note with no instrument", p, t, r)
        notedata[t] += [0] * speed
      elif cmd == 0xE9 and arg2 != 0:
        # Retrig note
        for i in range(speed):
          if (i % arg2) == 0:
            notedata[t] += [inst[t]]
          else:
            notedata[t] += [0]
      elif tr.note is not None and cmd == 0xED:
        # Notedelay
        if arg2 < speed and arg2 < musicspeed:
          notedata[t] += [0] * arg2 + [inst[t]] + [0] * (speed - arg2 - 1)
        else:
          notedata[t] += [0] * speed
      elif tr.note is not None and cmd not in [0x3, 0x5]:
        notedata[t] += [inst[t]] + [0] * (speed - 1)
      else:
        notedata[t] += [0] * speed

      # Offset data
      if cmd == 0x9:
        if tr.arg != 0:
          offset_value[t] = tr.arg
        elif offset_value[t] == 0:
          error("No previous offset", p, t, r)
        offset = offset_value[t]
        if inst[t] != 0 and tr.note and offset * 128 >= module.instruments[inst[t]].length:
          error("Offset beyond end of sample", p, t, r)
          offset = (module.instruments[inst[t]].length - 1) / 128
        offsetdata[t] += [offset] + [0] * (speed - 1)
      else:
        offsetdata[t] += [0] * speed

      # Period data
      if tr.note is not None and cmd not in [0x3, 0x5, 0xED]:
        if cmd in [0xE1, 0xE2]:
          error("Fineslide on note", p, t, r)
        period[t] = periodtable[tr.note]
      if cmd == 0x0 and tr.arg != 0:
        # Arpeggio
        if period[t] == 0:
          error("Arpeggio with no base note", p, t, r)
          period[t] = periodtable[0]
        note = min(i for i, p in enumerate(periodtable) if p <= period[t])
        if periodtable[note] != period[t]:
          error("Arpeggio with invalid base pitch (after slide)", p, t, r)
        arpnotes = [note, note + arg1, note + arg2]
        for a in [1, 2]:
          if arpnotes[a] >= len(periodtable):
            error("Arpeggio note above B-3", p, t, r)
            arpnotes[a] = len(periodtable) - 1
        for i in range(speed):
          perioddata[t] += [periodtable[arpnotes[(i % musicspeed) % 3]]]
      elif cmd in [0x1, 0x2]:
        # Portamento
        if period[t] == 0:
          error("Portamento with no source", p, t, r)
          period[t] = periodtable[0]
        slide = -tr.arg if cmd == 0x1 else tr.arg
        perioddata[t] += [max(periodtable[-1], min(period[t] +
                                                   i * slide, periodtable[0])) for i in range(speed)]
        period[t] = perioddata[t][-1]
      elif cmd in [0x3, 0x5]:
        # Toneportamento
        if tr.note is not None:
          portamento_target[t] = periodtable[tr.note]
        if cmd == 0x3 and tr.arg != 0:
          portamento_speed[t] = tr.arg
        if period[t] == 0:
          error("Toneportamento with no source", p, t, r)
          period[t] = periodtable[0]
        if portamento_target[t] == 0:
          error("Toneportamento with no target", p, t, r)
          portamento_target[t] = period[t]
        if portamento_speed[t] == 0:
          error("Toneportamento with no speed", p, t, r)
        perioddata[t] += [period[t]]
        for i in range(speed - 1):
          if portamento_target[t] > period[t]:
            period[t] = min(period[t] + portamento_speed[t],
                            portamento_target[t])
          else:
            period[t] = max(period[t] - portamento_speed[t],
                            portamento_target[t])
          perioddata[t] += [period[t]]
      elif tr.note is not None and cmd == 0xED:
        # Notedelay
        if arg2 < speed and arg2 < musicspeed:
          perioddata[t] += [period[t]] * arg2 + \
              [periodtable[tr.note]] * (speed - arg2)
        else:
          perioddata[t] += [period[t]] * speed
        period[t] = periodtable[tr.note]
      else:
        if cmd == 0xE1 and tr.note is None:
          # Fineslide up
          period[t] = max(period[t] - arg2, periodtable[-1])
        if cmd == 0xE2 and tr.note is None:
          # Fineslide down
          period[t] = min(period[t] + arg2, periodtable[0])
        perioddata[t] += [period[t]] * speed

    # Advance
    posdata += [(p, r)] * speed
    vblank += speed
    assert len(volumedata[t]) == vblank
    assert len(notedata[t]) == vblank
    assert len(offsetdata[t]) == vblank
    assert len(perioddata[t]) == vblank

    if patternbreak:
      break

  pos = next_pos
  if pos >= module.songlength:
    pos = 0


# Find note ranges and count notes per instrument
minmax_note = dict()
inst_counts = [0] * 32
for track in range(4):
  for inst, per, offset in zip(notedata[track], perioddata[track], offsetdata[track]):
    if inst != 0:
      inst_counts[inst] += 1
      note = periodtable.index(per)
      if (inst, offset) in minmax_note:
        note_min, note_max = minmax_note[(inst, offset)]
        note_min = min(note_min, note)
        note_max = max(note_max, note)
        minmax_note[(inst, offset)] = note_min, note_max
      else:
        minmax_note[(inst, offset)] = note, note


# List of used instruments
inst_list = [inst for inst in range(32) if inst_counts[inst] != 0]
inst_list.sort(key=(
    lambda i: 99999 - i if inst_params[i] is None else inst_counts[i]), reverse=True)


# Build note ID mapping table
note_id = 0
note_ids = dict()
note_range_list = []
note_id_start = [0]
for inst in inst_list:
  if (inst, 0) not in minmax_note:
    minmax_note[(inst, 0)] = (0, 0)
  for offset in range(0, 256):
    if (inst, offset) in minmax_note:
      note_min, note_max = minmax_note[(inst, offset)]
      note_range_list += [(note_min, note_max, offset)]
      for n in range(note_min, note_max + 1):
        note_ids[(inst, offset, n)] = note_id
        note_id += 1
  note_id_start.append(note_id)

if note_id > 512:
  print("More than 512 different note IDs!")
  n_errors += 1


# Export notes
VOLUME_SHIFT = 9
NOTE_SHIFT = 0
NOTE_ABS_MASK = 0x80

dataset = set()
track_data = [[], [], [], []]
for track in range(4):
  initial = True
  pvol = 0
  pper = 0
  pdper = 0
  for (pat, row), vol, per, inst, offset in zip(posdata, volumedata[track], perioddata[track], notedata[track], offsetdata[track]):
    if vol == 64:
      vol = 63
    if inst != 0:
      note = periodtable.index(per)
      data = 0x8000 | (note_ids[(inst, offset, note)] << NOTE_SHIFT) | (vol << VOLUME_SHIFT)
      initial = False
      pdper = 0
    elif initial:
      data = 0
    else:
      dper = (per - pper) & 511
      dvol = (vol - pvol) & 63
      if per != pper and dper != pdper and per in periodtable:
        note = periodtable.index(per)
        data = ((NOTE_ABS_MASK | note) << NOTE_SHIFT) | (dvol << VOLUME_SHIFT)
        pdper = 0
      else:
        if per - pper < -256 or per - pper > 255:
          error("Slide value out of range (from %d to %d)" %
                (pper, per), pat, track, row)
          per = pper + 255 if per > pper else pper - 256
          dper = (per - pper) & 511
        if ((dper >> 7) ^ (dper >> 6)) & 1 == 1:
          error("Unsupported slide value", pat, track, row)
          dper = 63
        data = (dper << NOTE_SHIFT) | (dvol << VOLUME_SHIFT)
        pdper = dper
    track_data[track].append(data)
    dataset.add(data)
    pvol = vol
    pper = per

  if stopped:
    track_data[track].append(0)

while restart > 0 and all(track_data[t][restart - 1] == track_data[t][-1] for t in range(4)):
  for t in range(4):
    track_data[t].pop()
  restart -= 1

notes_data = b''
for track in [3, 2, 1, 0]:
  notes_data += struct.pack(">%dH" %
                            len(track_data[track]), *track_data[track])
musiclength = len(notes_data) // 8

# Export note ranges
note_range_data = b''
for note_min, note_max, offset in note_range_list:
  note_range_data += struct.pack(">BBH", note_min,
                                 note_max - note_min + 1, offset * 128)
note_range_data += struct.pack(">h", (restart - musiclength + 1) * 2)

# Export instrument parameters
inst_data = [""] * len(inst_list)
raw_instruments = []
raw_inst_size = 0
total_inst_size = 0
total_inst_time = 1.0
last_nonempty_inst = max(i for i in range(
    1, 32) if module.instruments[i].name.strip() != "" or i in inst_list)
print()
print("Inst Name                   Length Repeat Idx Count  Low High 9xx  IDs  Error?")
for i in range(1, last_nonempty_inst + 1):
  inst = module.instruments[i]

  # Unused instrument?
  if i not in inst_list:
    print("%02d   %-22s" % (i, inst.name))
    continue

  # General statistics
  index = inst_list.index(i)
  min_note = min(note_min for ((inst, offset), (note_min, note_max))
                 in minmax_note.items() if inst == i)
  max_note = max(note_max for ((inst, offset), (note_min, note_max))
                 in minmax_note.items() if inst == i)
  offsets = [offset for oi, offset in minmax_note if oi == i]
  n_note_ids = note_id_start[index + 1] - note_id_start[index]
  msg = ""

  # Length and repeat length
  length = inst.length
  if length < 1:
    msg = "Empty!"
    length = 1
  if inst.repoffset == 0 and inst.replen in [0, 1]:
    replen = 0
    max_offset = max(offsets) * 128
    while length > max_offset and inst.samples[(length - 1) * 2:length * 2] == "\0\0":
      length -= 1
  else:
    replen = inst.replen
    if inst.repoffset != inst.length - inst.replen:
      msg = "Repeat is not at end!"
  total_inst_size += length
  inst.length = length

  p = inst_params[i]
  if p is not None:
    # Parameters on word form for synth code
    attack = 65536 - int(math.floor(10000.0 / (1 + p[0] * p[0])))
    decay = int(math.floor(10000.0 / (1 + p[1] * p[1])))
    mpitch = p[2] * 512
    mpitchdecay = int(math.floor(
        math.exp(-0.000002 * p[3] * p[3]) * 65536)) & 0xffff
    bpitch = p[4] * 512
    bpitchdecay = int(math.floor(
        math.exp(-0.000002 * p[5] * p[5]) * 65536)) & 0xffff
    mod = p[6]
    moddecay = int(math.floor(
        math.exp(-0.000002 * p[7] * p[7]) * 65536)) & 0xffff

    # Distortion parameters for synth code
    dist = (p[8] << 12) | (p[9] << 8) | (p[10] << 4) | p[11]

    inst_data[index] = struct.pack(">11H", length, replen, mpitch, mod,
                                   bpitch, attack, dist, decay, mpitchdecay, moddecay, bpitchdecay)
    total_inst_time += (20 + p[8] + p[9] + p[10] + p[11]) * length * 0.000017
    inst_type = "C"
  else:
    inst_data[index] = struct.pack(">2H", length, replen)
    raw_instruments.append(i)
    raw_inst_size += length
    inst_type = "R"

  print("%02d %c %-22s %6d %6s  %2d %5d  %3s  %3s %3d %4d  %s" % (
      i, inst_type, inst.name, length *
        2, "" if not replen else replen * 2, index, inst_counts[i],
        notename(min_note), notename(max_note), len(
            offsets) - 1, n_note_ids, msg
        ))

  if msg != "":
    n_errors += 1

insts_data = b''
if len(raw_instruments) > 0:
  insts_data += struct.pack(">h", -len(raw_instruments))
  insts_data += b''.join(inst_data[:len(raw_instruments)])
insts_data += struct.pack(">h", len(inst_list) - len(raw_instruments) - 1)
insts_data += b''.join(inst_data[len(raw_instruments):])

# Write output file
fout = open(output_file, "wb")
fout.write(insts_data)
fout.write(struct.pack(">hh", len(notes_data) // 4, len(note_range_data)))
fout.write(note_range_data)
fout.write(notes_data)
out_size = fout.tell()
fout.close()

if raw_inst_file is not None:
  fout = open(raw_inst_file, "wb")
  for i in raw_instruments:
    inst = module.instruments[i]
    fout.write(inst.samples[:inst.length * 2])
  fout.close()

print()
print("Uncompressed music data size: %7d bytes" % out_size)
if raw_inst_size > 0:
  print("Total raw instrument size:    %7d bytes" % (raw_inst_size * 2))
print("Total instrument memory:      %7d bytes" % (total_inst_size * 2))
print("Appr. precalc time on 68000:  %7d seconds" % int(total_inst_time + 0.5))
print("Music duration:               %7d vblanks (%d:%02d)" %
      (musiclength, (musiclength + 25) / 3000, (musiclength + 25) % 3000 / 50))
print("Restart position:             %7d vblanks (%d:%02d)" %
      (restart, (restart + 25) / 3000, (restart + 25) % 3000 / 50))
print("Number of different note IDs:   %5d" % note_id)
print("Number of different data words: %5d" % len(dataset))
print()
n_errors += len(reported_errors)
if n_errors == 0:
  print("No errors.")
else:
  print("%d error%s." % (n_errors, "s" if n_errors > 1 else ""))

if raw_inst_size > 0 and raw_inst_file is None:
  print()
  print("Warning: Raw instruments used, but no raw instrument output file specified!")
if raw_inst_size == 0 and raw_inst_file is not None:
  print()
  print("Warning: Raw instrument output file specified, but no raw instruments used!")
