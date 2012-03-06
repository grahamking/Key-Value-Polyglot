import pylibmc

mc = pylibmc.Client(["127.0.0.1"])
# set
for i in range(500):
    mc[str(i)] = i
# get
for i in range(500):
    val = mc[str(i)]
