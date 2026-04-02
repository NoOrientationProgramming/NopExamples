#!/usr/bin/env python3

import numpy as np
import argparse

np.set_printoptions(formatter = {'float': '{:7.3f}'.format})

cont = True

x = np.zeros((7, 1))
x[0] = 3.00

rr = 0.05 # Resistance resistor
rl = 0.90 # Resistance line

nr = 1.00 - rr
nr2 = nr - rr
nrl = 1.00 - rl
nb = 1.00 - rr - rl
i = 1.00 if cont else 0.00
o = 0.00 if cont else nr
ob = 0.00 if cont else rr
A = np.array([[   i, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00],
              [1.00,  nrl,   rl, 0.00, 0.00, 0.00, 0.00],
              [0.00,   rl,   nb,   rr, 0.00, 0.00, 0.00],
              [0.00, 0.00,   rr,  nr2,   rr, 0.00, 0.00],
              [0.00, 0.00, 0.00,   rr,  nr2,   rr, 0.00],
              [0.00, 0.00, 0.00, 0.00,   rr,  nr2,   ob],
              [0.00, 0.00, 0.00, 0.00, 0.00,   rr,    o]])

nDef = 1

def matrix_power(n):

	print(f"")

	print(f"x0")
	print(np.transpose(x))
	print(f"")

	print(f"A")
	print(A)

	sums = np.sum(A, axis = 0)
	print(f"Sums A")
	print(f" {sums}")

	print(f"")

	R = np.linalg.matrix_power(A, n)
	print(f"A^{n}")
	print(R)

	sums = np.sum(R, axis = 0)
	print(f"Sums A^{n}")
	print(f" {sums}")

	print(f"")

	xn = R @ x
	print(f"x{n} = A^{n} * x0")
	print(np.transpose(xn))
	print(f"                         ^                       ^")
	print(f"                         |       Resistor        |")
	print(f"                         |                       |")
	print(f"                     Beginning                  End")

	print(f"")

	return xn

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description = 'Calculating A^n')
	parser.add_argument('n', type = int, nargs = '?', default = nDef, help = f"Exponent for A^n (default: {nDef})")

	args = parser.parse_args()
	matrix_power(args.n)

