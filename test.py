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
for i in range(10000):
    mc[str(i)] = i          # This hashmap syntax writes to memcached
# get
for i in range(10000):
    val = mc[str(i)]        # Hashmap syntax reads from memcached
    assert(int(val) == i)
