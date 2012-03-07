import pylibmc

mc = pylibmc.Client(["127.0.0.1"])
# set
for i in range(10000):
    mc[str(i)] = i
# get
for i in range(10000):
    val = mc[str(i)]
    assert(int(val) == i)
