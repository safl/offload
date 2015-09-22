#!/usr/bin/env python
import numpy as np
import time

if "flush" in dir(np):
    np.flush()
begin = time.time()

a = np.ones(10) 
a = a +1

if "flush" in dir(np):
    np.flush()

end = time.time() - begin

print(end)
print(a)
