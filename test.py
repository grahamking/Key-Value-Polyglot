import pylibmc

mc = pylibmc.Client(["127.0.0.1"])

# Check empty behavior
try:
    mc[str(-1)]
    assert(False)
except KeyError:
    pass

# Check \r\n in data
mc['br'] = 'one\r\ntwo'
assert(mc['br'] == 'one\r\ntwo')

# set
for i in range(10):
    mc[str(i)] = i
# get
for i in range(10):
    val = mc[str(i)]
    assert(int(val) == i)
