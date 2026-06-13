#include "../include/kvstore.h"
#include <iostream>
#include <cassert>

using namespace std;

int main()
{
    remove("data.log");

    for(int i=1;i<=20;i++)
    {
        string file =
            "sstable_" +
            to_string(i) +
            ".dat";

        remove(file.c_str());
    }

    cout << "\n==============================\n";
    cout << "TEST 1 : SINGLE FLUSH\n";
    cout << "==============================\n";

    KVStore kv("data.log");

    for(int i=0;i<1000;i++)
    {
        kv.set(
            "key"+to_string(i),
            "value"+to_string(i)
        );
    }

    string val;

    assert(
        kv.get("key500",val)
    );

    assert(
        val=="value500"
    );

    cout << "PASS\n";



    cout << "\n==============================\n";
    cout << "TEST 2 : MULTIPLE FLUSHES\n";
    cout << "==============================\n";

    for(int i=1000;i<5000;i++)
    {
        kv.set(
            "key"+to_string(i),
            "value"+to_string(i)
        );
    }

    assert(
        kv.get("key4500",val)
    );

    assert(
        val=="value4500"
    );

    cout << "PASS\n";



    cout << "\n==============================\n";
    cout << "TEST 3 : RESTART RECOVERY\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        assert(
            recovered.get(
                "key4500",
                val
            )
        );

        assert(
            val=="value4500"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 4 : NEW WRITES AFTER RESTART\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        for(int i=5000;i<7000;i++)
        {
            recovered.set(
                "key"+to_string(i),
                "value"+to_string(i)
            );
        }

        assert(
            recovered.get(
                "key6500",
                val
            )
        );

        assert(
            val=="value6500"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 5 : UPDATE EXISTING KEY\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        recovered.set(
            "key100",
            "UPDATED"
        );

        assert(
            recovered.get(
                "key100",
                val
            )
        );

        assert(
            val=="UPDATED"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 6 : MULTIPLE UPDATES\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        recovered.set(
            "user",
            "v1"
        );

        recovered.set(
            "user",
            "v2"
        );

        recovered.set(
            "user",
            "v3"
        );

        assert(
            recovered.get(
                "user",
                val
            )
        );

        assert(
            val=="v3"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 7 : DELETE\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        recovered.set(
            "temp",
            "hello"
        );

        recovered.del("temp");

        assert(
            !recovered.get(
                "temp",
                val
            )
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 8 : DELETE AFTER FLUSH\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        recovered.set(
            "A",
            "value"
        );

        for(int i=0;i<1000;i++)
        {
            recovered.set(
                "flushA"+to_string(i),
                "x"
            );
        }

        recovered.del("A");

        assert(
            !recovered.get(
                "A",
                val
            )
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 9 : MASSIVE DATASET\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        for(int i=0;i<100000;i++)
        {
            recovered.set(
                "K"+to_string(i),
                "V"+to_string(i)
            );
        }

        assert(
            recovered.get(
                "K56789",
                val
            )
        );

        assert(
            val=="V56789"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 10 : RESTART AFTER MASSIVE DATASET\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        assert(
            recovered.get(
                "K56789",
                val
            )
        );

        assert(
            val=="V56789"
        );

        assert(
            recovered.get(
                "K99999",
                val
            )
        );

        assert(
            val=="V99999"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 11 : SSTABLE SEARCH ORDER\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        recovered.set(
            "name",
            "v1"
        );

        for(int i=0;i<1000;i++)
        {
            recovered.set(
                "dummy1"+to_string(i),
                "x"
            );
        }

        recovered.set(
            "name",
            "v2"
        );

        for(int i=0;i<1000;i++)
        {
            recovered.set(
                "dummy2"+to_string(i),
                "x"
            );
        }

        recovered.set(
            "name",
            "v3"
        );

        for(int i=0;i<1000;i++)
        {
            recovered.set(
                "dummy3"+to_string(i),
                "x"
            );
        }

        assert(
            recovered.get(
                "name",
                val
            )
        );

        assert(
            val=="v3"
        );

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "TEST 12 : RANDOM VERIFICATION\n";
    cout << "==============================\n";

    {
        KVStore recovered("data.log");

        for(int i=0;i<1000;i++)
        {
            int idx = rand()%100000;

            string expected =
                "V"+to_string(idx);

            if(recovered.get(
                "K"+to_string(idx),
                val))
            {
                assert(
                    val==expected
                );
            }
        }

        cout << "PASS\n";
    }



    cout << "\n==============================\n";
    cout << "ALL SSTABLE TESTS PASSED\n";
    cout << "==============================\n";

    return 0;
}