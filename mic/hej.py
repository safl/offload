#!/usr/bin/env python
import numpy as np
import time

if "flush" in dir(np):
    np.flush()
begin = time.time()

#a = np.sum(((np.ones(100)+1.0)*2.0)/2.0)
a = np.sum(np.random.random(50000000))
#a = np.multiply.accumulate(np.ones((8,8), dtype=np.float32))
print(a)

if "flush" in dir(np):
    np.flush()

end = time.time() - begin

print(end)
