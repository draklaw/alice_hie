#!/usr/bin/env python3

from sys import argv
from json import dump
from re import compile

_field_re = compile(r'.*=(.*)')
def parse_line(line):
	fields = []
	for tok in line.split():
		m = _field_re.match(tok)
		if(m):
			fields.append(m.group(1))
	return fields

lines = iter(open(argv[1]))

json = {}

line = parse_line(next(lines))
json["name"] = line[0].strip()
json["size"] = int(line[1])

line = parse_line(next(lines))
json["height"] = int(line[0])

line = parse_line(next(lines))
json["file"] = line[1][1:-1]

line = parse_line(next(lines))
count = int(line[0])

chars = json.setdefault("chars", [])
for i in range(count):
	chars.append(list(map(int, parse_line(next(lines)))))

dump(json, open(argv[2], 'w'), indent='\t')
