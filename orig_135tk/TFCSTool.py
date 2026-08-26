import os,sys,zlib,struct,csv

fmt = '=5sII'

def parseTFCS(cont):
	fmtsize = struct.calcsize(fmt)
	if len(cont) < fmtsize:
		raise IOError(sys.argv[2] + ' : Invalid TFCS file.')

	magic,compSize,origSize = struct.unpack(fmt,cont[:fmtsize])

	if magic != 'TFCS\0' or compSize < len(cont)-fmtsize:
		raise IOError(sys.argv[2] + ' : Invalid TFCS file. (Invalid header).')

	deco = zlib.decompress(cont[fmtsize:fmtsize+compSize])
	if len(deco) != origSize:
		raise IOError(sys.argv[2] + ' : Invalid TFCS file. (Decompress failed)')

	rowcnt, = struct.unpack('I',deco[:4])
	deco = deco[4:]
	result = []
	for i in xrange(rowcnt):
		colcnt, = struct.unpack('I',deco[:4])
		deco = deco[4:]
		trow = []
		for j in xrange(colcnt):
			tlen, = struct.unpack('I',deco[:4])
			trow += [deco[4:4+tlen]]
			deco = deco[4+tlen:]
		result += [trow]
	return result

def buildTFCS(data):
	stream = ''
	stream += struct.pack('I',len(data)) # row cnt
	for row in data:
		stream += struct.pack('I',len(row))
		for ele in row:
			stream += struct.pack('I',len(ele))
			stream += ele

	compressed = zlib.compress(stream)
	header = struct.pack(fmt,'TFCS\0',len(compressed),len(stream))
	return header + compressed

def parseCSV(cont):
	reader = csv.reader(cont)
	result = []
	for row in reader:
		result += [row]
	return result

def writeCSV(data,output):
	writer = csv.writer(output)
	writer.writerows(data)
	return

if __name__ == '__main__':
	print 'TFCSTool for TH13.5(HM)'
	print '              By Riatre'
	if len(sys.argv) < 4:
		print >>sys.stderr,'Usage: TFCSTool.py </d|/w> <binfile> <csvfile>'
		print >>sys.stderr,'       d for Dump, w for Write back.'
		sys.exit(0)

	if sys.argv[1] == '/d' or sys.argv[1] == '-d':
		with open(sys.argv[2],'rb') as fp:
			mt = parseTFCS(fp.read())
		with open(sys.argv[3],'wb') as fp:
			writeCSV(mt,fp)

		print '%s to %s, Done!' % (sys.argv[2],sys.argv[3])

	if sys.argv[1] == '/w' or sys.argv[1] == '-w':
		with open(sys.argv[3],'rb') as fp:
			mt = parseCSV(fp)
		if (mt[0][0] != 'key' or mt[1][0] != 'type') and mt[0][0] != 'type': raise SyntaxError('Not valid format.')
		with open(sys.argv[2],'wb') as fp:
			fp.write(buildTFCS(mt))

		print '%s to %s, Done!' % (sys.argv[3],sys.argv[2])
