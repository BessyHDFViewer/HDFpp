import hdfpp

def bessy_reshape(fn):
	hdf = hdfpp.HDFpp(fn)
	hlist = hdf.dump()
	hdict = dict()
	for dataset in hlist:
		dname = dataset['name']
		del dataset['name']
		if 'Name' in dataset['attrs']:
			name = dataset['attrs']['Name']
			del dataset['attrs']['Name']
		else:
			name = None
		
		if len(dataset['data'])!=0 :
			value = dataset
		else:
			value = dataset['attrs']

		if (name == None):
			hdict[dname] = value
		else:
			if dname not in hdict:
				hdict[dname] = dict()

			hdict[dname][name] = value

	return hdict

