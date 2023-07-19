import time
from collections import deque

# Normal List (Array)
normal_list = []

start_time = time.time()
for i in range(100000):
    normal_list.insert(0, i)
end_time = time.time()
normal_list_time = end_time - start_time

# Linked List
linked_list = deque()

start_time = time.time()
for i in range(100000):
    linked_list.appendleft(i)
end_time = time.time()
linked_list_time = end_time - start_time

print("Time taken for insertions in Normal List: %.5f seconds" % normal_list_time)
print("Time taken for insertions in Linked List: %.5f seconds" % linked_list_time)
