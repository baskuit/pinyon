from collections import defaultdict

data = []

with open('test.txt', 'r') as file:
    for line in file:
        values = line.strip().split(',')
        data.append(tuple(values))

exp3_data = defaultdict(lambda:(0, 0, 0, 0))
matrix_ucb_data = defaultdict(lambda:(0, 0, 0, 0))

def new(x, y):
    n = y[-1]
    return tuple(
        (_ + __ * n) / (n + 1) for _, __ in zip(x[:-1], y[:-1])
    ) + (n + 1,)

# Print the data list
for entry in data:
    name, seed, depth_bound, actions, transitions, chance_threshold, _, iterations, count, expl, duration = entry
    duration = duration[:-2]
    key = (depth_bound, actions, transitions, chance_threshold, iterations)
    value = (expl, count, duration, 1)
    value = tuple(float(_) for _ in value)
    if name == 'exp3':
        old_value = exp3_data[key]
        new_value = new(value, old_value)
        exp3_data[key] = new_value
    if name == 'matrix_ucb':
        old_value = matrix_ucb_data[key]
        new_value = new(value, old_value)
        matrix_ucb_data[key] = new_value

for key, value in matrix_ucb_data.items():
    value_ = exp3_data[key]
    print('depth, actions, chance, thresh, iterations')
    print(key)
    print(value_[0], value[0])
    print(int(value_[2]), int(value[2]))
    print()



