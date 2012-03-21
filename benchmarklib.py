import pylibmc

def main(clients=1):
    def _main():
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
        for i in range(500):
            mc[str(i)] = i          # This hashmap syntax writes to memcached
        # get
        for i in range(500):
            val = mc[str(i)]        # Hashmap syntax reads from memcached
            assert(int(val) == i)
    if clients == 1:
        _main()
    else:
        assert clients > 1
        import threading
        threads = [threading.Thread(target=_main) for _ in range(clients)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
